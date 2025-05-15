#include "target_detection.hpp"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include "../core/rtos.hpp"

namespace Kamikaze {
namespace Targeting {

const Float32 TargetDetection::MIN_CONFIDENCE_THRESHOLD = 70.0F;
const Float32 TargetDetection::MAX_TARGET_LOST_TIME_MS = 5000.0F;


TargetDetection& TargetDetection::getInstance() {
    static TargetDetection instance;
    return instance;
}


TargetDetection::TargetDetection() : 
    targetData_(),
    initialized_(false) {
    srand(static_cast<unsigned int>(time(nullptr)));
}


bool TargetDetection::initialize() {
    if (initialized_) {
        return true;
    }
    

    clearTarget();
    
    initialized_ = true;
    return true;
}


bool TargetDetection::update() {
    if (!initialized_) {
        return false;
    }
    

    if (targetData_.status == TargetStatus::NO_TARGET) {
        if (detectTargetWithSensors()) {
            targetData_.status = TargetStatus::TARGET_ACQUIRING;
        }
    } 
    else if (targetData_.status == TargetStatus::TARGET_ACQUIRING) {
        detectTargetWithSensors();
        
        if (targetData_.confidence >= MIN_CONFIDENCE_THRESHOLD) {
            targetData_.status = TargetStatus::TARGET_LOCKED;
        }
    }
    else if (targetData_.status == TargetStatus::TARGET_LOCKED) {
        if (targetData_.type == TargetType::MOVING) {
            Uint32 currentTime = Core::Rtos::getTicks();
            Uint32 deltaTimeMs = currentTime - targetData_.lastUpdateTime;
            Float32 deltaTimeSec = static_cast<Float32>(deltaTimeMs) / 1000.0F;
            
            Float32 distanceMoved = targetData_.speed * deltaTimeSec;
            
            const Float32 PI = 3.14159265358979323846F;
            const Float32 DEG_TO_RAD = PI / 180.0F;
            Float32 headingRad = targetData_.heading * DEG_TO_RAD;
            
            Float32 latChange = (distanceMoved * cosf(headingRad)) / 111111.0F;
            Float32 lonChange = (distanceMoved * sinf(headingRad)) / 
                              (111111.0F * cosf(static_cast<Float32>(targetData_.position.latitude) * DEG_TO_RAD));
            
            targetData_.position.latitude += static_cast<Float64>(latChange);
            targetData_.position.longitude += static_cast<Float64>(lonChange);
            
            targetData_.lastUpdateTime = currentTime;
        }
        Uint32 currentTime = Core::Rtos::getTicks();
        if ((currentTime - targetData_.lastUpdateTime) > static_cast<Uint32>(MAX_TARGET_LOST_TIME_MS)) {
            targetData_.status = TargetStatus::TARGET_LOST;
        }
    }
    // Jika target hilang, coba dapatkan kembali
    else if (targetData_.status == TargetStatus::TARGET_LOST) {
        // Coba deteksi target dengan sensor lagi
        if (detectTargetWithSensors()) {
            targetData_.status = TargetStatus::TARGET_ACQUIRING;
        } else {
            // Jika tidak berhasil dalam waktu tertentu, bersihkan target
            Uint32 currentTime = Core::Rtos::getTicks();
            if ((currentTime - targetData_.lastUpdateTime) > (2U * static_cast<Uint32>(MAX_TARGET_LOST_TIME_MS))) {
                clearTarget();
            }
        }
    }
    
    // Update jarak dan arah ke target
    updateTargetVectors();
    
    return true;
}

// Mendapatkan data target
const TargetData& TargetDetection::getTargetData() const {
    return targetData_;
}

// Set target manual
bool TargetDetection::setTarget(const GpsCoordinates& targetPosition, TargetType targetType) {
    if (!initialized_) {
        return false;
    }
    
    // Set data target
    targetData_.position = targetPosition;
    targetData_.type = targetType;
    targetData_.status = TargetStatus::TARGET_LOCKED;
    targetData_.confidence = 100.0F;  // Target manual selalu 100% confidence
    targetData_.lastUpdateTime = Core::Rtos::getTicks();
    
    // Kalkulasi jarak dan arah ke target
    updateTargetVectors();
    
    return true;
}

// Batalkan target
bool TargetDetection::clearTarget() {
    if (!initialized_) {
        return false;
    }
    
 ke default
    targetData_ = TargetData();
    
    return true;
}

// Cek apakah target terkunci
bool TargetDetection::isTargetLocked() const {
    return (targetData_.status == TargetStatus::TARGET_LOCKED);
}

// Cek apakah drone dalam jarak serangan
bool TargetDetection::isInAttackRange(Float32 attackRange) const {
    // Target dianggap valid jika:
    // 1. Status target adalah TARGET_LOCKED, atau
    // 2. Status target adalah TARGET_ACQUIRING dengan confidence cukup tinggi
    if (targetData_.status != TargetStatus::TARGET_LOCKED && 
        !(targetData_.status == TargetStatus::TARGET_ACQUIRING && 
          targetData_.confidence >= MIN_CONFIDENCE_THRESHOLD)) {
        return false;
    }
    
    // Cek jarak ke target
    return (targetData_.distance <= attackRange);
}

// Update jarak dan arah ke target
void TargetDetection::updateTargetVectors() {
    // Skip jika tidak ada target
    if (targetData_.status == TargetStatus::NO_TARGET) {
        targetData_.distance = 0.0F;
        targetData_.bearing = 0.0F;
        return;
    }
    
    // Dapatkan posisi drone saat ini
    const Navigation::GpsData& gpsData = Navigation::Gps::getInstance().getData();
    
    // Konstanta untuk konversi
    const Float32 PI = 3.14159265358979323846F;
    const Float32 EARTH_RADIUS = 6371000.0F; // radius bumi dalam meter
    const Float32 DEG_TO_RAD = PI / 180.0F;  // konversi derajat ke radian
    const Float32 RAD_TO_DEG = 180.0F / PI;  // konversi radian ke derajat
    
    // Konversi koordinat ke radian
    Float32 lat1 = static_cast<Float32>(gpsData.position.latitude) * DEG_TO_RAD;
    Float32 lon1 = static_cast<Float32>(gpsData.position.longitude) * DEG_TO_RAD;
    Float32 lat2 = static_cast<Float32>(targetData_.position.latitude) * DEG_TO_RAD;
    Float32 lon2 = static_cast<Float32>(targetData_.position.longitude) * DEG_TO_RAD;
    
    // Haversine formula untuk jarak
    Float32 dLat = lat2 - lat1;
    Float32 dLon = lon2 - lon1;
    Float32 a = sinf(dLat/2.0F) * sinf(dLat/2.0F) +
                cosf(lat1) * cosf(lat2) * 
                sinf(dLon/2.0F) * sinf(dLon/2.0F);
    Float32 c = 2.0F * atan2f(sqrtf(a), sqrtf(1.0F-a));
    targetData_.distance = EARTH_RADIUS * c;
    
    // Formula untuk bearing
    Float32 y = sinf(dLon) * cosf(lat2);
    Float32 x = cosf(lat1) * sinf(lat2) -
                sinf(lat1) * cosf(lat2) * cosf(dLon);
    targetData_.bearing = atan2f(y, x) * RAD_TO_DEG;
    
    // Konversi bearing ke 0-360
    if (targetData_.bearing < 0.0F) {
        targetData_.bearing += 360.0F;
    }
    
    // Update waktu kalkulasi terakhir jika target aktif
    if (targetData_.status == TargetStatus::TARGET_LOCKED || 
        targetData_.status == TargetStatus::TARGET_ACQUIRING) {
        targetData_.lastUpdateTime = Core::Rtos::getTicks();
    }
}

// Deteksi target otomatis dengan sensor
bool TargetDetection::detectTargetWithSensors() {
    // Implementasi deteksi target dengan sensor
    // Dalam contoh ini, kita simulasikan deteksi target
    
    // Simulasi deteksi: 75% chance target terdeteksi, 25% chance tidak terdeteksi
    bool targetDetected = ((rand() % 100) < 75);
    
    // Update confidence berdasarkan deteksi
    if (targetDetected) {
        // Naikkan confidence jika target terdeteksi
        targetData_.confidence += 5.0F;
        if (targetData_.confidence > 100.0F) {
            targetData_.confidence = 100.0F;
        }
    } else {
        // Turunkan confidence jika target tidak terdeteksi
        targetData_.confidence -= 10.0F;
        if (targetData_.confidence < 0.0F) {
            targetData_.confidence = 0.0F;
        }
    }
    
    // Simulasi: Jika tidak ada target, buat target dummy
    if (targetData_.status == TargetStatus::NO_TARGET) {
        // Dapatkan posisi drone saat ini
        const Navigation::GpsData& gpsData = Navigation::Gps::getInstance().getData();
        
        // Buat target 500m di depan posisi saat ini
        targetData_.position = gpsData.position;
        targetData_.position.latitude += 0.005;  // Sekitar 500m ke utara
        targetData_.type = TargetType::STATIC;
        targetData_.confidence = 20.0F;  // Mulai dengan confidence rendah
        targetData_.lastUpdateTime = Core::Rtos::getTicks();
        
        return true;
    }
    
    // Update waktu deteksi terakhir
    targetData_.lastUpdateTime = Core::Rtos::getTicks();
    
    return targetDetected;
}

 modul targeting
bool initializeTargeting() {
    return TargetDetection::getInstance().initialize();
}

} // namespace Targeting
} // namespace Kamikaze