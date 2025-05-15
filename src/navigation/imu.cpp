#include "imu.hpp"
#include <cmath>

namespace Kamikaze {
namespace Navigation {

const Float32 Imu::CALIBRATION_SAMPLES = 100.0F;

Imu& Imu::getInstance() {
    static Imu instance;
    return instance;
}

Imu::Imu() : initialized_(false) {
}

bool Imu::initialize() {
    if (initialized_) {
        return true;
    }
    
    bool calibrationResult = calibrate();
    
    initialized_ = calibrationResult;
    return calibrationResult;
}

bool Imu::calibrate() {
    if (!initialized_ && data_.status == ImuStatus::CALIBRATING) {
        data_.status = ImuStatus::CALIBRATING;
        
        Vector3D accelSum;
        Vector3D gyroSum;
        Vector3D magSum;
        
        for (Uint32 i = 0U; i < static_cast<Uint32>(CALIBRATION_SAMPLES); ++i) {
            Vector3D accel(0.0F, 0.0F, 9.81F);
            Vector3D gyro(0.0F, 0.0F, 0.0F);
            Vector3D mag(1.0F, 0.0F, 0.0F);
            
            accelSum.x += accel.x;
            accelSum.y += accel.y;
            accelSum.z += accel.z;
            
            gyroSum.x += gyro.x;
            gyroSum.y += gyro.y;
            gyroSum.z += gyro.z;
            
            magSum.x += mag.x;
            magSum.y += mag.y;
            magSum.z += mag.z;
        }
        
        data_.status = ImuStatus::READY;
        
        return true;
    }
    
    return false;
}

bool Imu::update() {
    if (!initialized_) {
        return false;
    }
    
    if (data_.status != ImuStatus::READY) {
        return false;
    }
    
    data_.acceleration.x = 0.0F;
    data_.acceleration.y = 0.0F;
    data_.acceleration.z = 9.81F;
    
    data_.gyroscope.x = 0.0F;
    data_.gyroscope.y = 0.0F;
    data_.gyroscope.z = 0.01F;
    
    data_.magnetometer.x = 1.0F;
    data_.magnetometer.y = 0.0F;
    data_.magnetometer.z = 0.0F;
    
    data_.temperature = 25.0F;
    
    updateOrientation();
    
    return true;
}

void Imu::updateOrientation() {
    Float32 roll = atan2f(data_.acceleration.y, data_.acceleration.z);
    Float32 pitch = atan2f(-data_.acceleration.x, 
                         sqrtf(data_.acceleration.y * data_.acceleration.y + 
                               data_.acceleration.z * data_.acceleration.z));
    
    Float32 yaw = atan2f(data_.magnetometer.y, data_.magnetometer.x);
    
    data_.orientation.x = roll;
    data_.orientation.y = pitch;
    data_.orientation.z = yaw;
}

const ImuData& Imu::getData() const {
    return data_;
}

Vector3D Imu::getOrientation() const {
    return data_.orientation;
}

Float32 Imu::getRoll() const {
    return data_.orientation.x;
}

Float32 Imu::getPitch() const {
    return data_.orientation.y;
}

Float32 Imu::getYaw() const {
    return data_.orientation.z;
}

} // namespace Navigation
} // namespace Kamikaze