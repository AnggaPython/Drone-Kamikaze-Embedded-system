#ifndef KAMIKAZE_SAFETY_SAFETY_MANAGER_HPP_
#define KAMIKAZE_SAFETY_SAFETY_MANAGER_HPP_

#include "../../include/common.hpp"
#include "../targeting/target_detection.hpp"

namespace Kamikaze {
namespace Safety {

/**
 * @brief Mode keamanan
 */
enum class SafetyMode : Uint8 {
    NORMAL = 0U,          // Mode normal
    WARNING = 1U,         // Mode peringatan
    CRITICAL = 2U,        // Mode kritis
    EMERGENCY = 3U        // Mode darurat
};

/**
 * @brief Tindakan keamanan
 */
enum class SafetyAction : Uint8 {
    NONE = 0U,            // Tidak ada tindakan
    ALERT = 1U,           // Hanya memberikan peringatan
    RTL = 2U,             // Return-to-launch
    LAND = 3U,            // Mendarat di tempat
    DISARM = 4U,          // Mematikan motor
    ABORT = 5U            // Membatalkan misi
};

/**
 * @brief Status baterai
 */
struct BatteryStatus {
    Float32 voltage;       // Tegangan (volt)
    Float32 current;       // Arus (ampere)
    Float32 remaining;     // Kapasitas tersisa (persen)
    Float32 temperature;   // Temperatur (celsius)
    
    // Konstruktor dengan inisialisasi sesuai MISRA
    BatteryStatus() :
        voltage(0.0F),
        current(0.0F),
        remaining(100.0F),
        temperature(25.0F) {
    }
};

/**
 * @brief Data keamanan
 */
struct SafetyData {
    SafetyMode mode;              // Mode keamanan saat ini
    ErrorCode error;              // Kode error saat ini
    BatteryStatus battery;        // Status baterai
    bool gpsSignalLost;           // Status sinyal GPS
    bool radioLinkLost;           // Status koneksi radio
    bool imuError;                // Status IMU
    bool motorError;              // Status motor
    bool targetLost;              // Status target hilang
    bool inAttackRange;           // Status dalam jarak serangan
    Uint32 lastErrorTime;         // Waktu error terakhir
    
    // Konstruktor dengan inisialisasi sesuai MISRA
    SafetyData() :
        mode(SafetyMode::NORMAL),
        error(ErrorCode::NONE),
        battery(),
        gpsSignalLost(false),
        radioLinkLost(false),
        imuError(false),
        motorError(false),
        targetLost(false),
        inAttackRange(false),
        lastErrorTime(0U) {
    }
};

/**
 * @brief Kelas untuk mengelola keamanan drone
 */
class SafetyManager {
public:
    /**
     * @brief Mendapatkan instance singleton SafetyManager
     * @return Referensi ke instance SafetyManager
     */
    static SafetyManager& getInstance();
    
    /**
     * @brief Inisialisasi safety manager
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Update status keamanan
     * @return true jika berhasil, false jika gagal
     */
    bool update();
    
    /**
     * @brief Mendapatkan data keamanan
     * @return Data keamanan saat ini
     */
    const SafetyData& getSafetyData() const;
    
    /**
     * @brief Mendapatkan mode keamanan
     * @return Mode keamanan saat ini
     */
    SafetyMode getMode() const;
    
    /**
     * @brief Mendapatkan error saat ini
     * @return Kode error saat ini
     */
    ErrorCode getError() const;
    
    /**
     * @brief Mendapatkan tindakan keamanan berdasarkan mode
     * @param mode Mode keamanan
     * @return Tindakan keamanan yang direkomendasikan
     */
    SafetyAction getRecommendedAction(SafetyMode mode) const;
    
    /**
     * @brief Eksekusi tindakan keamanan
     * @param action Tindakan yang akan dieksekusi
     * @return true jika berhasil, false jika gagal
     */
    bool executeAction(SafetyAction action);
    
    /**
     * @brief Menetapkan error
     * @param error Kode error
     */
    void setError(ErrorCode error);
    
    /**
     * @brief Clear error
     */
    void clearError();
    
    /**
     * @brief Cek apakah drone dalam kondisi aman
     * @return true jika aman, false jika tidak
     */
    bool isSafe() const;
    
    /**
     * @brief Cek apakah aman untuk menyerang target
     * @return true jika aman untuk menyerang, false jika tidak
     */
    bool isSafeToAttack() const;
    
    /**
     * @brief Cek apakah drone memiliki target yang valid
     * @return true jika target valid, false jika tidak
     */
    bool hasValidTarget() const;
    
    /**
     * @brief Cek apakah drone berada dalam jarak serangan
     * @param attackRange Jarak serangan dalam meter
     * @return true jika dalam jarak serangan, false jika tidak
     */
    bool isInAttackRange(Float32 attackRange = DEFAULT_ATTACK_RANGE_M) const;
    
    /**
     * @brief Cek kondisi target dan update status keamanan terkait
     * @return true jika target valid dan aman, false jika tidak
     */
    bool checkTargetConditions();
    
private:
    // Private constructor untuk singleton
    SafetyManager();
    
    // Mencegah copy constructor dan assignment operator
    SafetyManager(const SafetyManager&);
    SafetyManager& operator=(const SafetyManager&);
    
    // Update berbagai status
    void updateBatteryStatus();
    void updateGpsStatus();
    void updateRadioStatus();
    void updateSensorStatus();
    void updateMotorStatus();
    void updateTargetStatus();
    
    // Update mode berdasarkan status
    void updateSafetyMode();
    
    // Data keamanan
    SafetyData safetyData_;
    
    // Status
    bool initialized_;
    
    // Konstanta
    static const Float32 LOW_BATTERY_THRESHOLD;
    static const Float32 CRITICAL_BATTERY_THRESHOLD;
    static const Uint32 LINK_LOSS_TIMEOUT_MS;
    static const Float32 MIN_TARGET_CONFIDENCE;
};

/**
 * @brief Fungsi inisialisasi modul safety
 * @return true jika berhasil, false jika gagal
 */
bool initializeSafety();

} // namespace Safety
} // namespace Kamikaze

#endif // KAMIKAZE_SAFETY_SAFETY_MANAGER_HPP_