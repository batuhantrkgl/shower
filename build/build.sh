#!/bin/bash
# Universal Build Script for VideoTimeline
# Supports both qmake and CMake build systems
# Works on Linux, macOS, and Windows (with bash)

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="VideoTimeline"
BUILD_DIR="build"
OUTPUT_DIR="out"
SRC_DIR="src"

# Function to print colored output
print_status() {
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

# Function to detect available build systems
detect_build_systems() {
    local qmake_found=false
    local cmake_found=false

    # Check for qmake (try different variants)
    if command -v qmake6 >/dev/null 2>&1; then
        QMAKE_CMD="qmake6"
        qmake_found=true
    elif command -v qmake >/dev/null 2>&1; then
        QMAKE_CMD="qmake"
        qmake_found=true
    fi

    # Check for cmake
    if command -v cmake >/dev/null 2>&1; then
        CMAKE_CMD="cmake"
        cmake_found=true
    fi

    # Check for make
    if command -v make >/dev/null 2>&1; then
        MAKE_CMD="make"
    elif command -v mingw32-make >/dev/null 2>&1; then
        MAKE_CMD="mingw32-make"
    elif command -v nmake >/dev/null 2>&1; then
        MAKE_CMD="nmake"
    else
        MAKE_CMD=""
    fi

    echo "$qmake_found,$cmake_found"
}

# Function to check Qt installation
check_qt() {
    print_status "Checking Qt installation..."

    # Try to find Qt version
    if command -v qmake6 >/dev/null 2>&1; then
        QT_VERSION=$(qmake6 -query QT_VERSION 2>/dev/null || echo "unknown")
        print_status "Found Qt6 version: $QT_VERSION"
        return 0
    elif command -v qmake >/dev/null 2>&1; then
        QT_VERSION=$(qmake -query QT_VERSION 2>/dev/null || echo "unknown")
        print_status "Found Qt version: $QT_VERSION"
        return 0
    else
        print_warning "Qt not found in PATH"
        return 1
    fi
}

# Function to build with qmake
build_with_qmake() {
    print_status "Building with qmake..."

    # Clean previous build
    if [ -f "$SRC_DIR/Makefile" ]; then
        print_status "Cleaning previous build..."
        cd $SRC_DIR
        $MAKE_CMD clean 2>/dev/null || true
        rm -f Makefile 2>/dev/null || true
        cd ..
    fi

    # Generate Makefile
    print_status "Generating Makefile..."
    $QMAKE_CMD $SRC_DIR/$PROJECT_NAME.pro

    if [ ! -f "Makefile" ]; then
        print_error "Failed to generate Makefile"
        return 1
    fi

    # Build project
    print_status "Compiling project..."
    cd $SRC_DIR
    $MAKE_CMD -j$(nproc 2>/dev/null || echo 4)
    cd ..

    # Check if executable was created
    if [ -f "$SRC_DIR/$OUTPUT_DIR/$PROJECT_NAME" ] || [ -f "$SRC_DIR/$OUTPUT_DIR/$PROJECT_NAME.exe" ]; then
        print_success "Build completed successfully with qmake"
        # Copy executable to root output directory
        mkdir -p "../$OUTPUT_DIR"
        cp "$SRC_DIR/$OUTPUT_DIR/$PROJECT_NAME"* "../$OUTPUT_DIR/" 2>/dev/null || true
        return 0
    else
        print_error "Build failed - executable not found"
        return 1
    fi
}

# Function to build with CMake
build_with_cmake() {
    print_status "Building with CMake..."

    # Create build directory
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Configure project
    print_status "Configuring project..."
    $CMAKE_CMD .. -DCMAKE_BUILD_TYPE=Release

    # Build project
    print_status "Compiling project..."
    $CMAKE_CMD --build . --config Release -j $(nproc 2>/dev/null || echo 4)

    # Go back to project root
    cd ..

    # Check if executable was created
    if [ -f "$BUILD_DIR/$OUTPUT_DIR/$PROJECT_NAME" ] || [ -f "$BUILD_DIR/$OUTPUT_DIR/$PROJECT_NAME.exe" ]; then
        # Copy executable to main output directory
        mkdir -p "$OUTPUT_DIR"
        cp "$BUILD_DIR/$OUTPUT_DIR/$PROJECT_NAME"* "$OUTPUT_DIR/" 2>/dev/null || true
        print_success "Build completed successfully with CMake"
        return 0
    else
        print_error "Build failed - executable not found"
        return 1
    fi
}

# Function to clean build artifacts
clean_build() {
    print_status "Cleaning build artifacts..."

    # Remove qmake artifacts
    rm -f Makefile* 2>/dev/null || true
    rm -f .qmake.stash 2>/dev/null || true
    rm -rf $OUTPUT_DIR/obj $OUTPUT_DIR/moc $OUTPUT_DIR/rcc $OUTPUT_DIR/ui 2>/dev/null || true

    # Remove CMake artifacts
    rm -rf "$BUILD_DIR" 2>/dev/null || true

    # Remove object files
    find . -name "*.o" -delete 2>/dev/null || true
    find . -name "*.obj" -delete 2>/dev/null || true

    print_success "Clean completed"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTION]"
    echo ""
    echo "Options:"
    echo "  build, -b     Build the project (default)"
    echo "  clean, -c     Clean build artifacts"
    echo "  qmake, -q     Force build with qmake"
    echo "  cmake, -m     Force build with CMake"
    echo "  help, -h      Show this help message"
    echo "  check         Check build environment"
    echo ""
    echo "Examples:"
    echo "  $0                # Auto-detect and build"
    echo "  $0 clean          # Clean build artifacts"
    echo "  $0 qmake          # Force qmake build"
    echo "  $0 cmake          # Force CMake build"
}

# Function to check build environment
check_environment() {
    print_status "Checking build environment..."

    # Check operating system
    OS=$(uname -s 2>/dev/null || echo "Windows")
    print_status "Operating System: $OS"

    # Check Qt
    check_qt

    # Check build systems
    IFS=',' read -r qmake_available cmake_available <<< "$(detect_build_systems)"

    echo ""
    print_status "Available build systems:"
    if [ "$qmake_available" = "true" ]; then
        echo "  ✓ qmake ($QMAKE_CMD)"
    else
        echo "  ✗ qmake"
    fi

    if [ "$cmake_available" = "true" ]; then
        echo "  ✓ CMake ($CMAKE_CMD)"
    else
        echo "  ✗ CMake"
    fi

    if [ -n "$MAKE_CMD" ]; then
        echo "  ✓ Make ($MAKE_CMD)"
    else
        echo "  ✗ Make"
    fi

    echo ""
    print_status "Required files:"
    if [ -f "$PROJECT_NAME.pro" ]; then
        echo "  ✓ $PROJECT_NAME.pro (qmake)"
    else
        echo "  ✗ $PROJECT_NAME.pro"
    fi

    if [ -f "CMakeLists.txt" ]; then
        echo "  ✓ CMakeLists.txt (CMake)"
    else
        echo "  ✗ CMakeLists.txt"
    fi

    # Source files check
    echo ""
    print_status "Source files:"
    for file in main.cpp mainwindow.cpp videowidget.cpp mediaplayer.cpp; do
        if [ -f "$file" ]; then
            echo "  ✓ $file"
        else
            echo "  ✗ $file"
        fi
    done
}

# Main build function
main_build() {
    local force_system="$1"

    print_status "Starting build process..."

    # Detect available build systems
    IFS=',' read -r qmake_available cmake_available <<< "$(detect_build_systems)"

    # Choose build system
    if [ "$force_system" = "qmake" ]; then
        if [ "$qmake_available" = "true" ]; then
            build_with_qmake
        else
            print_error "qmake not available"
            return 1
        fi
    elif [ "$force_system" = "cmake" ]; then
        if [ "$cmake_available" = "true" ]; then
            build_with_cmake
        else
            print_error "CMake not available"
            return 1
        fi
    else
        # Auto-detect best build system
        if [ "$qmake_available" = "true" ] && [ -f "$PROJECT_NAME.pro" ]; then
            print_status "Auto-detected: using qmake"
            build_with_qmake
        elif [ "$cmake_available" = "true" ] && [ -f "CMakeLists.txt" ]; then
            print_status "Auto-detected: using CMake"
            build_with_cmake
        else
            print_error "No suitable build system found"
            print_error "Please install Qt development tools or CMake"
            return 1
        fi
    fi
}

# Main script logic
case "${1:-build}" in
    "build"|"-b"|"")
        main_build
        ;;
    "clean"|"-c")
        clean_build
        ;;
    "qmake"|"-q")
        main_build "qmake"
        ;;
    "cmake"|"-m")
        main_build "cmake"
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
