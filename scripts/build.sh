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
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"
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

# Function to detect package manager and install dependencies
install_dependencies() {
    print_info "Detecting Linux distribution and installing dependencies..."

    local sudo_cmd=""
    if [ "$(id -u)" -ne 0 ]; then
        if command -v sudo >/dev/null 2>&1; then
            sudo_cmd="sudo"
        else
            print_error "Root privileges or sudo required to install dependencies."
            exit 1
        fi
    fi

    local distro_id="unknown"
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        distro_id="${ID:-unknown}"
    fi

    if command -v apt-get >/dev/null 2>&1; then
        print_info "Detected Debian/Ubuntu/Raspbian-based system ($distro_id)"
        $sudo_cmd apt-get update
        $sudo_cmd apt-get install -y \
            build-essential \
            cmake \
            pkg-config \
            qt6-base-dev \
            qt6-multimedia-dev \
            libqt6widgets6 \
            libqt6network6 \
            libqt6multimedia6 \
            libqt6multimediawidgets6
    elif command -v dnf >/dev/null 2>&1; then
        print_info "Detected Fedora/RHEL/CentOS-based system ($distro_id)"
        $sudo_cmd dnf install -y \
            gcc-c++ \
            make \
            cmake \
            pkgconf-pkg-config \
            qt6-qtbase-devel \
            qt6-qtmultimedia-devel
    elif command -v pacman >/dev/null 2>&1; then
        print_info "Detected Arch Linux-based system ($distro_id)"
        $sudo_cmd pacman -Sy --noconfirm --needed \
            base-devel \
            cmake \
            pkgconf \
            qt6-base \
            qt6-multimedia
    elif command -v zypper >/dev/null 2>&1; then
        print_info "Detected openSUSE/SUSE-based system ($distro_id)"
        $sudo_cmd zypper --non-interactive install -y \
            gcc-c++ \
            cmake \
            pkg-config \
            qt6-base-devel \
            qt6-multimedia-devel
    elif command -v apk >/dev/null 2>&1; then
        print_info "Detected Alpine Linux ($distro_id)"
        $sudo_cmd apk add --no-cache \
            build-base \
            cmake \
            pkgconf \
            qt6-qtbase-dev \
            qt6-qtmultimedia-dev
    elif command -v xbps-install >/dev/null 2>&1; then
        print_info "Detected Void Linux ($distro_id)"
        $sudo_cmd xbps-install -Sy \
            base-devel \
            cmake \
            pkg-config \
            qt6-base-devel \
            qt6-multimedia-devel
    else
        print_error "Unsupported package manager. Please manually install CMake, a C++ compiler, and Qt6 development libraries (Core, Widgets, Network, Multimedia)."
        exit 1
    fi
    print_success "Dependencies installed successfully!"
}

# Function to check Qt installation
check_qt() {
    print_info "Checking Qt installation..."

    if command -v cmake >/dev/null 2>&1; then
        print_info "CMake found: $(cmake --version | head -n1)"
    else
        print_error "CMake not found. Please install CMake or run: $0 --deps"
        exit 1
    fi

    # Try to find Qt6
    if pkg-config --exists "Qt6Core Qt6Widgets Qt6Network Qt6Multimedia" 2>/dev/null; then
        print_info "Qt6 found via pkg-config"
        return 0
    elif command -v qmake6 >/dev/null 2>&1; then
        QT_VERSION=$(qmake6 -query QT_VERSION 2>/dev/null || echo "unknown")
        print_info "Qt6 found via qmake6 (version: $QT_VERSION)"
        return 0
    elif cmake --find-package -DNAME=Qt6 -DCOMPONENTS="Core;Widgets;Network;Multimedia" -DMODE=EXIST >/dev/null 2>&1; then
        print_info "Qt6 found via CMake package detection"
        return 0
    else
        print_error "Qt6 development libraries not found."
        print_error "You can automatically install dependencies across any Linux distribution by running:"
        print_error "  $0 --deps"
        print_error ""
        print_error "Or manually install Qt6 development packages:"
        print_error "  - Debian/Ubuntu/Raspbian: sudo apt install qt6-base-dev qt6-multimedia-dev cmake build-essential"
        print_error "  - Fedora/RHEL/CentOS:     sudo dnf install qt6-qtbase-devel qt6-qtmultimedia-devel gcc-c++ cmake"
        print_error "  - Arch Linux / Manjaro:   sudo pacman -S qt6-base qt6-multimedia cmake base-devel"
        print_error "  - openSUSE:               sudo zypper in qt6-base-devel qt6-multimedia-devel gcc-c++ cmake"
        print_error "  - Alpine Linux:           sudo apk add qt6-qtbase-dev qt6-qtmultimedia-dev build-base cmake"
        exit 1
    fi
}

# Function to clean build artifacts
clean_build() {
    print_info "Cleaning build artifacts..."

    # Remove entire CMake build directory (including cache)
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
    cmake "$REPO_ROOT" -DCMAKE_BUILD_TYPE=Release

    # Build project
    print_info "Compiling project..."
    cmake --build . --config Release -j$(nproc 2>/dev/null || echo 4)

    # Go back to project root
    cd "$REPO_ROOT"

    mkdir -p "$BUILD_DIR/server"

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
    echo "  build, -b          Build the project (default)"
    echo "  deps, -d, --deps   Install build dependencies for your Linux distribution"
    echo "  clean, -c          Clean build artifacts"
    echo "  check              Check build environment"
    echo "  help, -h           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                 # Build project"
    echo "  $0 --deps          # Install build dependencies"
    echo "  $0 clean           # Clean build artifacts"
    echo "  $0 check           # Check build environment"
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
    "deps"|"-d"|"--deps")
        install_dependencies
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
