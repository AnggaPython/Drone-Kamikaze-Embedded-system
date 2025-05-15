#ifndef KAMIKAZE_COMMON_HPP_
#define KAMIKAZE_COMMON_HPP_

// MISRA C++ compliant common header

#include <cstdint>
#include <limits>
#include <cmath>

namespace Kamikaze {

// Gunakan tipe data standar dengan ukuran eksplisit untuk MISRA compliance
typedef std::int8_t  Int8;
typedef std::int16_t Int16;
typedef std::int32_t Int32;
typedef std::int64_t Int64;

typedef std::uint8_t  Uint8;
typedef std::uint16_t Uint16;
typedef std::uint32_t Uint32;
typedef std::uint64_t Uint64;

typedef float  Float32;
typedef double Float64;

// Konstanta sistem
static const Uint16 MAX_ALTITUDE_METERS = 500U;
static const Uint16 MAX_SPEED_KMH = 180U;
static const Uint8 MAX_MOTOR_COUNT = 4U;
static const Uint16 SYSTEM_TICK_MS = 10U;
static const Uint16 TELEMETRY_UPDATE_MS = 100U;
static const Uint16 GPS_UPDATE_MS = 1000U;
static const Float32 DEFAULT_PID_P = 1.0F;
static const Float32 DEFAULT_PID_I = 0.0F;
static const Float32 DEFAULT_PID_D = 0.0F;

// Konstanta navigasi
static const Float32 PI = 3.14159265358979323846F;
static const Float32 DEG_TO_RAD = PI / 180.0F;
static const Float32 RAD_TO_DEG = 180.0F / PI;
static const Float32 EARTH_RADIUS_M = 6371000.0F;  // Radius bumi dalam meter
static const Float32 METERS_PER_DEGREE_LAT = 111111.0F; // Approx meter per derajat lintang
static const Float32 DEFAULT_ATTACK_RANGE_M = 50.0F; // Jarak serang default dalam meter
static const Float32 DEFAULT_DRONE_SPEED_MS = 10.0F; // Kecepatan default drone dalam m/s
static const Float32 SAFETY_ALTITUDE_M = 20.0F; // Ketinggian minimal untuk keamanan

// Target-related constants
static const Float32 TARGET_ACQUIRE_CONFIDENCE = 50.0F; // Confidence untuk mulai akuisisi target
static const Float32 TARGET_LOCK_CONFIDENCE = 70.0F;    // Confidence untuk mengunci target
static const Uint32 TARGET_LOST_TIMEOUT_MS = 5000U;     // Timeout untuk target hilang

// Status dan error codes
enum class SystemStatus : Uint8 {
    INITIALIZING = 0U,
    READY = 1U,
    ARMED = 2U,
    FLYING = 3U,
    TARGETING = 4U,
    ATTACKING = 5U,
    EMERGENCY = 6U,
    ERROR = 7U
};

enum class ErrorCode : Uint8 {
    NONE = 0U,
    INIT_FAILED = 1U,
    SENSOR_ERROR = 2U,
    MOTOR_ERROR = 3U,
    COMMUNICATION_ERROR = 4U,
    LOW_BATTERY = 5U,
    GPS_LOST = 6U,
    IMU_ERROR = 7U,
    TARGET_LOST = 8U,
    GEOFENCE_BREACH = 9U,
    ALTITUDE_BREACH = 10U
};

struct Vector3D {
    Float32 x;
    Float32 y;
    Float32 z;

    Vector3D() : x(0.0F), y(0.0F), z(0.0F) {}
    Vector3D(Float32 x_, Float32 y_, Float32 z_) : x(x_), y(y_), z(z_) {}
};

struct GpsCoordinates {
    Float64 latitude;
    Float64 longitude;
    Float32 altitude;
    
    GpsCoordinates() : latitude(0.0), longitude(0.0), altitude(0.0F) {}
    GpsCoordinates(Float64 lat, Float64 lon, Float32 alt) : 
        latitude(lat), longitude(lon), altitude(alt) {}
};

// Utility templates compliant with MISRA rules
template<typename T>
static inline T constrain(T value, T min, T max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

// Fungsi utilitas navigasi dan GPS

/**
 * @brief Menghitung jarak antara dua koordinat GPS
 * @param coord1 Koordinat pertama
 * @param coord2 Koordinat kedua
 * @return Jarak dalam meter
 */
static Float32 calculateDistanceGps(const GpsCoordinates& coord1, const GpsCoordinates& coord2) {
    // Haversine formula untuk menghitung jarak
    Float32 lat1 = static_cast<Float32>(coord1.latitude) * DEG_TO_RAD;
    Float32 lon1 = static_cast<Float32>(coord1.longitude) * DEG_TO_RAD;
    Float32 lat2 = static_cast<Float32>(coord2.latitude) * DEG_TO_RAD;
    Float32 lon2 = static_cast<Float32>(coord2.longitude) * DEG_TO_RAD;
    
    Float32 dLat = lat2 - lat1;
    Float32 dLon = lon2 - lon1;
    
    Float32 a = sinf(dLat/2.0F) * sinf(dLat/2.0F) +
                cosf(lat1) * cosf(lat2) *
                sinf(dLon/2.0F) * sinf(dLon/2.0F);
    Float32 c = 2.0F * atan2f(sqrtf(a), sqrtf(1.0F-a));
    
    return EARTH_RADIUS_M * c;
}

/**
 * @brief Menghitung bearing (arah) dari posisi 1 ke posisi 2
 * @param coord1 Koordinat asal
 * @param coord2 Koordinat tujuan
 * @return Bearing dalam derajat (0-360)
 */
static Float32 calculateBearing(const GpsCoordinates& coord1, const GpsCoordinates& coord2) {
    Float32 lat1 = static_cast<Float32>(coord1.latitude) * DEG_TO_RAD;
    Float32 lon1 = static_cast<Float32>(coord1.longitude) * DEG_TO_RAD;
    Float32 lat2 = static_cast<Float32>(coord2.latitude) * DEG_TO_RAD;
    Float32 lon2 = static_cast<Float32>(coord2.longitude) * DEG_TO_RAD;
    
    Float32 dLon = lon2 - lon1;
    
    Float32 y = sinf(dLon) * cosf(lat2);
    Float32 x = cosf(lat1) * sinf(lat2) -
               sinf(lat1) * cosf(lat2) * cosf(dLon);
    
    Float32 bearing = atan2f(y, x) * RAD_TO_DEG;
    
    // Normalisasi ke 0-360
    if (bearing < 0.0F) {
        bearing += 360.0F;
    }
    
    return bearing;
}

/**
 * @brief Menghitung koordinat GPS baru berdasarkan koordinat awal, bearing, dan jarak
 * @param start Koordinat awal
 * @param bearing Arah dalam derajat
 * @param distance Jarak dalam meter
 * @return Koordinat GPS baru
 */
static GpsCoordinates calculateDestinationPoint(const GpsCoordinates& start, Float32 bearing, Float32 distance) {
    Float32 lat1 = static_cast<Float32>(start.latitude) * DEG_TO_RAD;
    Float32 lon1 = static_cast<Float32>(start.longitude) * DEG_TO_RAD;
    Float32 bearingRad = bearing * DEG_TO_RAD;
    
    Float32 angDist = distance / EARTH_RADIUS_M;
    
    Float32 lat2 = asinf(sinf(lat1) * cosf(angDist) + 
                        cosf(lat1) * sinf(angDist) * cosf(bearingRad));
    Float32 lon2 = lon1 + atan2f(sinf(bearingRad) * sinf(angDist) * cosf(lat1),
                               cosf(angDist) - sinf(lat1) * sinf(lat2));
    
    // Normalisasi longitude ke -180 sampai +180
    lon2 = fmodf(lon2 + 3.0F * PI, 2.0F * PI) - PI;
    
    GpsCoordinates result;
    result.latitude = static_cast<Float64>(lat2 * RAD_TO_DEG);
    result.longitude = static_cast<Float64>(lon2 * RAD_TO_DEG);
    result.altitude = start.altitude;
    
    return result;
}

/**
 * @brief Mengkonversi meter ke derajat latitude
 * @param meters Jarak dalam meter
 * @return Derajat latitude
 */
static Float64 metersToLatDegrees(Float32 meters) {
    return static_cast<Float64>(meters / METERS_PER_DEGREE_LAT);
}

/**
 * @brief Mengkonversi meter ke derajat longitude pada latitude tertentu
 * @param meters Jarak dalam meter
 * @param latitude Latitude dalam derajat
 * @return Derajat longitude
 */
static Float64 metersToLonDegrees(Float32 meters, Float64 latitude) {
    Float32 latRad = static_cast<Float32>(latitude) * DEG_TO_RAD;
    Float32 metersPerDegree = METERS_PER_DEGREE_LAT * cosf(latRad);
    return static_cast<Float64>(meters / metersPerDegree);
}

} // namespace Kamikaze

#endif // KAMIKAZE_COMMON_HPP_ 