#include "telemetry.hpp"
#include "../core/rtos.hpp"

namespace Kamikaze {
namespace Communication {


Telemetry& Telemetry::getInstance() {
    static Telemetry instance;
    return instance;
}


Telemetry::Telemetry() :
    attitudeData_(),
    gpsData_(),
    batteryData_(),
    systemData_(),
    missionData_(),
    targetData_(),
    updateRateMs_(TELEMETRY_UPDATE_MS),
    lastUpdateMs_(0U),
    initialized_(false) {
    attitudeData_.type = TelemetryType::ATTITUDE;
    gpsData_.type = TelemetryType::GPS;
    batteryData_.type = TelemetryType::BATTERY;
    systemData_.type = TelemetryType::SYSTEM;
    missionData_.type = TelemetryType::MISSION;
    targetData_.type = TelemetryType::TARGET;
}


bool Telemetry::initialize() {
    if (initialized_) {
        return true;
    }
    

    lastUpdateMs_ = 0U;
    

    updateSystem();
    
    initialized_ = true;
    return true;
}


bool Telemetry::update() {
    if (!initialized_) {
        return false;
    }
    

    Uint32 currentTime = Core::Rtos::getTicks();
    if ((currentTime - lastUpdateMs_) < updateRateMs_) {
        return false;
    }
    

    lastUpdateMs_ = currentTime;
    

    bool result = true;
    result = result && updateAttitude();
    result = result && updateGps();
    result = result && updateBattery();
    result = result && updateSystem();
    result = result && updateMission();
    result = result && updateTarget();
    
    return result;
}


bool Telemetry::send() {
    bool result = true;
    

    result = result && sendData(TelemetryType::ATTITUDE);
    result = result && sendData(TelemetryType::GPS);
    result = result && sendData(TelemetryType::BATTERY);
    result = result && sendData(TelemetryType::SYSTEM);
    result = result && sendData(TelemetryType::MISSION);
    result = result && sendData(TelemetryType::TARGET);
    
    return result;
}


bool Telemetry::sendData(TelemetryType type) {
    if (!initialized_) {
        return false;
    }
    

    const TelemetryData& data = getData(type);
    

    RadioMessage message;
    if (!encodeData(data, message)) {
        return false;
    }
    
    // Kirim pesan melalui radio
    return Radio::getInstance().sendMessage(message);
}

// Set update rate
void Telemetry::setUpdateRate(Uint32 rateMs) {
    if (rateMs > 0U) {
        updateRateMs_ = rateMs;
    }
}

// Mendapatkan data telemetri
const TelemetryData& Telemetry::getData(TelemetryType type) const {
    switch (type) {
        case TelemetryType::ATTITUDE:
            return attitudeData_;
            
        case TelemetryType::GPS:
            return gpsData_;
            
        case TelemetryType::BATTERY:
            return batteryData_;
            
        case TelemetryType::MISSION:
            return missionData_;
            
        case TelemetryType::TARGET:
            return targetData_;
            
        case TelemetryType::SYSTEM:
        default:
            return systemData_;
    }
}

// Update data sikap dari IMU
bool Telemetry::updateAttitude() {

    attitudeData_.timestamp = Core::Rtos::getTicks();
    
    // Ambil data dari IMU
    const Navigation::ImuData& imuData = Navigation::Imu::getInstance().getData();
    
    // Set data
    attitudeData_.data.attitude.roll = imuData.orientation.x;
    attitudeData_.data.attitude.pitch = imuData.orientation.y;
    attitudeData_.data.attitude.yaw = imuData.orientation.z;
    
    return true;
}

// Update data GPS
bool Telemetry::updateGps() {

    gpsData_.timestamp = Core::Rtos::getTicks();
    
    // Ambil data GPS
    const Navigation::GpsData& gpsData = Navigation::Gps::getInstance().getData();
    
    // Set data
    gpsData_.data.gps.latitude = gpsData.position.latitude;
    gpsData_.data.gps.longitude = gpsData.position.longitude;
    gpsData_.data.gps.altitude = gpsData.position.altitude;
    gpsData_.data.gps.speed = gpsData.speed;
    gpsData_.data.gps.satellites = gpsData.satellites;
    
    return true;
}

// Update data baterai
bool Telemetry::updateBattery() {

    batteryData_.timestamp = Core::Rtos::getTicks();
    
    // Baca status baterai - implementasi dummy untuk contoh
    batteryData_.data.battery.voltage = 11.8F;
    batteryData_.data.battery.current = 5.2F;
    batteryData_.data.battery.remaining = 75.0F;
    
    return true;
}

// Update status sistem
bool Telemetry::updateSystem() {

    systemData_.timestamp = Core::Rtos::getTicks();
    
    // Set data sistem - implementasi dummy untuk contoh
    systemData_.data.system.status = SystemStatus::READY;
    systemData_.data.system.error = ErrorCode::NONE;
    systemData_.data.system.uptime = Core::Rtos::getTicks();
    
    return true;
}

// Update data misi
bool Telemetry::updateMission() {

    missionData_.timestamp = Core::Rtos::getTicks();
    
    // Set data misi - implementasi dummy untuk contoh
    missionData_.data.mission.missionId = 1U;
    missionData_.data.mission.waypointIndex = 0U;
    missionData_.data.mission.progress = 0.0F;
    missionData_.data.mission.timeToTarget = 0.0F;
    
    return true;
}

// Update data target
bool Telemetry::updateTarget() {

    targetData_.timestamp = Core::Rtos::getTicks();
    
    // Ambil data target
    const Targeting::TargetData& targetData = Targeting::TargetDetection::getInstance().getTargetData();
    
    // Set data
    targetData_.data.target.latitude = targetData.position.latitude;
    targetData_.data.target.longitude = targetData.position.longitude;
    targetData_.data.target.distance = targetData.distance;
    targetData_.data.target.bearing = targetData.bearing;
    targetData_.data.target.confidence = targetData.confidence;
    targetData_.data.target.status = static_cast<Uint8>(targetData.status);
    targetData_.data.target.type = static_cast<Uint8>(targetData.type);
    targetData_.data.target.speed = targetData.speed;
    
    // Hitung estimasi waktu menuju target
    targetData_.data.target.timeToIntercept = calculateTimeToIntercept();
    
    return true;
}

// Hitung estimasi waktu menuju target
Float32 Telemetry::calculateTimeToIntercept() {
    // Dapatkan data target
    const Targeting::TargetData& targetData = Targeting::TargetDetection::getInstance().getTargetData();
    
    // Jika tidak ada target atau jarak adalah nol, return 0
    if (targetData.status == Targeting::TargetStatus::NO_TARGET ||
        targetData.distance < 0.1F) {
        return 0.0F;
    }
    
    // Dapatkan data GPS untuk kecepatan drone
    const Navigation::GpsData& gpsData = Navigation::Gps::getInstance().getData();
    
    // Jika drone tidak bergerak, gunakan kecepatan default
    Float32 droneSpeed = gpsData.speed;
    if (droneSpeed < 0.1F) {
        droneSpeed = 10.0F; // Default 10 m/s jika drone tidak bergerak
    }
    
    // Hitung estimasi waktu berdasarkan jarak dan kecepatan
    // Untuk simulasi sederhana, kita asumsikan drone bergerak langsung ke target
    // Dalam implementasi nyata, ini akan mempertimbangkan rute dan manuver
    return targetData.distance / droneSpeed;
}

// Encode data telemetri ke pesan radio
bool Telemetry::encodeData(const TelemetryData& data, RadioMessage& message) {
    // Set tipe pesan
    message.type = RadioMessageType::TELEMETRY;
    
    // Encode tipe telemetri dan timestamp
    message.payload[0] = static_cast<Uint8>(data.type);
    
    // Encode timestamp (4 bytes)
    message.payload[1] = static_cast<Uint8>((data.timestamp >> 24) & 0xFFU);
    message.payload[2] = static_cast<Uint8>((data.timestamp >> 16) & 0xFFU);
    message.payload[3] = static_cast<Uint8>((data.timestamp >> 8) & 0xFFU);
    message.payload[4] = static_cast<Uint8>(data.timestamp & 0xFFU);
    
    // Encode data berdasarkan tipe
    switch (data.type) {
        case TelemetryType::ATTITUDE:
            // Encode attitude data
            // Dalam implementasi nyata, ini akan mengonversi float ke bytes
            message.payload[5] = static_cast<Uint8>(data.data.attitude.roll);
            message.payload[9] = static_cast<Uint8>(data.data.attitude.pitch);
            message.payload[13] = static_cast<Uint8>(data.data.attitude.yaw);
            message.length = 17U;
            break;
            
        case TelemetryType::GPS:
            // Encode GPS data
            message.payload[5] = static_cast<Uint8>(data.data.gps.latitude);
            message.payload[13] = static_cast<Uint8>(data.data.gps.longitude);
            message.payload[21] = static_cast<Uint8>(data.data.gps.altitude);
            message.payload[25] = static_cast<Uint8>(data.data.gps.speed);
            message.payload[29] = data.data.gps.satellites;
            message.length = 30U;
            break;
            
        case TelemetryType::BATTERY:
            // Encode battery data
            message.payload[5] = static_cast<Uint8>(data.data.battery.voltage);
            message.payload[9] = static_cast<Uint8>(data.data.battery.current);
            message.payload[13] = static_cast<Uint8>(data.data.battery.remaining);
            message.length = 17U;
            break;
            
        case TelemetryType::SYSTEM:
            // Encode system data
            message.payload[5] = static_cast<Uint8>(data.data.system.status);
            message.payload[6] = static_cast<Uint8>(data.data.system.error);
            
            // Encode uptime (4 bytes)
            message.payload[7] = static_cast<Uint8>((data.data.system.uptime >> 24) & 0xFFU);
            message.payload[8] = static_cast<Uint8>((data.data.system.uptime >> 16) & 0xFFU);
            message.payload[9] = static_cast<Uint8>((data.data.system.uptime >> 8) & 0xFFU);
            message.payload[10] = static_cast<Uint8>(data.data.system.uptime & 0xFFU);
            
            message.length = 11U;
            break;
            
        case TelemetryType::MISSION:
            // Encode mission data
            message.payload[5] = data.data.mission.missionId;
            message.payload[6] = data.data.mission.waypointIndex;
            message.payload[7] = static_cast<Uint8>(data.data.mission.progress);
            message.payload[11] = static_cast<Uint8>(data.data.mission.timeToTarget);
            message.length = 15U;
            break;
            
        case TelemetryType::TARGET:
            // Encode target data
            // Latitude (8 bytes)
            Float64 lat = data.data.target.latitude;
            message.payload[5] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lat) >> 56) & 0xFFU);
            message.payload[6] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lat) >> 48) & 0xFFU);
            message.payload[7] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lat) >> 40) & 0xFFU);
            message.payload[8] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lat) >> 32) & 0xFFU);
            message.payload[9] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lat) >> 24) & 0xFFU);
            message.payload[10] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lat) >> 16) & 0xFFU);
            message.payload[11] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lat) >> 8) & 0xFFU);
            message.payload[12] = static_cast<Uint8>(reinterpret_cast<Uint64&>(lat) & 0xFFU);
            
            // Longitude (8 bytes)
            Float64 lon = data.data.target.longitude;
            message.payload[13] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lon) >> 56) & 0xFFU);
            message.payload[14] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lon) >> 48) & 0xFFU);
            message.payload[15] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lon) >> 40) & 0xFFU);
            message.payload[16] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lon) >> 32) & 0xFFU);
            message.payload[17] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lon) >> 24) & 0xFFU);
            message.payload[18] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lon) >> 16) & 0xFFU);
            message.payload[19] = static_cast<Uint8>((reinterpret_cast<Uint64&>(lon) >> 8) & 0xFFU);
            message.payload[20] = static_cast<Uint8>(reinterpret_cast<Uint64&>(lon) & 0xFFU);
            
            // Distance (4 bytes)
            Float32 dist = data.data.target.distance;
            message.payload[21] = static_cast<Uint8>((reinterpret_cast<Uint32&>(dist) >> 24) & 0xFFU);
            message.payload[22] = static_cast<Uint8>((reinterpret_cast<Uint32&>(dist) >> 16) & 0xFFU);
            message.payload[23] = static_cast<Uint8>((reinterpret_cast<Uint32&>(dist) >> 8) & 0xFFU);
            message.payload[24] = static_cast<Uint8>(reinterpret_cast<Uint32&>(dist) & 0xFFU);
            
            // Bearing (4 bytes)
            Float32 bear = data.data.target.bearing;
            message.payload[25] = static_cast<Uint8>((reinterpret_cast<Uint32&>(bear) >> 24) & 0xFFU);
            message.payload[26] = static_cast<Uint8>((reinterpret_cast<Uint32&>(bear) >> 16) & 0xFFU);
            message.payload[27] = static_cast<Uint8>((reinterpret_cast<Uint32&>(bear) >> 8) & 0xFFU);
            message.payload[28] = static_cast<Uint8>(reinterpret_cast<Uint32&>(bear) & 0xFFU);
            
            // Confidence, status, type (1 byte each)
            message.payload[29] = static_cast<Uint8>(data.data.target.confidence);
            message.payload[30] = data.data.target.status;
            message.payload[31] = data.data.target.type;
            
            message.length = 32U;
            break;
            
        default:
            // Tipe tidak dikenal
            return false;
    }
    
    return true;
}

} // namespace Communication
} // namespace Kamikaze