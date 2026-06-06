#!/bin/bash
set -e

# Force execution from the project root directory
cd "$(dirname "$0")/.."

# Configurable paths
BUILD_DIR=${1:-build-release}
STAGING_DIR="dist"

# Use the LINUXDEPLOY environment variable if set, otherwise fallback to Downloads
LINUXDEPLOY_BIN=${LINUXDEPLOY:-"$HOME/Downloads/linuxdeploy-x86_64.AppImage"}

if [ ! -f "$LINUXDEPLOY_BIN" ]; then
    echo "Error: linuxdeploy binary not found at '$LINUXDEPLOY_BIN'."
    exit 1
fi

if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory '$BUILD_DIR' not found. Please compile the project first."
    exit 1
fi

echo "Cleaning previous AppImage staging areas..."
rm -rf "$STAGING_DIR/AppDir" Monkey-Moore*.AppImage
mkdir -p "$STAGING_DIR/AppDir"

echo "Staging files via CMake install..."
# Force the prefix to /usr so the folder structure matches standard Linux expectations
DESTDIR="$STAGING_DIR/AppDir" cmake --install "$BUILD_DIR" --prefix "/usr"

echo "Running linuxdeploy..."
# 1. Export ARCH to prevent architecture guessing errors
export ARCH=x86_64

# 2. Run linuxdeploy
# We point to the new desktop file location.
# We removed --icon-file because CMake is now installing the icons directly into 
# AppDir/usr/share/icons, so linuxdeploy will find and package them automatically.
"$LINUXDEPLOY_BIN" \
    --appdir "$STAGING_DIR/AppDir" \
    --executable "$STAGING_DIR/AppDir/usr/bin/monkey-moore" \
    --desktop-file assets/platform/linux/monkey-moore.desktop \
    --output appimage

echo "Moving generated AppImage to staging folder..."
mv Monkey-Moore*.AppImage "$STAGING_DIR/"

echo "Packaging complete! Your AppImage is located in the '$STAGING_DIR' folder."