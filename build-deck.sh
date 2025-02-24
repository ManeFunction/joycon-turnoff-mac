#!/bin/bash

# Exit on any error
set -e

# Function to restore backups if build fails
restore_backups() {
    echo "Build failed, restoring original files..."
    if [ -d "/tmp/jcoff_backup" ]; then
        sudo cp -r /tmp/jcoff_backup/* /
        sudo rm -rf /tmp/jcoff_backup
        echo "Original files restored."
    fi
    # Clean up temp directory if it exists
    [ -d "$TEMP_DIR" ] && rm -rf "$TEMP_DIR"
    exit 1
}

# Set up trap to restore backups on script failure
trap restore_backups ERR

echo "Backing up existing files..."
sudo mkdir -p /tmp/jcoff_backup/usr/lib
sudo mkdir -p /tmp/jcoff_backup/usr/include
sudo mkdir -p /tmp/jcoff_backup/usr/lib/pkgconfig

# Only backup files if they exist
[ -f /usr/lib/libudev.a ] && sudo cp /usr/lib/libudev.a /tmp/jcoff_backup/usr/lib/
[ -f /usr/include/libudev.h ] && sudo cp /usr/include/libudev.h /tmp/jcoff_backup/usr/include/
[ -f /usr/lib/pkgconfig/libudev.pc ] && sudo cp /usr/lib/pkgconfig/libudev.pc /tmp/jcoff_backup/usr/lib/pkgconfig/

# Only remove our custom-built static libraries and headers
echo "Cleaning previous custom builds..."
sudo rm -f /usr/local/lib/libhidapi*.a
sudo rm -f /usr/local/include/hidapi/hidapi.h
rm -rf build/

echo "Installing static development libraries..."
sudo pacman -S --needed --noconfirm \
    cmake \
    hidapi \
    libusb \
    systemd-libs

# Build
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Show info
ldd ./jcoff
file ./jcoff

echo "Build complete! Binary is at: $(pwd)/jcoff" 