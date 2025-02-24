#!/bin/bash

# Exit on error
set -e

# Install dependencies
brew install cmake hidapi

# Build
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(sysctl -n hw.ncpu)

# Show info
otool -L ./jcoff
file ./jcoff

echo "Build complete! Binary is at: $(pwd)/jcoff" 