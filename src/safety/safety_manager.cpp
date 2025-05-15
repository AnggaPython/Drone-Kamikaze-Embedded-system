#include "safety_manager.hpp"
#include "../communication/radio.hpp"
#include "../navigation/gps.hpp"
#include "../navigation/imu.hpp"
#include "../control/motor_controller.hpp"
#include "../targeting/target_detection.hpp"
#include "../core/rtos.hpp"

namespace Kamikaze {
namespace Safety {

// Inisialisasi static members
const Float32 SafetyManager::LOW_BATTERY_THRESHOLD = 20.0F;        // 20% baterai tersisa
const Float32 SafetyManager::CRITICAL_BATTERY_THRESHOLD = 10.0F;   // 10% baterai tersisa
const Uint32 SafetyManager::LINK_LOSS_TIMEOUT_MS = 3000U;         // 3 detik timeout radio link
const Float32 SafetyManager::MIN_TARGET_CONFIDENCE = 70.0F;       // Minimum confidence untuk target valid

// Implementasi singleton
SafetyManager& SafetyManager::getInstance() {
    static SafetyManager instance;
    return instance;
}

// Konstruktor
SafetyManager::SafetyManager() : 
    safetyData_(),
    initialized_(false) {
    // Tidak ada inisialisasi tambahan
}

// Inisialisasi
bool SafetyManager::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Reset safety data
    safetyData_ = SafetyData();
    
    initialized_ = true;
    return true;
}

// Update status keamanan
bool SafetyManager::update() {
    if (!initialized_) {
        return false;
    }
    
    // Update berbagai status
    updateBatteryStatus();
    updateGpsStatus();
    updateRadioStatus();
    updateSensorStatus();
    updateMotorStatus();
    updateTargetStatus();
    
    // Update mode keamanan berdasarkan status
    updateSafetyMode();
    
    return true;
}

// Mendapatkan data keamanan
const SafetyData& SafetyManager::getSafetyData() const {
    return safetyData_;
}

// Mendapatkan mode keamanan
SafetyMode SafetyManager::getMode() const {
    return safetyData_.mode;
}

// Mendapatkan error saat ini
ErrorCode SafetyManager::getError() const {
    return safetyData_.error;
}

// Mendapatkan tindakan keamanan berdasarkan mode
SafetyAction SafetyManager::getRecommendedAction(SafetyMode mode) const {
    switch (mode) {
        case SafetyMode::NORMAL:
            return SafetyAction::NONE;
            
        case SafetyMode::WARNING:
            return SafetyAction::ALERT;
            
        case SafetyMode::CRITICAL:
            return SafetyAction::RTL;
            
        case SafetyMode::EMERGENCY:
            return SafetyAction::LAND;
            
        default:
            return SafetyAction::ABORT;
    }
}

// Eksekusi tindakan keamanan
bool SafetyManager::executeAction(SafetyAction action) {
    bool result = true;
    
    switch (action) {
        case SafetyAction::NONE:
            // Tidak ada tindakan yang diambil
            break;
            
        case SafetyAction::ALERT:
            // Kirim peringatan melalui telemetri
            // Implementasi dummy untuk contoh
            break;
            
        case SafetyAction::RTL:
            // Inisiasi return to launch
            // Implementasi dummy untuk contoh
            break;
            
        case SafetyAction::LAND:
            // Inisiasi pendaratan
            // Implementasi dummy untuk contoh
            break;
            
        case SafetyAction::DISARM:
            // Matikan motor
            result = Control::MotorController::getInstance().stopAll();
            break;
            
        case SafetyAction::ABORT:
            // Abort mission dan emergency landing
            result = Control::MotorController::getInstance().stopAll();
            break;
            
        default:
            result = false;
            break;
    }
    
    return result;
}

// Menetapkan error
void SafetyManager::setError(ErrorCode error) {
    safetyData_.error = error;
    safetyData_.lastErrorTime = Core::Rtos::getTicks();
    
    // Update mode keamanan
    updateSafetyMode();
}

// Clear error
void SafetyManager::clearError() {
    safetyData_.error = ErrorCode::NONE;
    
    // Update mode keamanan
    updateSafetyMode();
}

// Cek apakah drone dalam kondisi aman
bool SafetyManager::isSafe() const {
    return (safetyData_.mode == SafetyMode::NORMAL);
}

// Update status baterai
void SafetyManager::updateBatteryStatus() {
    // Untuk contoh ini, kita akan menggunakan nilai dummy
    
    // Perbarui status baterai
    safetyData_.battery.voltage = 11.8F;
    safetyData_.battery.current = 5.2F;
    safetyData_.battery.remaining = 75.0F;
    safetyData_.battery.temperature = 25.0F;
    
    // Cek status baterai
    if (safetyData_.battery.remaining <= CRITICAL_BATTERY_THRESHOLD) {
        setError(ErrorCode::LOW_BATTERY);
    } else if (safetyData_.battery.remaining <= LOW_BATTERY_THRESHOLD) {
        // Jika baterai rendah tapi tidak kritis, tetapkan warning
        if (safetyData_.error == ErrorCode::NONE) {
            safetyData_.error = ErrorCode::LOW_BATTERY;
        }
    }
}

// Update status GPS
void SafetyManager::updateGpsStatus() {
    // Dapatkan data GPS dari modul navigasi
    const Navigation::GpsData& gpsData = Navigation::Gps::getInstance().getData();
    
    // Periksa status GPS
    if (gpsData.status == Navigation::GpsStatus::NO_FIX) {
        safetyData_.gpsSignalLost = true;
        
        // Set error jika belum diatur
        if (safetyData_.error == ErrorCode::NONE) {
            setError(ErrorCode::GPS_LOST);
        }
    } else {
        safetyData_.gpsSignalLost = false;
        
        // Clear error GPS lost jika saat ini aktif
        if (safetyData_.error == ErrorCode::GPS_LOST) {
            clearError();
        }
    }
}

// Update status radio
void SafetyManager::updateRadioStatus() {
    // Periksa status radio
    Communication::RadioStatus radioStatus = Communication::Radio::getInstance().getStatus();
    
    if (radioStatus == Communication::RadioStatus::DISCONNECTED || 
        radioStatus == Communication::RadioStatus::ERROR) {
        // Set radio link lost
        safetyData_.radioLinkLost = true;
        
        // Set error jika belum diatur
        if (safetyData_.error == ErrorCode::NONE) {
            setError(ErrorCode::COMMUNICATION_ERROR);
        }
    } else {
        safetyData_.radioLinkLost = false;
        
        // Clear communication error jika saat ini aktif
        if (safetyData_.error == ErrorCode::COMMUNICATION_ERROR) {
            clearError();
        }
    }
}

// Update status sensor
void SafetyManager::updateSensorStatus() {
    // Periksa status IMU
    Navigation::ImuStatus imuStatus = Navigation::Imu::getInstance().getData().status;
    
    if (imuStatus == Navigation::ImuStatus::ERROR) {
        // Set IMU error
        safetyData_.imuError = true;
        
        // Set error jika belum diatur
        if (safetyData_.error == ErrorCode::NONE) {
            setError(ErrorCode::IMU_ERROR);
        }
    } else {
        safetyData_.imuError = false;
        
        // Clear IMU error jika saat ini aktif
        if (safetyData_.error == ErrorCode::IMU_ERROR) {
            clearError();
        }
    }
}

// Update status motor
void SafetyManager::updateMotorStatus() {
    // Periksa status motor
    bool motorError = false;
    
    // Cek setiap motor
    Control::MotorController& motorController = Control::MotorController::getInstance();
    
    for (Uint8 i = 0U; i < static_cast<Uint8>(MAX_MOTOR_COUNT); ++i) {
        Control::MotorPosition position = static_cast<Control::MotorPosition>(i);
        Control::MotorStatus status = motorController.getMotor(position).getStatus();
        
        if (status == Control::MotorStatus::ERROR) {
            motorError = true;
            break;
        }
    }
    
    // Update status motor
    safetyData_.motorError = motorError;
    
    // Set error jika ada masalah motor
    if (motorError && safetyData_.error == ErrorCode::NONE) {
        setError(ErrorCode::MOTOR_ERROR);
    } else if (!motorError && safetyData_.error == ErrorCode::MOTOR_ERROR) {
        clearError();
    }
}

// Cek apakah aman untuk menyerang target
bool SafetyManager::isSafeToAttack() const {
    // Kondisi yang harus dipenuhi untuk menyerang:
    // 1. Mode keamanan normal atau warning (tidak kritis atau darurat)
    // 2. GPS harus berfungsi
    // 3. IMU harus berfungsi
    // 4. Target harus valid dan dalam jarak serangan
    
    if (safetyData_.mode == SafetyMode::CRITICAL || safetyData_.mode == SafetyMode::EMERGENCY) {
        return false;
    }
    
    if (safetyData_.gpsSignalLost || safetyData_.imuError) {
        return false;
    }
    
    return (hasValidTarget() && safetyData_.inAttackRange);
}

// Cek apakah drone memiliki target yang valid
bool SafetyManager::hasValidTarget() const {
    const Targeting::TargetData& targetData = Targeting::TargetDetection::getInstance().getTargetData();
    
    // Target dianggap valid jika:
    // 1. Status target adalah LOCKED
    // 2. Atau status target adalah ACQUIRING dengan confidence di atas threshold
    
    if (targetData.status == Targeting::TargetStatus::TARGET_LOCKED) {
        return true;
    }
    
    if (targetData.status == Targeting::TargetStatus::TARGET_ACQUIRING && 
        targetData.confidence >= MIN_TARGET_CONFIDENCE) {
        return true;
    }
    
    return false;
}

// Cek apakah drone berada dalam jarak serangan
bool SafetyManager::isInAttackRange(Float32 attackRange) const {
    return Targeting::TargetDetection::getInstance().isInAttackRange(attackRange);
}

// Cek kondisi target dan update status keamanan terkait
bool SafetyManager::checkTargetConditions() {
    // Update status target
    updateTargetStatus();
    
    // Cek jika target hilang dan perlu diatur error
    if (safetyData_.targetLost && safetyData_.error == ErrorCode::NONE) {
        setError(ErrorCode::TARGET_LOST);
        return false;
    }
    
    // Cek jika drone dalam jarak serangan dan kondisi aman
    if (safetyData_.inAttackRange && isSafeToAttack()) {
        return true;
    }
    
    return false;
}

// Update status target
void SafetyManager::updateTargetStatus() {
    // Dapatkan data target dari modul targeting
    const Targeting::TargetData& targetData = Targeting::TargetDetection::getInstance().getTargetData();
    
    // Cek jika target hilang
    safetyData_.targetLost = (targetData.status == Targeting::TargetStatus::TARGET_LOST || 
                            targetData.status == Targeting::TargetStatus::NO_TARGET);
    
    // Cek jika dalam jarak serangan
    safetyData_.inAttackRange = isInAttackRange();
    
    // Clear error TARGET_LOST jika target sudah tidak hilang lagi
    if (!safetyData_.targetLost && safetyData_.error == ErrorCode::TARGET_LOST) {
        clearError();
    }
}

// Update mode keamanan berdasarkan status
void SafetyManager::updateSafetyMode() {
    // Tentukan mode keamanan berdasarkan kondisi drone
    
    // Jika tidak ada error, mode normal
    if (safetyData_.error == ErrorCode::NONE) {
        safetyData_.mode = SafetyMode::NORMAL;
        return;
    }
    
    // Jika ada error, tentukan tingkat keparahannya
    switch (safetyData_.error) {
        case ErrorCode::LOW_BATTERY:
            // Cek level baterai
            if (safetyData_.battery.remaining <= CRITICAL_BATTERY_THRESHOLD) {
                safetyData_.mode = SafetyMode::CRITICAL;
            } else {
                safetyData_.mode = SafetyMode::WARNING;
            }
            break;
            
        case ErrorCode::GPS_LOST:
            // GPS lost adalah warning, kecuali dalam misi
            safetyData_.mode = SafetyMode::WARNING;
            break;
            
        case ErrorCode::COMMUNICATION_ERROR:
            // Radio link lost adalah critical
            safetyData_.mode = SafetyMode::CRITICAL;
            break;
            
        case ErrorCode::TARGET_LOST:
            // Target lost adalah warning
            safetyData_.mode = SafetyMode::WARNING;
            break;
            
        case ErrorCode::IMU_ERROR:
        case ErrorCode::MOTOR_ERROR:
        case ErrorCode::SENSOR_ERROR:
            // Error pada sensor atau motor adalah emergency
            safetyData_.mode = SafetyMode::EMERGENCY;
            break;
            
        default:
            // Error lainnya adalah warning
            safetyData_.mode = SafetyMode::WARNING;
            break;
    }
}

// Inisialisasi modul safety
bool initializeSafety() {
    return SafetyManager::getInstance().initialize();
}

} // namespace Safety
} // namespace Kamikaze 