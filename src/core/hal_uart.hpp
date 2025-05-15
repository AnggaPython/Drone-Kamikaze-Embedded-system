#ifndef KAMIKAZE_CORE_HAL_UART_HPP_
#define KAMIKAZE_CORE_HAL_UART_HPP_

#include "../../include/common.hpp"

namespace Kamikaze {
namespace Core {

/**
 * @brief Enumerasi baudrate UART yang didukung
 */
enum class UartBaudRate : Uint32 {
    BAUD_9600 = 9600U,
    BAUD_19200 = 19200U,
    BAUD_38400 = 38400U,
    BAUD_57600 = 57600U,
    BAUD_115200 = 115200U
};

/**
 * @brief Enumerasi mode parity UART
 */
enum class UartParity : Uint8 {
    NONE = 0U,
    ODD = 1U,
    EVEN = 2U
};

/**
 * @brief Enumerasi jumlah stop bit UART
 */
enum class UartStopBits : Uint8 {
    ONE = 0U,
    TWO = 1U
};

/**
 * @brief Enumerasi port UART yang tersedia
 */
enum class UartPort : Uint8 {
    UART1 = 0U,
    UART2 = 1U,
    UART3 = 2U
};

/**
 * @brief Enumerasi kode error UART
 */
enum class UartError : Uint8 {
    NONE = 0U,                 // Tidak ada error
    NOT_INITIALIZED = 1U,      // UART belum diinisialisasi
    INVALID_PORT = 2U,         // Port UART tidak valid
    INVALID_PARAMETER = 3U,    // Parameter tidak valid
    BUFFER_NULL = 4U,          // Buffer pointer null
    TIMEOUT = 5U,              // Operasi timeout
    HARDWARE_ERROR = 6U        // Error hardware
};

/**
 * @brief Kelas untuk mengelola UART pada drone
 */
class HalUart {
public:
    /**
     * @brief Mendapatkan instance singleton dari UART
     * @return Referensi ke instance UART
     */
    static HalUart& getInstance();

    /**
     * @brief Inisialisasi subsistem UART
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();

    /**
     * @brief Konfigurasi port UART
     * @param port Port UART
     * @param baudRate Baudrate komunikasi
     * @param parity Mode parity
     * @param stopBits Jumlah stop bits
     * @return true jika berhasil, false jika gagal
     */
    bool configurePort(UartPort port, UartBaudRate baudRate, 
                      UartParity parity, UartStopBits stopBits);

    /**
     * @brief Mengirim data melalui UART
     * @param port Port UART
     * @param data Pointer ke data yang akan dikirim
     * @param size Ukuran data dalam bytes
     * @param timeoutMs Timeout dalam milidetik (0 = tanpa timeout)
     * @return Jumlah byte yang berhasil dikirim
     */
    Uint32 transmit(UartPort port, const Uint8* data, Uint32 size, Uint32 timeoutMs = 0U);

    /**
     * @brief Menerima data dari UART
     * @param port Port UART
     * @param buffer Buffer untuk menyimpan data
     * @param size Ukuran buffer
     * @param timeoutMs Timeout dalam milidetik (0 = tanpa timeout)
     * @return Jumlah byte yang berhasil diterima
     */
    Uint32 receive(UartPort port, Uint8* buffer, Uint32 size, Uint32 timeoutMs = 0U);
    
    /**
     * @brief Mengecek apakah data tersedia untuk dibaca
     * @param port Port UART
     * @return Jumlah bytes yang tersedia
     */
    Uint32 available(UartPort port);
    
    /**
     * @brief Mendapatkan error terakhir
     * @return Kode error terakhir
     */
    UartError getLastError() const;
    
    /**
     * @brief Mengosongkan buffer UART
     * @param port Port UART
     * @return true jika berhasil, false jika gagal
     */
    bool flush(UartPort port);

private:
    // Private constructor untuk singleton
    HalUart();
    
    // Mencegah copy constructor dan assignment operator
    HalUart(const HalUart&);
    HalUart& operator=(const HalUart&);
    
    // Implementasi platform-specific
    bool initialized_;
    
    // Error handling
    UartError lastError_;
    
    // Timeout handling
    bool checkTimeout(Uint32 startTime, Uint32 timeoutMs) const;
    
    // Validasi port
    bool isValidPort(UartPort port) const;
    
    // Konstanta port maksimum (sesuai dengan jumlah port dalam enum UartPort)
    static const Uint8 MAX_PORT = 3U;
    
    // Konstanta timeout default
    static const Uint32 DEFAULT_TIMEOUT_MS = 1000U;
};

} // namespace Core
} // namespace Kamikaze

#endif // KAMIKAZE_CORE_HAL_UART_HPP_