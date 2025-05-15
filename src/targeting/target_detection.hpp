#ifndef KAMIKAZE_TARGETING_TARGET_DETECTION_HPP_
#define KAMIKAZE_TARGETING_TARGET_DETECTION_HPP_

#include "../../include/common.hpp"
#include "../navigation/gps.hpp"

namespace Kamikaze {
namespace Targeting {

enum class TargetStatus : Uint8 {
    NO_TARGET = 0U,
    TARGET_ACQUIRING = 1U,
    TARGET_LOCKED = 2U,
    TARGET_LOST = 3U
};

enum class TargetType : Uint8 {
    STATIC = 0U,
    MOVING = 1U,
    AREA = 2U
};

struct TargetData {
    GpsCoordinates position;
    TargetStatus status;
    TargetType type;
    Float32 confidence;
    Float32 distance;
    Float32 bearing;
    Float32 speed;
    Float32 heading;
    Uint32 lastUpdateTime;
    
    TargetData() :
        position(),
        status(TargetStatus::NO_TARGET),
        type(TargetType::STATIC),
        confidence(0.0F),
        distance(0.0F),
        bearing(0.0F),
        speed(0.0F),
        heading(0.0F),
        lastUpdateTime(0U) {
    }
};

class TargetDetection {
public:
    static TargetDetection& getInstance();
    
    bool initialize();
    
    bool update();
    
    const TargetData& getTargetData() const;
    
    bool setTarget(const GpsCoordinates& targetPosition, TargetType targetType);
    
    bool clearTarget();
    
    bool isTargetLocked() const;
    
    bool isInAttackRange(Float32 attackRange) const;
    
private:
    TargetDetection();
    
    TargetDetection(const TargetDetection&);
    TargetDetection& operator=(const TargetDetection&);
    
    void updateTargetVectors();
    
    bool detectTargetWithSensors();
    
    TargetData targetData_;
    
    bool initialized_;
    
    static const Float32 MIN_CONFIDENCE_THRESHOLD;
    static const Float32 MAX_TARGET_LOST_TIME_MS;
};

bool initializeTargeting();

} // namespace Targeting
} // namespace Kamikaze

#endif // KAMIKAZE_TARGETING_TARGET_DETECTION_HPP_