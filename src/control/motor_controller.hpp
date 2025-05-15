#ifndef KAMIKAZE_CONTROL_MOTOR_CONTROLLER_HPP_
#define KAMIKAZE_CONTROL_MOTOR_CONTROLLER_HPP_

#include "../../include/common.hpp"
#include "../core/hal_gpio.hpp"

namespace Kamikaze {
namespace Control {

/**
 * @brief Enum untuk status motor
 */
enum class MotorStatus : Uint8 {
    IDLE = 0U,          // Motor dalam keadaan diam
    RUNNING = 1U,       // Motor sedang berjalan
    ERROR = 2U          // Error pada motor
};

/**
 * @brief Enum untuk posisi motor
 */
enum class MotorPosition : Uint8 {
    FRONT_LEFT = 0U,    // Motor depan kiri
    FRONT_RIGHT = 1U,   // Motor depan kanan
    REAR_LEFT = 2U,     // Motor belakang kiri
    REAR_RIGHT = 3U     // Motor belakang kanan
};

/**
 * @brief Kelas untuk mengelola satu motor
 */
class Motor {
public:
    /**
     * @brief Konstruktor
     * @param position Posisi motor pada drone
     */
    explicit Motor(MotorPosition position);
    
    /**
     * @brief Inisialisasi motor
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Mengatur kecepatan motor
     * @param speed Kecepatan motor (0-100%)
     * @return true jika berhasil, false jika gagal
     */
    bool setSpeed(Float32 speed);
    
    /**
     * @brief Mendapatkan kecepatan motor
     * @return Kecepatan motor (0-100%)
     */
    Float32 getSpeed() const;
    
    /**
     * @brief Menghentikan motor
     * @return true jika berhasil, false jika gagal
     */
    bool stop();
    
    /**
     * @brief Mendapatkan status motor
     * @return Status motor
     */
    MotorStatus getStatus() const;
    
private:
    MotorPosition position_;    // Posisi motor
    Float32 speed_;             // Kecepatan saat ini (0-100%)
    MotorStatus status_;        // Status motor
    bool initialized_;          // Status inisialisasi
    
    // Hardware control
    Core::GpioPort gpioPort_;   // Port GPIO untuk kontrol motor
    Uint8 gpioPin_;             // Pin GPIO untuk kontrol motor
    
    // Mencegah default constructor, copy constructor dan assignment operator
    Motor();
    Motor(const Motor&);
    Motor& operator=(const Motor&);
};

/**
 * @brief Kelas untuk mengelola semua motor pada drone
 */
class MotorController {
public:
    /**
     * @brief Mendapatkan instance singleton MotorController
     * @return Referensi ke instance MotorController
     */
    static MotorController& getInstance();
    
    /**
     * @brief Inisialisasi semua motor
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Mengatur kecepatan semua motor
     * @param frontLeft Kecepatan motor depan kiri (0-100%)
     * @param frontRight Kecepatan motor depan kanan (0-100%)
     * @param rearLeft Kecepatan motor belakang kiri (0-100%)
     * @param rearRight Kecepatan motor belakang kanan (0-100%)
     * @return true jika berhasil, false jika gagal
     */
    bool setSpeeds(Float32 frontLeft, Float32 frontRight, 
                   Float32 rearLeft, Float32 rearRight);
    
    /**
     * @brief Mengatur semua motor ke kecepatan yang sama
     * @param speed Kecepatan untuk semua motor (0-100%)
     * @return true jika berhasil, false jika gagal
     */
    bool setAllSpeeds(Float32 speed);
    
    /**
     * @brief Menghentikan semua motor
     * @return true jika berhasil, false jika gagal
     */
    bool stopAll();
    
    /**
     * @brief Mendapatkan referensi ke motor tertentu
     * @param position Posisi motor
     * @return Referensi ke motor
     */
    Motor& getMotor(MotorPosition position);
    
private:
    // Private constructor untuk singleton
    MotorController();
    
    // Mencegah copy constructor dan assignment operator
    MotorController(const MotorController&);
    MotorController& operator=(const MotorController&);
    
    // Array motor
    Motor motors_[static_cast<Uint8>(MAX_MOTOR_COUNT)];
    
    // Status inisialisasi
    bool initialized_;
};

/**
 * @brief Fungsi inisialisasi modul control
 * @return true jika berhasil, false jika gagal
 */
bool initializeControl();

} // namespace Control
} // namespace Kamikaze

#endif // KAMIKAZE_CONTROL_MOTOR_CONTROLLER_HPP_ 