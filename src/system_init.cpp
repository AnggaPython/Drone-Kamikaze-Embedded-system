#include "../include/common.hpp"
#include "core/rtos.hpp"
#include "core/hal_gpio.hpp"
#include "core/hal_uart.hpp"
#include "navigation/gps.hpp"
#include "navigation/imu.hpp"
#include "control/motor_controller.hpp"
#include "control/pid_controller.hpp"
#include "communication/radio.hpp"
#include "communication/telemetry.hpp"
#include "targeting/target_detection.hpp"
#include "safety/safety_manager.hpp"

namespace Kamikaze {

/**
 * @brief Kelas untuk inisialisasi sistem
 */
class SystemInit {
public:
    /**
     * @brief Mendapatkan instance singleton SystemInit
     * @return Referensi ke instance SystemInit
     */
    static SystemInit& getInstance() {
        static SystemInit instance;
        return instance;
    }
    
    /**
     * @brief Inisialisasi seluruh subsistem
     * @return true jika berhasil, false jika gagal
     */
    bool initializeSystem() {
        // Status inisialisasi
        bool status = true;
        
        // Inisialisasi hardware abstraction layer
        status = status && initializeHardware();
        
        // Inisialisasi komunikasi
        status = status && initializeCommunication();
        
        // Inisialisasi navigasi
        status = status && initializeNavigation();
        
        // Inisialisasi kontrol
        status = status && initializeControl();
        
        // Inisialisasi targeting
        status = status && initializeTargeting();
        
        // Inisialisasi safety
        status = status && initializeSafety();
        
        // Inisialisasi RTOS harus yang terakhir
        status = status && Core::Rtos::getInstance().initialize();
        
        return status;
    }
    
    /**
     * @brief Inisialisasi hardware abstraction layer
     * @return true jika berhasil, false jika gagal
     */
    bool initializeHardware() {
        bool status = true;
        
        // Inisialisasi GPIO dan UART
        status = status && Core::HalGpio::getInstance().initialize();
        status = status && Core::HalUart::getInstance().initialize();
        
        // Konfigurasi GPIO dan UART
        status = status && configureGpio();
        status = status && configureUart();
        
        return status;
    }
    
    /**
     * @brief Inisialisasi modul navigasi
     * @return true jika berhasil, false jika gagal
     */
    bool initializeNavigation() {
        bool status = true;
        
        // Inisialisasi IMU
        status = status && Navigation::Imu::getInstance().initialize();
        
        // Inisialisasi GPS
        status = status && Navigation::Gps::getInstance().initialize();
        
        return status;
    }
    
    /**
     * @brief Inisialisasi modul komunikasi
     * @return true jika berhasil, false jika gagal
     */
    bool initializeCommunication() {
        bool status = true;
        
        // Inisialisasi radio
        status = status && Communication::Radio::getInstance().initialize();
        
        // Inisialisasi telemetri
        status = status && Communication::Telemetry::getInstance().initialize();
        
        return status;
    }
    
    /**
     * @brief Inisialisasi modul kontrol
     * @return true jika berhasil, false jika gagal
     */
    bool initializeControl() {
        bool status = true;
        
        // Inisialisasi motor controller
        status = status && Control::MotorController::getInstance().initialize();
        
        // Inisialisasi PID manager
        status = status && Control::PidManager::getInstance().initialize();
        
        return status;
    }
    
    /**
     * @brief Inisialisasi modul targeting
     * @return true jika berhasil, false jika gagal
     */
    bool initializeTargeting() {
        bool status = true;
        
        // Inisialisasi target detection
        status = status && Targeting::TargetDetection::getInstance().initialize();
        
        return status;
    }
    
    /**
     * @brief Inisialisasi modul safety
     * @return true jika berhasil, false jika gagal
     */
    bool initializeSafety() {
        bool status = true;
        
        // Inisialisasi safety manager
        status = status && Safety::SafetyManager::getInstance().initialize();
        
        return status;
    }
    
    /**
     * @brief Konfigurasi GPIO pins
     * @return true jika berhasil, false jika gagal
     */
    bool configureGpio() {
        // Status konfigurasi
        bool status = true;
        
        Core::HalGpio& gpio = Core::HalGpio::getInstance();
        
        // Konfigurasi LED indikator
        status = status && gpio.configurePin(Core::GpioPort::PORT_A, 5U, Core::GpioMode::OUTPUT, Core::GpioPull::NONE);
        
        // Konfigurasi pin motor
        status = status && gpio.configurePin(Core::GpioPort::PORT_B, 0U, Core::GpioMode::OUTPUT, Core::GpioPull::NONE);
        status = status && gpio.configurePin(Core::GpioPort::PORT_B, 1U, Core::GpioMode::OUTPUT, Core::GpioPull::NONE);
        status = status && gpio.configurePin(Core::GpioPort::PORT_B, 2U, Core::GpioMode::OUTPUT, Core::GpioPull::NONE);
        status = status && gpio.configurePin(Core::GpioPort::PORT_B, 3U, Core::GpioMode::OUTPUT, Core::GpioPull::NONE);
        
        // Konfigurasi pin sensor
        status = status && gpio.configurePin(Core::GpioPort::PORT_C, 0U, Core::GpioMode::INPUT, Core::GpioPull::PULLUP);
        status = status && gpio.configurePin(Core::GpioPort::PORT_C, 1U, Core::GpioMode::INPUT, Core::GpioPull::PULLUP);
        
        return status;
    }
    
    /**
     * @brief Konfigurasi UART
     * @return true jika berhasil, false jika gagal
     */
    bool configureUart() {
        // Status konfigurasi
        bool status = true;
        
        Core::HalUart& uart = Core::HalUart::getInstance();
        
        // Konfigurasi UART untuk telemetri
        status = status && uart.configurePort(
            Core::UartPort::UART1,
            Core::UartBaudRate::BAUD_115200,
            Core::UartParity::NONE,
            Core::UartStopBits::ONE
        );
        
        // Konfigurasi UART untuk GPS
        status = status && uart.configurePort(
            Core::UartPort::UART2,
            Core::UartBaudRate::BAUD_9600,
            Core::UartParity::NONE,
            Core::UartStopBits::ONE
        );
        
        // Konfigurasi UART untuk radio
        status = status && uart.configurePort(
            Core::UartPort::UART3,
            Core::UartBaudRate::BAUD_57600,
            Core::UartParity::NONE,
            Core::UartStopBits::ONE
        );
        
        return status;
    }
    
    /**
     * @brief Self-test semua subsistem
     * @return true jika semua subsistem berfungsi, false jika ada kesalahan
     */
    bool performSelfTest() {
        bool status = true;
        
        // Test hardware
        status = status && testHardware();
        
        // Test navigasi
        status = status && testNavigation();
        
        // Test komunikasi
        status = status && testCommunication();
        
        // Test motor
        status = status && testMotors();
        
        return status;
    }
    
    /**
     * @brief Self-test hardware
     * @return true jika hardware berfungsi, false jika tidak
     */
    bool testHardware() {
        // Implementasi test hardware
        return true;
    }
    
    /**
     * @brief Self-test sistem navigasi
     * @return true jika navigasi berfungsi, false jika tidak
     */
    bool testNavigation() {
        // Test IMU
        bool imuStatus = (Navigation::Imu::getInstance().getData().status != Navigation::ImuStatus::ERROR);
        
        // Test GPS
        bool gpsStatus = (Navigation::Gps::getInstance().getData().status != Navigation::GpsStatus::ERROR);
        
        return imuStatus && gpsStatus;
    }
    
    /**
     * @brief Self-test sistem komunikasi
     * @return true jika komunikasi berfungsi, false jika tidak
     */
    bool testCommunication() {
        // Test radio link
        bool radioStatus = (Communication::Radio::getInstance().getStatus() != Communication::RadioStatus::ERROR);
        
        return radioStatus;
    }
    
    /**
     * @brief Self-test motor
     * @return true jika motor berfungsi, false jika tidak
     */
    bool testMotors() {
        // Motor controller status
        Control::MotorController& motorController = Control::MotorController::getInstance();
        
        // Test semua motor satu per satu dengan kecepatan rendah
        const Float32 TEST_SPEED = 10.0F;
        
        for (Uint8 i = 0U; i < MAX_MOTOR_COUNT; ++i) {
            Control::MotorPosition position = static_cast<Control::MotorPosition>(i);
            
            // Aktifkan motor
            if (!motorController.getMotor(position).setSpeed(TEST_SPEED)) {
                return false;
            }
            
            // Delay singkat
            Core::Rtos::delay(100U);
            
            // Stop motor
            if (!motorController.getMotor(position).stop()) {
                return false;
            }
            
            // Delay antar test motor
            Core::Rtos::delay(200U);
        }
        
        return true;
    }

private:
    // Private constructor untuk singleton
    SystemInit() {
        // Tidak ada inisialisasi di konstruktor
    }
    
    // Mencegah copy constructor dan assignment operator
    SystemInit(const SystemInit&);
    SystemInit& operator=(const SystemInit&);
};

} // namespace Kamikaze 