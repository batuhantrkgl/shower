#!/bin/bash

# Unified VideoTimeline Build Script
# Builds both the main application and server

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
BUILD_DIR="build"
CMAKE_BUILD_DIR="$BUILD_DIR/cmake"

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check Qt installation
check_qt() {
    print_info "Checking Qt installation..."

    if command -v cmake >/dev/null 2>&1; then
        print_info "CMake found: $(cmake --version | head -n1)"
    else
        print_error "CMake not found. Please install CMake."
        exit 1
    fi

    # Try to find Qt6
    if pkg-config --exists "Qt6Core Qt6Widgets Qt6Network Qt6Multimedia"; then
        print_info "Qt6 found via pkg-config"
        return 0
    elif command -v qmake6 >/dev/null 2>&1; then
        QT_VERSION=$(qmake6 -query QT_VERSION 2>/dev/null || echo "unknown")
        print_info "Qt6 found via qmake6 (version: $QT_VERSION)"
        return 0
    else
        print_error "Qt6 development libraries not found."
        print_error "Please install Qt6 development packages:"
        print_error "  - Ubuntu/Debian: sudo apt install qt6-base-dev qt6-multimedia-dev cmake"
        print_error "  - Fedora: sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel cmake"
        print_error "  - Arch: sudo pacman -S qt6-base qt6-multimedia cmake"
        exit 1
    fi
}

# Function to clean build artifacts
clean_build() {
    print_info "Cleaning build artifacts..."

    # Remove CMake build directory
    rm -rf "$CMAKE_BUILD_DIR"

    # Remove built binaries
    rm -f "$BUILD_DIR/VideoTimeline"
    rm -f "$BUILD_DIR/server/server"

    print_success "Clean completed"
}

# Function to build project
build_project() {
    print_info "Building VideoTimeline project..."

    # Create CMake build directory
    mkdir -p "$CMAKE_BUILD_DIR"
    cd "$CMAKE_BUILD_DIR"

    # Configure project
    print_info "Configuring project with CMake..."
    cmake ../.. -DCMAKE_BUILD_TYPE=Release

    # Build project
    print_info "Compiling project..."
    cmake --build . --config Release -j$(nproc 2>/dev/null || echo 4)

    # Go back to project root
    cd ../..

    # Copy binaries to build directory
    if [ -f "$CMAKE_BUILD_DIR/bin/VideoTimeline" ]; then
        cp "$CMAKE_BUILD_DIR/bin/VideoTimeline" "$BUILD_DIR/"
        print_success "Main application built successfully"
    else
        print_error "Main application binary not found"
        return 1
    fi

    if [ -f "$CMAKE_BUILD_DIR/bin/server" ]; then
        cp "$CMAKE_BUILD_DIR/bin/server" "$BUILD_DIR/server/"
        print_success "Server built successfully"
    else
        print_error "Server binary not found"
        return 1
    fi

    print_success "Build completed successfully!"
    print_info "Binaries are located in: $BUILD_DIR/"
}

# Function to show usage
show_usage() {
    echo "VideoTimeline Unified Build Script"
    echo ""
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  build, -b     Build the project (default)"
    echo "  clean, -c     Clean build artifacts"
    echo "  check         Check build environment"
    echo "  help, -h      Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                # Build project"
    echo "  $0 clean          # Clean build artifacts"
    echo "  $0 check          # Check build environment"
}

# Function to check build environment
check_environment() {
    print_info "Checking build environment..."

    # Check operating system
    OS=$(uname -s 2>/dev/null || echo "Windows")
    print_info "Operating System: $OS"

    # Check Qt
    check_qt

    # Check available tools
    echo ""
    print_info "Available tools:"
    if command -v cmake >/dev/null 2>&1; then
        echo "  ✓ CMake ($(cmake --version | head -n1))"
    else
        echo "  ✗ CMake"
    fi

    if command -v make >/dev/null 2>&1; then
        echo "  ✓ Make"
    else
        echo "  ✗ Make"
    fi

    if command -v g++ >/dev/null 2>&1; then
        echo "  ✓ G++ ($(g++ --version | head -n1))"
    elif command -v clang++ >/dev/null 2>&1; then
        echo "  ✓ Clang++ ($(clang++ --version | head -n1))"
    else
        echo "  ✗ C++ compiler"
    fi

    # Check source files
    echo ""
    print_info "Source files:"
    for file in src/main.cpp server/server.cpp; do
        if [ -f "$file" ]; then
            echo "  ✓ $file"
        else
            echo "  ✗ $file"
        fi
    done

    if [ -f "CMakeLists.txt" ]; then
        echo "  ✓ CMakeLists.txt (root)"
    else
        echo "  ✗ CMakeLists.txt"
    fi
}

# Main script logic
case "${1:-build}" in
    "build"|"-b"|"")
        check_qt
        build_project
        ;;
    "clean"|"-c")
        clean_build
        ;;
    "check")
        check_environment
        ;;
    "help"|"-h"|"--help")
        show_usage
        ;;
    *)
        print_error "Unknown option: $1"
        show_usage
        exit 1
        ;;
esac
