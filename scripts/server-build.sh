#!/bin/bash

# VideoTimeline Server Build Script

echo "Building VideoTimeline Server..."

# Check for CMake
if ! command -v cmake >/dev/null 2>&1; then
    echo "Error: CMake not found. Please install CMake."
    exit 1
fi

echo "Using: cmake"

# Clean up any previous build artifacts
if [ -d "server/build" ]; then
    echo "Cleaning up old build artifacts..."
    rm -rf server/build
fi

# Create build directory
mkdir -p server/build
cd server/build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Compiling..."
make -j$(nproc 2>/dev/null || echo 4)

cd ../..

if [ -f "server/build/server" ] || [ -f "server/build/server.exe" ]; then
    echo "✓ Build successful!"
    # Copy to main build directory
    mkdir -p build/server
    cp server/build/server* build/server/ 2>/dev/null || true
    echo "Binary copied to build/server/"
else
    echo "✗ Build failed!"
    exit 1
fi

