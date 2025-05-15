#include "pid_controller.hpp"

namespace Kamikaze {
namespace Control {

// Implementasi PidController

PidController::PidController(PidControlType type, Float32 kp, Float32 ki, Float32 kd) :
    kp_(kp),
    ki_(ki),
    kd_(kd),
    lastInput_(0.0F),
    outputSum_(0.0F),
    lastError_(0.0F),
    output_(0.0F),
    outputMin_(0.0F),
    outputMax_(100.0F),
    type_(type),
    mode_(PidMode::AUTOMATIC),
    initialized_(false) {
    // Inisialisasi dengan nilai default
}

bool PidController::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Reset PID
    reset();
    
    initialized_ = true;
    return true;
}

Float32 PidController::compute(Float32 input, Float32 setpoint) {
    if (!initialized_ || mode_ != PidMode::AUTOMATIC) {
        return output_;
    }
    
    // Hitung error saat ini
    Float32 error = setpoint - input;
    
    // Komponen proporsional
    Float32 pOutput = kp_ * error;
    
    // Komponen integral
    outputSum_ += ki_ * error;
    
    // Anti-windup: batasi outputSum dalam batas output
    if (outputSum_ > outputMax_) {
        outputSum_ = outputMax_;
    } else if (outputSum_ < outputMin_) {
        outputSum_ = outputMin_;
    }
    
    // Komponen derivatif (pada perubahan input, bukan perubahan error)
    Float32 dInput = input - lastInput_;
    Float32 dOutput = -kd_ * dInput;  // Negatif karena dInput berlawanan dengan arah error
    
    // Hitung output total
    output_ = pOutput + outputSum_ + dOutput;
    
    // Batasi output dalam rentang yang valid
    if (output_ > outputMax_) {
        output_ = outputMax_;
    } else if (output_ < outputMin_) {
        output_ = outputMin_;
    }
    
    // Simpan nilai untuk iterasi berikutnya
    lastError_ = error;
    lastInput_ = input;
    
    return output_;
}

void PidController::setTunings(Float32 kp, Float32 ki, Float32 kd) {
    if (kp < 0.0F || ki < 0.0F || kd < 0.0F) {
        return;
    }
    
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}

void PidController::setOutputLimits(Float32 min, Float32 max) {
    if (min >= max) {
        return;
    }
    
    outputMin_ = min;
    outputMax_ = max;
    
    // Anti-windup: sesuaikan outputSum jika diluar batas
    if (outputSum_ > outputMax_) {
        outputSum_ = outputMax_;
    } else if (outputSum_ < outputMin_) {
        outputSum_ = outputMin_;
    }
    
    // Sesuaikan output jika diluar batas
    if (output_ > outputMax_) {
        output_ = outputMax_;
    } else if (output_ < outputMin_) {
        output_ = outputMin_;
    }
}

void PidController::setMode(PidMode mode) {
    // Jika beralih dari MANUAL ke AUTOMATIC, inisialisasi ulang
    if (mode_ == PidMode::MANUAL && mode == PidMode::AUTOMATIC) {
        reset();
    }
    
    mode_ = mode;
}

PidMode PidController::getMode() const {
    return mode_;
}

PidControlType PidController::getType() const {
    return type_;
}

void PidController::reset() {
    // Reset status internal PID
    lastInput_ = 0.0F;
    outputSum_ = 0.0F;
    lastError_ = 0.0F;
    output_ = 0.0F;
}

// Implementasi PidManager

PidManager& PidManager::getInstance() {
    static PidManager instance;
    return instance;
}

PidManager::PidManager() :
    controllers_{
        // Inisialisasi array controllers dengan parameter default
        PidController(PidControlType::ROLL, DEFAULT_PID_P, DEFAULT_PID_I, DEFAULT_PID_D),
        PidController(PidControlType::PITCH, DEFAULT_PID_P, DEFAULT_PID_I, DEFAULT_PID_D),
        PidController(PidControlType::YAW, DEFAULT_PID_P, DEFAULT_PID_I, DEFAULT_PID_D),
        PidController(PidControlType::THROTTLE, DEFAULT_PID_P, DEFAULT_PID_I, DEFAULT_PID_D),
        PidController(PidControlType::ALTITUDE, DEFAULT_PID_P, DEFAULT_PID_I, DEFAULT_PID_D)
    },
    initialized_(false) {
    // Inisialisasi array PID controller
}

bool PidManager::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Inisialisasi semua PID controller
    bool result = true;
    
    for (Uint8 i = 0U; i < NUM_CONTROLLERS; ++i) {
        result = result && controllers_[i].initialize();
    }
    
    initialized_ = result;
    return result;
}

PidController& PidManager::getController(PidControlType type) {
    return controllers_[static_cast<Uint8>(type)];
}

} // namespace Control
} // namespace Kamikaze 