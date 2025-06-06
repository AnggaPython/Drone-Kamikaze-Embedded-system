#ifndef KAMIKAZE_NAVIGATION_IMU_HPP_
#define KAMIKAZE_NAVIGATION_IMU_HPP_

#include "../../include/common.hpp"

namespace Kamikaze {
namespace Navigation {

/**
 * @brief Status IMU
 */
enum class ImuStatus : Uint8 {
    CALIBRATING = 0U,   // Kalibrasi
    READY = 1U,         // Siap digunakan
    ERROR = 2U          // Error pada IMU
};

/**
 * @brief Data IMU
 */
struct ImuData {
    Vector3D acceleration;    // Percepatan dalam m/s^2
    Vector3D gyroscope;       // Kecepatan sudut dalam rad/s
    Vector3D magnetometer;    // Data magnetometer
    Vector3D orientation;     // Orientasi (roll, pitch, yaw) dalam radian
    Float32 temperature;      // Temperatur sensor dalam Celsius
    ImuStatus status;         // Status IMU
    
    // Konstruktor dengan inisialisasi sesuai MISRA
    ImuData() :
        acceleration(),
        gyroscope(),
        magnetometer(),
        orientation(),
        temperature(0.0F),
        status(ImuStatus::CALIBRATING) {
    }
};

/**
 * @brief Kelas untuk mengelola IMU
 */
class Imu {
public:
    /**
     * @brief Mendapatkan instance singleton IMU
     * @return Referensi ke instance IMU
     */
    static Imu& getInstance();
    
    /**
     * @brief Inisialisasi modul IMU
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Kalibrasi IMU
     * @return true jika berhasil, false jika gagal
     */
    bool calibrate();
    
    /**
     * @brief Update data IMU
     * @return true jika berhasil mendapatkan data baru, false jika gagal
     */
    bool update();
    
    /**
     * @brief Mendapatkan data IMU
     * @return Data IMU terbaru
     */
    const ImuData& getData() const;
    
    /**
     * @brief Mendapatkan orientasi
     * @return Vector3D berisi roll, pitch, yaw dalam radian
     */
    Vector3D getOrientation() const;
    
    /**
     * @brief Mendapatkan roll
     * @return Nilai roll dalam radian
     */
    Float32 getRoll() const;
    
    /**
     * @brief Mendapatkan pitch
     * @return Nilai pitch dalam radian
     */
    Float32 getPitch() const;
    
    /**
     * @brief Mendapatkan yaw
     * @return Nilai yaw dalam radian
     */
    Float32 getYaw() const;
    
private:
    // Private constructor untuk singleton
    Imu();
    
    // Mencegah copy constructor dan assignment operator
    Imu(const Imu&);
    Imu& operator=(const Imu&);
    
    // Update orientasi dari data sensor
    void updateOrientation();
    
    // Data
    ImuData data_;
    bool initialized_;
    
    // Konstanta kalibrasi
    static const Float32 CALIBRATION_SAMPLES;
};

} // namespace Navigation
} // namespace Kamikaze

#endif // KAMIKAZE_NAVIGATION_IMU_HPP_
