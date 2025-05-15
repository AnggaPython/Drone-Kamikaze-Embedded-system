#ifndef KAMIKAZE_CORE_HAL_GPIO_HPP_
#define KAMIKAZE_CORE_HAL_GPIO_HPP_

#include "../../include/common.hpp"

namespace Kamikaze {
namespace Core {

/**
 * @brief Enumerasi mode GPIO yang didukung
 */
enum class GpioMode : Uint8 {
    INPUT = 0U,
    OUTPUT = 1U,
    ANALOG = 2U,
    ALTERNATE = 3U
};

/**
 * @brief Enumerasi pull-up/pull-down resistor
 */
enum class GpioPull : Uint8 {
    NONE = 0U,
    PULLUP = 1U,
    PULLDOWN = 2U
};

/**
 * @brief Enumerasi port GPIO
 */
enum class GpioPort : Uint8 {
    PORT_A = 0U,
    PORT_B = 1U,
    PORT_C = 2U,
    PORT_D = 3U,
    PORT_E = 4U,
    PORT_F = 5U
};

/**
 * @brief Kelas untuk mengelola GPIO pada drone
 */
class HalGpio {
public:
    /**
     * @brief Mendapatkan instance singleton dari kelas GPIO
     * @return Referensi ke instance GPIO
     */
    static HalGpio& getInstance();

    /**
     * @brief Inisialisasi subsistem GPIO
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();

    /**
     * @brief Mengkonfigurasi pin GPIO
     * @param port Port GPIO
     * @param pin Nomor pin (0-15)
     * @param mode Mode GPIO (INPUT, OUTPUT, dll)
     * @param pull Pull-up/pull-down resistor
     * @return true jika berhasil, false jika gagal
     */
    bool configurePin(GpioPort port, Uint8 pin, GpioMode mode, GpioPull pull);

    /**
     * @brief Menulis nilai ke pin GPIO
     * @param port Port GPIO
     * @param pin Nomor pin (0-15)
     * @param value Nilai untuk ditulis (true=HIGH, false=LOW)
     * @return true jika berhasil, false jika gagal
     */
    bool writePin(GpioPort port, Uint8 pin, bool value);

    /**
     * @brief Membaca nilai dari pin GPIO
     * @param port Port GPIO
     * @param pin Nomor pin (0-15)
     * @return Nilai pin (true=HIGH, false=LOW)
     */
    bool readPin(GpioPort port, Uint8 pin);

    /**
     * @brief Toggle nilai pin GPIO
     * @param port Port GPIO
     * @param pin Nomor pin (0-15)
     * @return true jika berhasil, false jika gagal
     */
    bool togglePin(GpioPort port, Uint8 pin);

private:
    // Private constructor untuk singleton
    HalGpio();
    
    // Mencegah copy constructor dan assignment operator
    HalGpio(const HalGpio&);
    HalGpio& operator=(const HalGpio&);
    
    // Implementasi platform-specific
    bool initialized_;
    
    // Konstanta pin dan port maksimum
    static const Uint8 MAX_PIN = 15U;
    static const Uint8 MAX_PORT = 5U;
};

} // namespace Core
} // namespace Kamikaze

#endif // KAMIKAZE_CORE_HAL_GPIO_HPP_ 