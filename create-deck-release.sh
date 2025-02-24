#!/bin/bash

VERSION="2.1.0"
PACKAGE_NAME="joycon-turnoff-steamdeck-${VERSION}"

# Create package directory
mkdir -p "${PACKAGE_NAME}"

# Copy required files
cp build/jcoff "${PACKAGE_NAME}/"
cp deck-install.sh "${PACKAGE_NAME}/"
cp deck-uninstall.sh "${PACKAGE_NAME}/"
cp README.md "${PACKAGE_NAME}/"
cp LICENSE "${PACKAGE_NAME}/"

# Create archive
tar -czf "${PACKAGE_NAME}.tar.gz" "${PACKAGE_NAME}"

echo "Package created: ${PACKAGE_NAME}.tar.gz"
echo "Contents:"
tar -tvf "${PACKAGE_NAME}.tar.gz"

# Cleanup
rm -rf "${PACKAGE_NAME}" 