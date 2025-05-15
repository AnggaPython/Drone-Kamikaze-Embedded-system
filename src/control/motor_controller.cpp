#include "motor_controller.hpp"

namespace Kamikaze {
namespace Control {

// Implementasi Motor

Motor::Motor(MotorPosition position) : 
    position_(position),
    speed_(0.0F),
    status_(MotorStatus::IDLE),
    initialized_(false),
    gpioPort_(Core::GpioPort::PORT_B),
    gpioPin_(static_cast<Uint8>(position)) {
    // Inisialisasi pin GPIO berdasarkan posisi motor
}

bool Motor::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Konfigurasi pin GPIO untuk motor
    Core::HalGpio& gpio = Core::HalGpio::getInstance();
    bool result = gpio.configurePin(gpioPort_, gpioPin_, Core::GpioMode::OUTPUT, Core::GpioPull::NONE);
    
    // Set ke kondisi awal (motor berhenti)
    if (result) {
        result = stop();
    }
    
    initialized_ = result;
    return result;
}

bool Motor::setSpeed(Float32 speed) {
    // Validasi parameter
    if (speed < 0.0F) {
        speed = 0.0F;
    } else if (speed > 100.0F) {
        speed = 100.0F;
    }
    
    if (!initialized_) {
        return false;
    }
    
    // Implementasi kontrol PWM untuk motor
    // Dalam contoh ini, kita gunakan GPIO biasa dan representasikan
    // sebagai hidup/mati saja (lebih realistisnya menggunakan PWM hardware)
    
    Core::HalGpio& gpio = Core::HalGpio::getInstance();
    bool motorOn = (speed > 0.0F);
    bool result = gpio.writePin(gpioPort_, gpioPin_, motorOn);
    
    if (result) {
        speed_ = speed;
        status_ = (motorOn) ? MotorStatus::RUNNING : MotorStatus::IDLE;
    }
    
    return result;
}

Float32 Motor::getSpeed() const {
    return speed_;
}

bool Motor::stop() {
    return setSpeed(0.0F);
}

MotorStatus Motor::getStatus() const {
    return status_;
}

// Implementasi MotorController

MotorController& MotorController::getInstance() {
    static MotorController instance;
    return instance;
}

MotorController::MotorController() :
    motors_{
        Motor(MotorPosition::FRONT_LEFT),
        Motor(MotorPosition::FRONT_RIGHT),
        Motor(MotorPosition::REAR_LEFT),
        Motor(MotorPosition::REAR_RIGHT)
    },
    initialized_(false) {
    // Inisialisasi array motor
}

bool MotorController::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Inisialisasi semua motor
    bool result = true;
    
    for (Uint8 i = 0U; i < MAX_MOTOR_COUNT; ++i) {
        result = result && motors_[i].initialize();
    }
    
    // Pastikan semua motor berhenti
    if (result) {
        result = stopAll();
    }
    
    initialized_ = result;
    return result;
}

bool MotorController::setSpeeds(Float32 frontLeft, Float32 frontRight, 
                              Float32 rearLeft, Float32 rearRight) {
    if (!initialized_) {
        return false;
    }
    
    // Set kecepatan untuk masing-masing motor
    bool result = true;
    
    result = result && motors_[static_cast<Uint8>(MotorPosition::FRONT_LEFT)].setSpeed(frontLeft);
    result = result && motors_[static_cast<Uint8>(MotorPosition::FRONT_RIGHT)].setSpeed(frontRight);
    result = result && motors_[static_cast<Uint8>(MotorPosition::REAR_LEFT)].setSpeed(rearLeft);
    result = result && motors_[static_cast<Uint8>(MotorPosition::REAR_RIGHT)].setSpeed(rearRight);
    
    return result;
}

bool MotorController::setAllSpeeds(Float32 speed) {
    return setSpeeds(speed, speed, speed, speed);
}

bool MotorController::stopAll() {
    return setAllSpeeds(0.0F);
}

Motor& MotorController::getMotor(MotorPosition position) {
    return motors_[static_cast<Uint8>(position)];
}

// Inisialisasi modul Control
bool initializeControl() {
    return MotorController::getInstance().initialize();
}

} // namespace Control
} // namespace Kamikaze 