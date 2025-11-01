#!/bin/bash

# VideoTimeline Server Build Script

echo "Building VideoTimeline Server..."

# Check for Qt6
if command -v qmake6 >/dev/null 2>&1; then
    QMAKE="qmake6"
elif command -v qmake >/dev/null 2>&1; then
    QMAKE="qmake"
    # Check if it's Qt6
    QT_VERSION=$(qmake -query QT_VERSION 2>/dev/null | cut -d. -f1)
    if [ "$QT_VERSION" != "6" ]; then
        echo "Error: Qt6 is required (found Qt$QT_VERSION)"
        exit 1
    fi
else
    echo "Error: qmake not found. Please install Qt6 development tools."
    exit 1
fi

echo "Using: $QMAKE"

# Clean up any previous in-place build artifacts
if [ -f "server/Makefile" ]; then
    echo "Cleaning up old build artifacts..."
    cd server
    make clean 2>/dev/null || true
    rm -f Makefile .qmake.stash
    cd ..
fi

# Create necessary directories
mkdir -p server/moc server/obj server/rcc

# Generate Makefile
echo "Generating Makefile..."
$QMAKE server/server.pro

# Build
echo "Compiling..."
cd server
make -j$(nproc 2>/dev/null || echo 4)
cd ..

if [ -f "server/server" ] || [ -f "server/server.exe" ]; then
    echo "✓ Build successful!"
    # Copy to build directory
    mkdir -p build/server
    cp server/server* build/server/ 2>/dev/null || true
    echo "Binary copied to build/server/"
else
    echo "✗ Build failed!"
    exit 1
fi

