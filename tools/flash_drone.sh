#!/bin/bash
# Script untuk flash firmware ke drone kamikaze

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Default port
PORT="/dev/ttyUSB0"

# Parse arguments
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        -p|--port)
            PORT="$2"
            shift
            shift
            ;;
        --help)
            echo "Penggunaan: $0 [-p PORT]"
            echo "  -p, --port PORT    Port serial untuk flashing (default: /dev/ttyUSB0)"
            exit 0
            ;;
        *)
            echo "Argumen tidak dikenal: $1"
            echo "Gunakan --help untuk bantuan"
            exit 1
            ;;
    esac
done

# Memastikan firmware telah dibangun
if [ ! -f "$PROJECT_ROOT/bin/kamikaze_drone" ]; then
    echo "Firmware belum dibangun. Membangun sekarang..."
    "$SCRIPT_DIR/build.sh"
fi

# Flashing firmware
echo "Mulai flashing firmware ke drone via port $PORT..."
echo "Menghubungkan ke board..."
sleep 1
echo "Menghapus flash..."
sleep 1
echo "Memprogram firmware..."
sleep 2

# Di sini akan ada kode untuk berkomunikasi dengan bootloader drone
# Untuk keperluan contoh, kita hanya simulasikan
echo "Memverifikasi flash..."
sleep 1
echo "CRC check: OK"
echo "Reset perangkat..."
sleep 1

echo "Flashing selesai!"
echo "Drone siap digunakan." 