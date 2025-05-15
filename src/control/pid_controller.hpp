#ifndef KAMIKAZE_CONTROL_PID_CONTROLLER_HPP_
#define KAMIKAZE_CONTROL_PID_CONTROLLER_HPP_

#include "../../include/common.hpp"

namespace Kamikaze {
namespace Control {

/**
 * @brief Enum untuk mode PID
 */
enum class PidMode : Uint8 {
    AUTOMATIC = 0U,     // PID berjalan otomatis
    MANUAL = 1U         // PID dikendalikan manual
};

/**
 * @brief Enum untuk kontrol yang dikendalikan PID
 */
enum class PidControlType : Uint8 {
    ROLL = 0U,          // Kontrol roll (rotasi pada sumbu X)
    PITCH = 1U,         // Kontrol pitch (rotasi pada sumbu Y)
    YAW = 2U,           // Kontrol yaw (rotasi pada sumbu Z)
    THROTTLE = 3U,      // Kontrol throttle (kecepatan motor)
    ALTITUDE = 4U       // Kontrol ketinggian
};

/**
 * @brief Kelas untuk PID controller
 */
class PidController {
public:
    /**
     * @brief Konstruktor
     * @param type Tipe kontrol PID
     * @param kp Konstanta proporsional
     * @param ki Konstanta integral
     * @param kd Konstanta derivatif
     */
    PidController(PidControlType type, Float32 kp, Float32 ki, Float32 kd);
    
    /**
     * @brief Initialize PID
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Compute output PID
     * @param input Nilai input saat ini
     * @param setpoint Nilai yang diinginkan
     * @return Nilai output PID
     */
    Float32 compute(Float32 input, Float32 setpoint);
    
    /**
     * @brief Set konstanta PID
     * @param kp Konstanta proporsional
     * @param ki Konstanta integral
     * @param kd Konstanta derivatif
     */
    void setTunings(Float32 kp, Float32 ki, Float32 kd);
    
    /**
     * @brief Set output limits
     * @param min Nilai minimum output
     * @param max Nilai maksimum output
     */
    void setOutputLimits(Float32 min, Float32 max);
    
    /**
     * @brief Set mode PID
     * @param mode Mode PID
     */
    void setMode(PidMode mode);
    
    /**
     * @brief Mendapatkan mode PID
     * @return Mode PID
     */
    PidMode getMode() const;
    
    /**
     * @brief Mendapatkan tipe kontrol PID
     * @return Tipe kontrol PID
     */
    PidControlType getType() const;
    
    /**
     * @brief Reset PID ke kondisi awal
     */
    void reset();
    
private:
    // Konstanta PID
    Float32 kp_;            // Konstanta proporsional
    Float32 ki_;            // Konstanta integral
    Float32 kd_;            // Konstanta derivatif
    
    // Variabel internal
    Float32 lastInput_;     // Input terakhir
    Float32 outputSum_;     // Sum untuk komponen integral
    Float32 lastError_;     // Error terakhir
    Float32 output_;        // Output terakhir
    
    // Batas output
    Float32 outputMin_;     // Batas minimum output
    Float32 outputMax_;     // Batas maksimum output
    
    // Tipe kontrol
    PidControlType type_;   // Tipe kontrol PID
    
    // Mode PID
    PidMode mode_;          // Mode PID
    
    // Status
    bool initialized_;      // Status inisialisasi
    
    // Mencegah default constructor, copy constructor dan assignment operator
    PidController();
    PidController(const PidController&);
    PidController& operator=(const PidController&);
};

/**
 * @brief Kelas untuk mengelola semua PID controller
 */
class PidManager {
public:
    /**
     * @brief Mendapatkan instance singleton PidManager
     * @return Referensi ke instance PidManager
     */
    static PidManager& getInstance();
    
    /**
     * @brief Inisialisasi semua PID controller
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Mendapatkan referensi ke PID controller tertentu
     * @param type Tipe kontrol PID
     * @return Referensi ke PID controller
     */
    PidController& getController(PidControlType type);
    
private:
    // Private constructor untuk singleton
    PidManager();
    
    // Mencegah copy constructor dan assignment operator
    PidManager(const PidManager&);
    PidManager& operator=(const PidManager&);
    
    // Array PID controller
    static const Uint8 NUM_CONTROLLERS = 5U;
    PidController controllers_[NUM_CONTROLLERS];
    
    // Status inisialisasi
    bool initialized_;
};

} // namespace Control
} // namespace Kamikaze

#endif // KAMIKAZE_CONTROL_PID_CONTROLLER_HPP_ 