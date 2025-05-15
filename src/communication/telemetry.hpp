#ifndef KAMIKAZE_COMMUNICATION_TELEMETRY_HPP_
#define KAMIKAZE_COMMUNICATION_TELEMETRY_HPP_

#include "../../include/common.hpp"
#include "radio.hpp"
#include "../navigation/gps.hpp"
#include "../navigation/imu.hpp"
#include "../targeting/target_detection.hpp"

namespace Kamikaze {
namespace Communication {

enum class TelemetryType : Uint8 {
    ATTITUDE = 0U,
    GPS = 1U,
    BATTERY = 2U,
    SYSTEM = 3U,
    MISSION = 4U,
    TARGET = 5U
};

struct TelemetryData {
    TelemetryType type;
    Uint32 timestamp;
    
    union {
        struct {
            Float32 roll;
            Float32 pitch;
            Float32 yaw;
        } attitude;
        
        struct {
            Float64 latitude;
            Float64 longitude;
            Float32 altitude;
            Float32 speed;
            Uint8 satellites;
        } gps;
        
        struct {
            Float32 voltage;
            Float32 current;
            Float32 remaining;
        } battery;
        
        struct {
            SystemStatus status;
            ErrorCode error;
            Uint32 uptime;
        } system;
        
        struct {
            Uint8 missionId;
            Uint8 waypointIndex;
            Float32 progress;
            Float32 timeToTarget;
        } mission;
        
        struct {
            Float64 latitude;
            Float64 longitude;
            Float32 distance;
            Float32 bearing;
            Float32 confidence;
            Uint8 status;
            Uint8 type;
            Float32 speed;
            Float32 timeToIntercept;
        } target;
    } data;
    
    TelemetryData() : 
        type(TelemetryType::SYSTEM),
        timestamp(0U) {
        data.system.status = SystemStatus::INITIALIZING;
        data.system.error = ErrorCode::NONE;
        data.system.uptime = 0U;
    }
};

class Telemetry {
public:
    static Telemetry& getInstance();
    
    bool initialize();
    
    bool update();
    
    bool send();
    
    bool sendData(TelemetryType type);
    
    void setUpdateRate(Uint32 rateMs);
    
    const TelemetryData& getData(TelemetryType type) const;
    
private:
    Telemetry();
    
    Telemetry(const Telemetry&);
    Telemetry& operator=(const Telemetry&);
    
    bool updateAttitude();
    bool updateGps();
    bool updateBattery();
    bool updateSystem();
    bool updateMission();
    bool updateTarget();
    
    bool encodeData(const TelemetryData& data, RadioMessage& message);
    
    Float32 calculateTimeToIntercept();
    
    TelemetryData attitudeData_;
    TelemetryData gpsData_;
    TelemetryData batteryData_;
    TelemetryData systemData_;
    TelemetryData missionData_;
    TelemetryData targetData_;
    
    Uint32 updateRateMs_;
    Uint32 lastUpdateMs_;
    
    bool initialized_;
};

} // namespace Communication
} // namespace Kamikaze

#endif // KAMIKAZE_COMMUNICATION_TELEMETRY_HPP_