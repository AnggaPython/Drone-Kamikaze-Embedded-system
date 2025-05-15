#ifndef KAMIKAZE_COMMUNICATION_RADIO_HPP_
#define KAMIKAZE_COMMUNICATION_RADIO_HPP_

#include "../../include/common.hpp"
#include "../core/hal_uart.hpp"

namespace Kamikaze {
namespace Communication {

/**
 * @brief Status radio
 */
enum class RadioStatus : Uint8 {
    DISCONNECTED = 0U,    // Tidak terhubung
    CONNECTING = 1U,      // Sedang menghubungkan
    CONNECTED = 2U,       // Terhubung
    ERROR = 3U            // Error
};

/**
 * @brief Jenis pesan radio
 */
enum class RadioMessageType : Uint8 {
    COMMAND = 0U,         // Perintah kontrol
    TELEMETRY = 1U,       // Data telemetri
    STATUS = 2U,          // Status
    HEARTBEAT = 3U,       // Heartbeat untuk koneksi
    ACK = 4U              // Acknowledgement
};

/**
 * @brief Tipe perintah remote control
 */
enum class CommandType : Uint8 {
    ARM = 0U,             // Arm motors
    DISARM = 1U,          // Disarm motors
    TAKEOFF = 2U,         // Takeoff
    LAND = 3U,            // Land
    GOTO = 4U,            // Go to location
    RTL = 5U,             // Return to launch
    ATTACK = 6U,          // Execute attack
    ABORT = 7U            // Abort mission
};

/**
 * @brief Struktur pesan radio
 */
struct RadioMessage {
    RadioMessageType type;        // Tipe pesan
    Uint8 length;                 // Panjang payload
    Uint8 payload[32];            // Payload data (max 32 bytes)
    
    // Konstruktor default
    RadioMessage() : 
        type(RadioMessageType::HEARTBEAT),
        length(0U) {
        // Inisialisasi array
        for (Uint8 i = 0U; i < 32U; ++i) {
            payload[i] = 0U;
        }
    }
};

/**
 * @brief Struktur perintah remote control
 */
struct Command {
    CommandType type;             // Tipe perintah
    Float32 param1;               // Parameter 1
    Float32 param2;               // Parameter 2
    Float32 param3;               // Parameter 3
    Float32 param4;               // Parameter 4
    
    // Konstruktor default
    Command() : 
        type(CommandType::DISARM),
        param1(0.0F),
        param2(0.0F),
        param3(0.0F),
        param4(0.0F) {
    }
};

/**
 * @brief Kelas untuk modul radio
 */
class Radio {
public:
    /**
     * @brief Mendapatkan instance singleton Radio
     * @return Referensi ke instance Radio
     */
    static Radio& getInstance();
    
    /**
     * @brief Inisialisasi modul radio
     * @return true jika berhasil, false jika gagal
     */
    bool initialize();
    
    /**
     * @brief Memproses pesan yang diterima
     * @return true jika ada pesan baru yang diproses, false jika tidak
     */
    bool processReceivedMessages();
    
    /**
     * @brief Mengirim pesan
     * @param message Pesan yang akan dikirim
     * @return true jika berhasil, false jika gagal
     */
    bool sendMessage(const RadioMessage& message);
    
    /**
     * @brief Mengirim perintah
     * @param command Perintah yang akan dikirim
     * @return true jika berhasil, false jika gagal
     */
    bool sendCommand(const Command& command);
    
    /**
     * @brief Mengecek status radio
     * @return Status radio
     */
    RadioStatus getStatus() const;
    
    /**
     * @brief Mendapatkan perintah terakhir
     * @return Perintah terakhir yang diterima
     */
    const Command& getLastCommand() const;
    
private:
    // Private constructor untuk singleton
    Radio();
    
    // Mencegah copy constructor dan assignment operator
    Radio(const Radio&);
    Radio& operator=(const Radio&);
    
    // Memproses pesan berdasarkan jenis
    void processMessage(const RadioMessage& message);
    
    // Decode message menjadi command
    bool decodeCommand(const RadioMessage& message, Command& command);
    
    // Encode command menjadi message
    bool encodeCommand(const Command& command, RadioMessage& message);
    
    // Status dan data
    RadioStatus status_;              // Status radio
    Command lastCommand_;             // Perintah terakhir
    Core::UartPort radioUartPort_;    // UART port untuk radio
    bool initialized_;                // Status inisialisasi
};

/**
 * @brief Fungsi inisialisasi modul komunikasi
 * @return true jika berhasil, false jika gagal
 */
bool initializeCommunication();

} // namespace Communication
} // namespace Kamikaze

#endif // KAMIKAZE_COMMUNICATION_RADIO_HPP_ 