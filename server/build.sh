#!/bin/bash

# Simple build script for VideoTimeline Server

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
if [ -f "Makefile" ]; then
    echo "Cleaning up old build artifacts..."
    make clean 2>/dev/null || true
    rm -f Makefile .qmake.stash
fi

# Create necessary directories
mkdir -p moc obj rcc

# Generate Makefile
echo "Generating Makefile..."
$QMAKE server.pro

# Build
echo "Compiling..."
make -j$(nproc 2>/dev/null || echo 4)

if [ -f "server" ] || [ -f "server.exe" ]; then
    echo "✓ Build successful!"
    echo "Run with: ./server/server"
else
    echo "✗ Build failed!"
    exit 1
fi

