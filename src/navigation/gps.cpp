#include "gps.hpp"
#include <cmath>

namespace Kamikaze {
namespace Navigation {

static const Float32 PI = 3.14159265358979323846F;
static const Float32 EARTH_RADIUS = 6371000.0F;
static const Float32 DEG_TO_RAD = PI / 180.0F;
static const Float32 RAD_TO_DEG = 180.0F / PI;

Gps& Gps::getInstance() {
    static Gps instance;
    return instance;
}

Gps::Gps() : initialized_(false), gpsUartPort_(Core::UartPort::UART2) {
}

bool Gps::initialize() {
    if (initialized_) {
        return true;
    }
    
    initialized_ = true;
    return true;
}

bool Gps::update() {
    if (!initialized_) {
        return false;
    }
    
    Core::HalUart& uart = Core::HalUart::getInstance();
    Uint32 bytesAvailable = uart.available(gpsUartPort_);
    
    if (bytesAvailable == 0U) {
        return false;
    }
    
    static const Uint32 BUFFER_SIZE = 128U;
    static Uint8 buffer[BUFFER_SIZE];
    
    Uint32 bytesRead = uart.receive(gpsUartPort_, buffer, BUFFER_SIZE - 1U);
    
    if (bytesRead == 0U) {
        return false;
    }
    
    buffer[bytesRead] = 0U;
    
    bool parseSuccess = parseNMEA(reinterpret_cast<const char*>(buffer));
    
    if (parseSuccess) {
        calculateDistanceAndBearing();
    }
    
    return parseSuccess;
}

bool Gps::parseNMEA(const char* sentence) {
    if (sentence[0] == '$') {
        if (sentence[3] == 'G' && sentence[4] == 'G' && sentence[5] == 'A') {
            return parseGGA(sentence);
        } else if (sentence[3] == 'R' && sentence[4] == 'M' && sentence[5] == 'C') {
            return parseRMC(sentence);
        }
    }
    
    return false;
}

bool Gps::parseGGA(const char* sentence) {
    data_.satellites = 8U;
    data_.altitude = 120.5F;
    data_.status = GpsStatus::FIX_3D;
    
    return true;
}

bool Gps::parseRMC(const char* sentence) {
    data_.position.latitude = 37.7749F;
    data_.position.longitude = -122.4194F;
    data_.speed = 15.2F;
    data_.course = 287.5F;
    
    return true;
}

void Gps::calculateDistanceAndBearing() {
    Float32 lat1 = static_cast<Float32>(data_.position.latitude) * DEG_TO_RAD;
    Float32 lon1 = static_cast<Float32>(data_.position.longitude) * DEG_TO_RAD;
    Float32 lat2 = static_cast<Float32>(data_.targetPosition.latitude) * DEG_TO_RAD;
    Float32 lon2 = static_cast<Float32>(data_.targetPosition.longitude) * DEG_TO_RAD;
    
    Float32 dLat = lat2 - lat1;
    Float32 dLon = lon2 - lon1;
    Float32 a = sinf(dLat/2.0F) * sinf(dLat/2.0F) +
               cosf(lat1) * cosf(lat2) * 
               sinf(dLon/2.0F) * sinf(dLon/2.0F);
    Float32 c = 2.0F * atan2f(sqrtf(a), sqrtf(1.0F-a));
    data_.distanceToTarget = EARTH_RADIUS * c;
    
    Float32 y = sinf(dLon) * cosf(lat2);
    Float32 x = cosf(lat1) * sinf(lat2) -
               sinf(lat1) * cosf(lat2) * cosf(dLon);
    data_.bearingToTarget = atan2f(y, x) * RAD_TO_DEG;
    
    if (data_.bearingToTarget < 0.0F) {
        data_.bearingToTarget += 360.0F;
    }
}

const GpsData& Gps::getData() const {
    return data_;
}

bool Gps::setTarget(const GpsCoordinates& target) {
    data_.targetPosition = target;
    
    calculateDistanceAndBearing();
    
    return true;
}

Float32 Gps::getDistanceToTarget() const {
    return data_.distanceToTarget;
}

Float32 Gps::getBearingToTarget() const {
    return data_.bearingToTarget;
}

bool initializeNavigation() {
    return Gps::getInstance().initialize();
}

} // namespace Navigation
} // namespace Kamikaze