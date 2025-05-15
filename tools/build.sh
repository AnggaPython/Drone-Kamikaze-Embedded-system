#!/bin/bash
# Script untuk build drone kamikaze firmware

set -e

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Masuk ke direktori proyek
cd "$PROJECT_ROOT"

# Membuat direktori yang diperlukan
mkdir -p bin obj

# Menjalankan build dengan make
echo "Memulai build firmware drone kamikaze..."
make -f config/build_config.mk

# Memeriksa hasil build
if [ $? -eq 0 ]; then
    echo "Build berhasil! Binary ada di folder bin/"
    ls -lh bin/
else
    echo "Build gagal!"
    exit 1
fi

# Menjalankan MISRA checker jika diminta
if [ "$1" == "--check-misra" ]; then
    echo "Menjalankan MISRA checker..."
    "$SCRIPT_DIR/misra_check.sh"
fi

echo "Selesai." 