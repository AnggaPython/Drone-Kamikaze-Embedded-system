#include "../include/common.hpp"
#include "core/rtos.hpp"
#include "system_init.cpp"
#include "control/motor_controller.hpp"
#include "control/pid_controller.hpp"
#include "navigation/gps.hpp"
#include "navigation/imu.hpp"
#include "communication/radio.hpp"
#include "communication/telemetry.hpp"
#include "targeting/target_detection.hpp"
#include "safety/safety_manager.hpp"

// Forward declarations dari modul lainnya
namespace Kamikaze {
namespace Navigation {
    bool initializeNavigation();
}

namespace Control {
    bool initializeControl();
}

namespace Communication {
    bool initializeCommunication();
}

namespace Targeting {
    bool initializeTargeting();
}

namespace Safety {
    bool initializeSafety();
}
}

/**
 * @brief Main task untuk drone kamikaze
 */
class MainTask : public Kamikaze::Core::Task {
public:
    MainTask() : 
        systemStatus_(Kamikaze::SystemStatus::INITIALIZING),
        armed_(false),
        targetLocked_(false),
        inAttackMode_(false) {
        // Konstruktor sesuai MISRA
    }
    
    virtual ~MainTask() {
        // Destruktor kosong sesuai MISRA
    }
    
    /**
     * @brief Implementasi utama task
     */
    virtual Kamikaze::Uint32 run() {
        // Main loop
        while (true) {
            // Update semua subsistem
            updateSubsystems();
            
            // Cek kondisi keamanan
            checkSafetyConditions();
            
            // Periksa dan terima perintah
            processCommands();
            
            // Switch berdasarkan status sistem
            switch (systemStatus_) {
                case Kamikaze::SystemStatus::INITIALIZING:
                    // Proses inisialisasi sudah selesai
                    if (performSelfTest()) {
                        systemStatus_ = Kamikaze::SystemStatus::READY;
                    } else {
                        systemStatus_ = Kamikaze::SystemStatus::ERROR;
                    }
                    break;
                    
                case Kamikaze::SystemStatus::READY:
                    // Drone siap untuk menerima perintah
                    // Transisi ke ARMED dilakukan saat menerima perintah ARM
                    break;
                    
                case Kamikaze::SystemStatus::ARMED:
                    // Drone diarm, siap untuk terbang
                    if (armed_) {
                        // Periksa apakah ada perintah takeoff atau navigasi
                        // Jika ada, transisi ke FLYING
                        // Di sini kita asumsikan transisi berdasarkan perubahan ketinggian
                        const Kamikaze::Navigation::GpsData& gpsData = 
                            Kamikaze::Navigation::Gps::getInstance().getData();
                        
                        if (gpsData.position.altitude > Kamikaze::SAFETY_ALTITUDE_M) {
                            systemStatus_ = Kamikaze::SystemStatus::FLYING;
                        }
                    } else {
                        // Jika motor tidak armed lagi, kembali ke READY
                        systemStatus_ = Kamikaze::SystemStatus::READY;
                    }
                    break;
                    
                case Kamikaze::SystemStatus::FLYING:
                    // Drone sedang terbang
                    if (armed_) {
                        // Periksa jika sedang mencari target
                        if (Kamikaze::Targeting::TargetDetection::getInstance().update()) {
                            // Target detection berhasil update, cek jika ada target
                            if (checkAndUpdateTarget()) {
                                systemStatus_ = Kamikaze::SystemStatus::TARGETING;
                            }
                        }
                    } else {
                        // Jika motor tidak armed lagi, kembali ke READY
                        systemStatus_ = Kamikaze::SystemStatus::READY;
                    }
                    break;
                    
                case Kamikaze::SystemStatus::TARGETING:
                    // Drone sedang mencari/mengunci target
                    if (armed_ && targetLocked_) {
                        // Periksa jika target sudah terkunci dan dalam jarak serangan
                        if (Kamikaze::Safety::SafetyManager::getInstance().isInAttackRange()) {
                            // Jika aman untuk menyerang
                            if (Kamikaze::Safety::SafetyManager::getInstance().isSafeToAttack()) {
                                systemStatus_ = Kamikaze::SystemStatus::ATTACKING;
                                inAttackMode_ = true;
                            }
                        }
                    } else if (!targetLocked_) {
                        // Jika target hilang, kembali ke FLYING untuk mencari target lagi
                        systemStatus_ = Kamikaze::SystemStatus::FLYING;
                    }
                    break;
                    
                case Kamikaze::SystemStatus::ATTACKING:
                    // Drone menjalankan misi utama (kamikaze)
                    if (inAttackMode_) {
                        // Eksekusi serangan
                        executeAttack();
                    } else {
                        // Jika serangan dibatalkan, kembali ke TARGETING
                        systemStatus_ = Kamikaze::SystemStatus::TARGETING;
                    }
                    break;
                    
                case Kamikaze::SystemStatus::EMERGENCY:
                    // Mode emergency
                    handleEmergency();
                    break;
                    
                case Kamikaze::SystemStatus::ERROR:
                    // Terjadi kesalahan
                    handleError();
                    break;
                    
                default:
                    // Status tidak dikenal
                    systemStatus_ = Kamikaze::SystemStatus::ERROR;
                    break;
            }
            
            // Delay untuk task
            Kamikaze::Core::Rtos::delay(Kamikaze::SYSTEM_TICK_MS);
        }
        
        return 0U;
    }

private:
    // Status dan flag
    Kamikaze::SystemStatus systemStatus_;
    bool armed_;
    bool targetLocked_;
    bool inAttackMode_;
    
    // Mencegah copy constructor dan assignment operator
    MainTask(const MainTask&);
    MainTask& operator=(const MainTask&);
    
    /**
     * @brief Update semua subsistem
     */
    void updateSubsystems() {
        // Update komunikasi
        Kamikaze::Communication::Radio::getInstance().processReceivedMessages();
        Kamikaze::Communication::Telemetry::getInstance().update();
        Kamikaze::Communication::Telemetry::getInstance().send();
        
        // Update navigasi
        Kamikaze::Navigation::Imu::getInstance().update();
        Kamikaze::Navigation::Gps::getInstance().update();
        
        // Update targeting
        Kamikaze::Targeting::TargetDetection::getInstance().update();
        
        // Update safety
        Kamikaze::Safety::SafetyManager::getInstance().update();
    }
    
    /**
     * @brief Cek kondisi keamanan dan transisi ke mode emergency jika perlu
     */
    void checkSafetyConditions() {
        Kamikaze::Safety::SafetyManager& safetyManager = 
            Kamikaze::Safety::SafetyManager::getInstance();
        
        // Jika mode keamanan kritis atau darurat, transisi ke EMERGENCY
        if (safetyManager.getMode() == Kamikaze::Safety::SafetyMode::CRITICAL || 
            safetyManager.getMode() == Kamikaze::Safety::SafetyMode::EMERGENCY) {
            systemStatus_ = Kamikaze::SystemStatus::EMERGENCY;
        }
    }
    
    /**
     * @brief Proses perintah dari ground station
     */
    void processCommands() {
        // Dapatkan perintah terakhir
        const Kamikaze::Communication::Command& command = 
            Kamikaze::Communication::Radio::getInstance().getLastCommand();
        
        // Proses perintah berdasarkan tipe
        switch (command.type) {
            case Kamikaze::Communication::CommandType::ARM:
                if (systemStatus_ == Kamikaze::SystemStatus::READY) {
                    armed_ = true;
                    systemStatus_ = Kamikaze::SystemStatus::ARMED;
                }
                break;
                
            case Kamikaze::Communication::CommandType::DISARM:
                armed_ = false;
                if (systemStatus_ != Kamikaze::SystemStatus::EMERGENCY && 
                    systemStatus_ != Kamikaze::SystemStatus::ERROR) {
                    systemStatus_ = Kamikaze::SystemStatus::READY;
                }
                break;
                
            case Kamikaze::Communication::CommandType::ABORT:
                // Batalkan misi dan kembali ke mode aman
                inAttackMode_ = false;
                if (systemStatus_ == Kamikaze::SystemStatus::ATTACKING) {
                    systemStatus_ = Kamikaze::SystemStatus::TARGETING;
                }
                break;
                
            case Kamikaze::Communication::CommandType::ATTACK:
                // Aktivasi mode serangan jika dalam keadaan targeting
                if (systemStatus_ == Kamikaze::SystemStatus::TARGETING && targetLocked_) {
                    inAttackMode_ = true;
                    systemStatus_ = Kamikaze::SystemStatus::ATTACKING;
                }
                break;
                
            // Proses perintah lainnya...
            
            default:
                // Perintah tidak dikenal, abaikan
                break;
        }
    }
    
    /**
     * @brief Self-test untuk memastikan semua sistem berfungsi
     * @return true jika semua sistem berfungsi dengan baik
     */
    bool performSelfTest() {
        return Kamikaze::SystemInit::getInstance().performSelfTest();
    }
    
    /**
     * @brief Cek dan update status target
     * @return true jika target terdeteksi dan valid
     */
    bool checkAndUpdateTarget() {
        Kamikaze::Targeting::TargetDetection& targeting = 
            Kamikaze::Targeting::TargetDetection::getInstance();
        
        // Cek status target
        const Kamikaze::Targeting::TargetData& targetData = targeting.getTargetData();
        
        // Update flag target locked
        targetLocked_ = targeting.isTargetLocked();
        
        // Cek kondisi target dari safety manager
        Kamikaze::Safety::SafetyManager::getInstance().checkTargetConditions();
        
        return targetLocked_;
    }
    
    /**
     * @brief Eksekusi serangan
     */
    void executeAttack() {
        // Dalam implementasi nyata, ini akan mengontrol drone untuk meluncur ke target
        
        // Dapatkan data target
        const Kamikaze::Targeting::TargetData& targetData = 
            Kamikaze::Targeting::TargetDetection::getInstance().getTargetData();
        
        // Dapatkan data IMU dan GPS
        const Kamikaze::Navigation::ImuData& imuData = 
            Kamikaze::Navigation::Imu::getInstance().getData();
        
        const Kamikaze::Navigation::GpsData& gpsData = 
            Kamikaze::Navigation::Gps::getInstance().getData();
        
        // Hitung arah dan sudut serangan
        // ...
        
        // Kontrol motor untuk serangan
        Kamikaze::Control::MotorController& motorController = 
            Kamikaze::Control::MotorController::getInstance();
        
        // Set kecepatan motor untuk serangan
        // Dalam contoh ini kita gunakan kecepatan maksimum untuk semua motor
        motorController.setAllSpeeds(100.0F);
    }
    
    /**
     * @brief Tangani keadaan darurat
     */
    void handleEmergency() {
        // Dapatkan tindakan yang direkomendasikan
        Kamikaze::Safety::SafetyAction action = 
            Kamikaze::Safety::SafetyManager::getInstance().getRecommendedAction(
                Kamikaze::Safety::SafetyManager::getInstance().getMode()
            );
        
        // Eksekusi tindakan keamanan
        Kamikaze::Safety::SafetyManager::getInstance().executeAction(action);
        
        // Reset flag
        inAttackMode_ = false;
        
        // Dalam kasus darurat, kita mungkin perlu disarm
        if (action == Kamikaze::Safety::SafetyAction::DISARM || 
            action == Kamikaze::Safety::SafetyAction::ABORT) {
            armed_ = false;
        }
    }
    
    /**
     * @brief Tangani keadaan error
     */
    void handleError() {
        // Matikan semua motor
        Kamikaze::Control::MotorController::getInstance().stopAll();
        
        // Reset flag
        armed_ = false;
        inAttackMode_ = false;
        targetLocked_ = false;
        
        // Kirim telemetri error
        Kamikaze::Communication::Telemetry::getInstance().update();
        Kamikaze::Communication::Telemetry::getInstance().send();
    }
};

/**
 * @brief Fungsi utama program
 */
int main() {
    // Inisialisasi sistem
    if (!Kamikaze::SystemInit::getInstance().initializeSystem()) {
        // Jika inisialisasi gagal, kita harus keluar
        return -1;
    }
    
    // Konfigurasi hardware
    if (!Kamikaze::SystemInit::getInstance().configureGpio() ||
        !Kamikaze::SystemInit::getInstance().configureUart()) {
        // Jika konfigurasi hardware gagal
        return -1;
    }
    
    // Inisialisasi subsistem lain
    if (!Kamikaze::Navigation::initializeNavigation() ||
        !Kamikaze::Control::initializeControl() ||
        !Kamikaze::Communication::initializeCommunication() ||
        !Kamikaze::Targeting::initializeTargeting() ||
        !Kamikaze::Safety::initializeSafety()) {
        // Jika subsistem gagal diinisialisasi
        return -1;
    }
    
    // Buat main task
    MainTask mainTask;
    
    // Register main task ke RTOS
    Kamikaze::Uint32 mainTaskId = Kamikaze::Core::Rtos::getInstance().registerTask(
        &mainTask, 
        Kamikaze::Core::TaskPriority::HIGH
    );
    
    if (mainTaskId == 0U) {
        // Jika gagal mendaftarkan task
        return -1;
    }
    
    // Mulai RTOS scheduler
    if (!Kamikaze::Core::Rtos::getInstance().start()) {
        // Jika gagal memulai scheduler
        return -1;
    }
    
    // Program tidak akan pernah mencapai titik ini jika RTOS berjalan dengan baik
    return 0;
} 