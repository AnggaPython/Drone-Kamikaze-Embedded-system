#ifndef KAMIKAZE_NAVIGATION_GPS_HPP_
#define KAMIKAZE_NAVIGATION_GPS_HPP_

#include "../../include/common.hpp"
#include "../core/hal_uart.hpp"

namespace Kamikaze {
namespace Navigation {

/**
 * @brief Status GPS
 */
enum class GpsStatus : Uint8 {
    NO_FIX = 0U,       // Tidak mendapatkan sinyal
    FIX_2D = 1U,       // Mendapatkan lokasi 2D
    FIX_3D = 2U,       // Mendapatkan lokasi 3D
    ERROR = 3U         // Error pada GPS
};

/**
 * @brief Data GPS
 */
struct GpsData {
    GpsCoordinates position;        // Posisi saat ini
    GpsCoordinates targetPosition;  // Posisi target
    Float32 speed;                  // Kecepatan dalam km/h
    Float32 altitude;               // Ketinggian dalam meter
    Float32 course;                 // Arah dalam derajat (0-359)
    Uint8 satellites;               // Jumlah satelit yang terdeteksi
    GpsStatus status;               // Status GPS
    Float32 distanceToTarget;       // Jarak ke target dalam meter
    Float32 bearingToTarget;        // Arah menuju target dalam derajat
    
    // Konstruktor dengan inisialisasi sesuai MISRA
    GpsData() :
        position(),
        targetPosition(),
        speed(0.0F),
        altitude(0.0F),
        course(0.0F),
        satellites(0U),
        status(GpsStatus::NO_FIX),
        distanceToTarget(0.0F),
        bearingToTarget(0.0F) {
    }
};

/**
 * @brief Kelas untuk mengelola GPS
 */
class Gps {
public:
    /**
     * @brief Mendapatkan instance singleton GPS
     * @return Referensi ke instance GPS
     */
    static Gps& getInstance();
    
    /**
     * @brief Inisialisasi modul GPS
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Update data GPS
     * @return true jika berhasil mendapatkan data baru, false jika gagal
     */
    bool update();
    
    /**
     * @brief Mendapatkan data GPS
     * @return Data GPS terbaru
     */
    const GpsData& getData() const;
    
    /**
     * @brief Mengatur koordinat target
     * @param target Koordinat target
     * @return true jika berhasil, false jika gagal
     */
    bool setTarget(const GpsCoordinates& target);
    
    /**
     * @brief Mendapatkan jarak ke target
     * @return Jarak dalam meter
     */
    Float32 getDistanceToTarget() const;
    
    /**
     * @brief Mendapatkan bearing ke target
     * @return Bearing dalam derajat (0-359)
     */
    Float32 getBearingToTarget() const;
    
private:
    // Private constructor untuk singleton
    Gps();
    
    // Mencegah copy constructor dan assignment operator
    Gps(const Gps&);
    Gps& operator=(const Gps&);
    
    // Parsing NMEA sentences
    bool parseNMEA(const char* sentence);
    bool parseGGA(const char* sentence);
    bool parseRMC(const char* sentence);
    
    // Kalkulasi jarak dan bearing
    void calculateDistanceAndBearing();
    
    // Data
    GpsData data_;
    bool initialized_;
    
    // UART yang digunakan untuk GPS
    Core::UartPort gpsUartPort_;
};

/**
 * @brief Fungsi inisialisasi modul navigasi
 * @return true jika berhasil, false jika gagal
 */
bool initializeNavigation();

} // namespace Navigation
} // namespace Kamikaze

#endif // KAMIKAZE_NAVIGATION_GPS_HPP_
