#include "radio.hpp"

namespace Kamikaze {
namespace Communication {

// Implementasi singleton
Radio& Radio::getInstance() {
    static Radio instance;
    return instance;
}

// Konstruktor
Radio::Radio() :
    status_(RadioStatus::DISCONNECTED),
    lastCommand_(),
    radioUartPort_(Core::UartPort::UART3),
    initialized_(false) {
    // Tidak ada inisialisasi tambahan
}

// Inisialisasi
bool Radio::initialize() {
    if (initialized_) {
        return true;
    }
    
    // Konfigurasi UART untuk radio
    Core::HalUart& uart = Core::HalUart::getInstance();
    bool result = uart.configurePort(
        radioUartPort_,
        Core::UartBaudRate::BAUD_57600,
        Core::UartParity::NONE,
        Core::UartStopBits::ONE
    );
    
    if (result) {
        status_ = RadioStatus::CONNECTING;
    } else {
        status_ = RadioStatus::ERROR;
    }
    
    initialized_ = result;
    return result;
}

// Proses pesan yang diterima
bool Radio::processReceivedMessages() {
    if (!initialized_) {
        return false;
    }
    
    // Cek data yang tersedia di UART
    Core::HalUart& uart = Core::HalUart::getInstance();
    Uint32 bytesAvailable = uart.available(radioUartPort_);
    
    if (bytesAvailable == 0U) {
        return false;
    }
    
    // Buffer untuk menerima data
    static const Uint32 BUFFER_SIZE = 64U;
    static Uint8 buffer[BUFFER_SIZE];
    
    // Baca data dari UART
    Uint32 bytesRead = uart.receive(radioUartPort_, buffer, BUFFER_SIZE - 1U);
    
    if (bytesRead == 0U) {
        return false;
    }
    
    // Parse pesan
    // Dalam implementasi sebenarnya, ini akan melibatkan parsing protokol yang tepat
    
    // Contoh: Buat pesan dummy untuk simulasi
    RadioMessage message;
    message.type = RadioMessageType::COMMAND;
    message.length = 1U;
    message.payload[0] = static_cast<Uint8>(CommandType::ARM);
    
    // Proses pesan
    processMessage(message);
    
    // Update status
    status_ = RadioStatus::CONNECTED;
    
    return true;
}

// Kirim pesan
bool Radio::sendMessage(const RadioMessage& message) {
    if (!initialized_) {
        return false;
    }
    
    // Komposisi buffer pesan
    static const Uint32 BUFFER_SIZE = 64U;
    static Uint8 buffer[BUFFER_SIZE];
    
    // Header: tipe pesan (1 byte) + panjang payload (1 byte)
    buffer[0] = static_cast<Uint8>(message.type);
    buffer[1] = message.length;
    
    // Copy payload
    for (Uint8 i = 0U; i < message.length && i < 32U; ++i) {
        buffer[i + 2U] = message.payload[i];
    }
    
    // Ukuran total pesan
    Uint32 messageSize = static_cast<Uint32>(2U + message.length);
    
    // Kirim data via UART
    Core::HalUart& uart = Core::HalUart::getInstance();
    Uint32 bytesSent = uart.transmit(radioUartPort_, buffer, messageSize);
    
    return (bytesSent == messageSize);
}

// Kirim perintah
bool Radio::sendCommand(const Command& command) {
    // Encode command menjadi message
    RadioMessage message;
    if (!encodeCommand(command, message)) {
        return false;
    }
    
    // Kirim message
    return sendMessage(message);
}

// Mendapatkan status radio
RadioStatus Radio::getStatus() const {
    return status_;
}

// Mendapatkan perintah terakhir
const Command& Radio::getLastCommand() const {
    return lastCommand_;
}

// Memproses pesan berdasarkan jenisnya
void Radio::processMessage(const RadioMessage& message) {
    switch (message.type) {
        case RadioMessageType::COMMAND:
            // Decode command
            decodeCommand(message, lastCommand_);
            break;
            
        case RadioMessageType::HEARTBEAT:
            // Update status koneksi
            status_ = RadioStatus::CONNECTED;
            
            // Kirim heartbeat ACK
            RadioMessage ack;
            ack.type = RadioMessageType::ACK;
            sendMessage(ack);
            break;
            
        case RadioMessageType::ACK:
            // Acknowledgement diterima
            break;
            
        case RadioMessageType::STATUS:
        case RadioMessageType::TELEMETRY:
            // Proses sesuai kebutuhan
            break;
            
        default:
            // Pesan tidak dikenal
            break;
    }
}

// Decode message menjadi command
bool Radio::decodeCommand(const RadioMessage& message, Command& command) {
    if (message.type != RadioMessageType::COMMAND || message.length < 1U) {
        return false;
    }
    
    // Decode command type
    command.type = static_cast<CommandType>(message.payload[0]);
    
    // Decode parameters jika ada
    if (message.length >= 5U) {
        // Parameter 1 (Bytes 1-4)
        // Dalam implementasi nyata, ini akan melibatkan konversi byte ke float
        command.param1 = static_cast<Float32>(message.payload[1]);
    }
    
    if (message.length >= 9U) {
        // Parameter 2 (Bytes 5-8)
        command.param2 = static_cast<Float32>(message.payload[5]);
    }
    
    if (message.length >= 13U) {
        // Parameter 3 (Bytes 9-12)
        command.param3 = static_cast<Float32>(message.payload[9]);
    }
    
    if (message.length >= 17U) {
        // Parameter 4 (Bytes 13-16)
        command.param4 = static_cast<Float32>(message.payload[13]);
    }
    
    return true;
}

// Encode command menjadi message
bool Radio::encodeCommand(const Command& command, RadioMessage& message) {
    // Set tipe pesan dan panjang payload
    message.type = RadioMessageType::COMMAND;
    message.length = 17U;  // 1 byte tipe + 4x4 bytes parameter
    
    // Encode command type
    message.payload[0] = static_cast<Uint8>(command.type);
    
    // Encode parameters
    // Dalam implementasi nyata, ini akan melibatkan konversi float ke byte
    message.payload[1] = static_cast<Uint8>(command.param1);
    message.payload[5] = static_cast<Uint8>(command.param2);
    message.payload[9] = static_cast<Uint8>(command.param3);
    message.payload[13] = static_cast<Uint8>(command.param4);
    
    return true;
}

// Inisialisasi modul komunikasi
bool initializeCommunication() {
    // Inisialisasi radio
    return Radio::getInstance().initialize();
}

} // namespace Communication
} // namespace Kamikaze 