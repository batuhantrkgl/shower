#!/bin/bash

# ==============================================================================
# VideoTimeline Unified Management Script
# Combines: Build, Run, Server, Hardware Acceleration, Installation, Fonts, Media
# ==============================================================================

set -e

# Ensure standard POSIX path utilities are available (e.g. MSYS2/Git Bash on Windows)
if [ -d "/usr/bin" ] && [[ ":$PATH:" != *":/usr/bin:"* ]]; then
    export PATH="/usr/bin:$PATH"
fi

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Root & paths configuration
if command -v dirname >/dev/null 2>&1; then
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" 2>/dev/null && pwd)"
else
    SCRIPT_DIR="${BASH_SOURCE[0]%/*}"
    [ "$SCRIPT_DIR" = "${BASH_SOURCE[0]}" ] && SCRIPT_DIR="."
    SCRIPT_DIR="$(cd "$SCRIPT_DIR" 2>/dev/null && pwd)"
fi
REPO_ROOT="$SCRIPT_DIR"
BUILD_DIR="$REPO_ROOT/build"
CMAKE_BUILD_DIR="$BUILD_DIR/cmake"
APP_BINARY="$BUILD_DIR/VideoTimeline"
SERVER_BINARY="$BUILD_DIR/server/server"
DEFAULT_SERVER_HOST="localhost"
DEFAULT_SERVER_PORT=3232
FONT_SRC_DIR="$REPO_ROOT/SFProDisplayFonts"

# Print helpers
print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_app() { echo -e "${CYAN}[APP]${NC} $1"; }
print_server() { echo -e "${MAGENTA}[SERVER]${NC} $1"; }

# Detect Linux distribution helper
detect_linux_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO_ID="${ID:-unknown}"
        DISTRO_LIKE="${ID_LIKE:-}"
    else
        DISTRO_ID="unknown"
        DISTRO_LIKE=""
    fi
}

get_sudo_cmd() {
    if [ "$(id -u)" -ne 0 ]; then
        if command -v sudo >/dev/null 2>&1; then
            echo "sudo"
        else
            print_error "Root privileges or sudo required."
            exit 1
        fi
    fi
}

PROG="./videotimeline.sh"

# ==============================================================================
# HELP & USAGE
# ==============================================================================
show_usage() {
    cat << EOF
$(echo -e "${CYAN}VideoTimeline Master Control Script${NC}")
Usage: $PROG [COMMAND] [OPTIONS]

$(echo -e "${YELLOW}Commands:${NC}")
  run               Run the VideoTimeline client application (default)
  server            Manage and run the VideoTimeline HTTP server
  build             Build client and/or server binaries with CMake
  install           Universal Linux setup (packages, kiosk, systemd, fonts)
  fonts             Install SF Pro Display fonts (system-wide or user)
  media             Media conversion and special playlist generation
  help, -h          Show this help message

$(echo -e "${YELLOW}Quick Examples:${NC}")
  $PROG                                  # Run client application
  $PROG run --auto                       # Auto-discover server and start playback
  $PROG run --hwaccel --nvidia           # Run with NVIDIA hardware acceleration
  $PROG server                           # Start server on default port (3232)
  $PROG server --status                  # Check if server is running
  $PROG server --kill                    # Stop all running server processes
  $PROG build                            # Build entire project (client + server)
  $PROG build --deps                     # Install build dependencies on any Linux distro
  $PROG build clean                      # Clean all build artifacts
  $PROG install                          # Full kiosk and service setup on any Linux distro
  $PROG fonts                            # Install SF Pro Display fonts
  $PROG media reencode data/media/special # Re-encode AV1/VP9 videos to H.264

Run '$PROG [command] --help' for detailed options on any command.
EOF
}

# ==============================================================================
# SUBCOMMAND: BUILD
# ==============================================================================
show_build_usage() {
    cat << EOF
VideoTimeline Build Tool
Usage: $PROG build [OPTIONS]

Options:
  all, -b, ""         Build both client and server (default)
  --server-only       Build only the HTTP server
  --client-only       Build only the client application
  --deps, -d          Install build dependencies for your Linux distribution
  clean, -c           Clean build artifacts
  check               Check build environment and tools
  -h, --help          Show this build help

Examples:
  $PROG build            # Build everything
  $PROG build --deps     # Install Qt6 and compiler dependencies
  $PROG build clean      # Remove build directories
  $PROG build check      # Inspect tools and libraries
EOF
}

install_build_dependencies() {
    print_info "Detecting Linux distribution and installing build dependencies..."
    local sudo_cmd=$(get_sudo_cmd)
    detect_linux_distro

    if command -v apt-get >/dev/null 2>&1; then
        print_info "Detected Debian/Ubuntu/Raspbian-based system ($DISTRO_ID)"
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
        print_info "Detected Fedora/RHEL/CentOS-based system ($DISTRO_ID)"
        $sudo_cmd dnf install -y \
            gcc-c++ \
            make \
            cmake \
            pkgconf-pkg-config \
            qt6-qtbase-devel \
            qt6-qtmultimedia-devel
    elif command -v pacman >/dev/null 2>&1; then
        print_info "Detected Arch Linux-based system ($DISTRO_ID)"
        $sudo_cmd pacman -Sy --noconfirm --needed \
            base-devel \
            cmake \
            pkgconf \
            qt6-base \
            qt6-multimedia
    elif command -v zypper >/dev/null 2>&1; then
        print_info "Detected openSUSE/SUSE-based system ($DISTRO_ID)"
        $sudo_cmd zypper --non-interactive install -y \
            gcc-c++ \
            cmake \
            pkg-config \
            qt6-base-devel \
            qt6-multimedia-devel
    elif command -v apk >/dev/null 2>&1; then
        print_info "Detected Alpine Linux ($DISTRO_ID)"
        $sudo_cmd apk add --no-cache \
            build-base \
            cmake \
            pkgconf \
            qt6-qtbase-dev \
            qt6-qtmultimedia-dev
    elif command -v xbps-install >/dev/null 2>&1; then
        print_info "Detected Void Linux ($DISTRO_ID)"
        $sudo_cmd xbps-install -Sy \
            base-devel \
            cmake \
            pkg-config \
            qt6-base-devel \
            qt6-multimedia-devel
    else
        print_error "Unsupported package manager. Please manually install CMake, a C++ compiler, and Qt6 development libraries."
        exit 1
    fi
    print_success "Build dependencies installed successfully!"
}

check_qt() {
    print_info "Checking Qt installation..."
    if ! command -v cmake >/dev/null 2>&1; then
        print_error "CMake not found. Please install CMake or run: $0 build --deps"
        exit 1
    fi

    if pkg-config --exists "Qt6Core Qt6Widgets Qt6Network Qt6Multimedia" 2>/dev/null; then
        print_info "Qt6 found via pkg-config"
        return 0
    elif command -v qmake6 >/dev/null 2>&1; then
        local qtv=$(qmake6 -query QT_VERSION 2>/dev/null || echo "unknown")
        print_info "Qt6 found via qmake6 (version: $qtv)"
        return 0
    elif cmake --find-package -DNAME=Qt6 -DCOMPONENTS="Core;Widgets;Network;Multimedia" -DMODE=EXIST >/dev/null 2>&1; then
        print_info "Qt6 found via CMake package detection"
        return 0
    else
        print_error "Qt6 development libraries not found."
        print_error "Run '$0 build --deps' to install automatically on your distribution."
        exit 1
    fi
}

clean_build() {
    print_info "Cleaning build artifacts..."
    rm -rf "$CMAKE_BUILD_DIR"
    rm -rf "$REPO_ROOT/server/build"
    rm -f "$BUILD_DIR/VideoTimeline" "$BUILD_DIR/VideoTimeline.exe"
    rm -f "$BUILD_DIR/server/server" "$BUILD_DIR/server/server.exe"
    print_success "Clean completed"
}

check_build_environment() {
    print_info "Checking build environment..."
    local os=$(uname -s 2>/dev/null || echo "Windows")
    print_info "Operating System: $os"

    check_qt

    echo ""
    print_info "Available tools:"
    for tool in cmake make ninja gcc g++ clang clang++; do
        if command -v $tool >/dev/null 2>&1; then
            echo "  ✓ $tool ($($tool --version 2>&1 | head -n1))"
        fi
    done

    echo ""
    print_info "Source files check:"
    for file in src/main.cpp server/server.cpp CMakeLists.txt; do
        if [ -f "$REPO_ROOT/$file" ]; then
            echo "  ✓ $file"
        else
            echo "  ✗ $file"
        fi
    done
}

build_all() {
    check_qt
    print_info "Building VideoTimeline project..."
    mkdir -p "$CMAKE_BUILD_DIR"
    cd "$CMAKE_BUILD_DIR"

    print_info "Configuring project with CMake..."
    cmake "$REPO_ROOT" -DCMAKE_BUILD_TYPE=Release

    print_info "Compiling project..."
    local jobs=$(nproc 2>/dev/null || echo 4)
    cmake --build . --config Release -j"$jobs"

    cd "$REPO_ROOT"
    mkdir -p "$BUILD_DIR/server"

    # Copy binaries
    local found_client=false
    local found_server=false

    for bin_path in "$CMAKE_BUILD_DIR/bin/VideoTimeline" "$CMAKE_BUILD_DIR/bin/VideoTimeline.exe" "$CMAKE_BUILD_DIR/VideoTimeline" "$CMAKE_BUILD_DIR/VideoTimeline.exe"; do
        if [ -f "$bin_path" ]; then
            cp "$bin_path" "$BUILD_DIR/"
            chmod +x "$BUILD_DIR/$(basename "$bin_path")" 2>/dev/null || true
            found_client=true
            break
        fi
    done

    for bin_path in "$CMAKE_BUILD_DIR/bin/server" "$CMAKE_BUILD_DIR/bin/server.exe" "$CMAKE_BUILD_DIR/server/server" "$CMAKE_BUILD_DIR/server/server.exe"; do
        if [ -f "$bin_path" ]; then
            cp "$bin_path" "$BUILD_DIR/server/"
            chmod +x "$BUILD_DIR/server/$(basename "$bin_path")" 2>/dev/null || true
            found_server=true
            break
        fi
    done

    if [ "$found_client" = true ]; then
        print_success "Client application built: $BUILD_DIR/VideoTimeline"
    else
        print_error "Client application binary not found in build tree"
    fi

    if [ "$found_server" = true ]; then
        print_success "Server built: $BUILD_DIR/server/server"
    else
        print_error "Server binary not found in build tree"
    fi

    if [ "$found_client" = true ] && [ "$found_server" = true ]; then
        print_success "Build completed successfully!"
    else
        return 1
    fi
}

build_server_only() {
    print_info "Building VideoTimeline Server only..."
    if ! command -v cmake >/dev/null 2>&1; then
        print_error "CMake not found. Please install CMake."
        exit 1
    fi

    mkdir -p "$REPO_ROOT/server/build"
    cd "$REPO_ROOT/server/build"

    cmake .. -DCMAKE_BUILD_TYPE=Release
    cmake --build . -j$(nproc 2>/dev/null || echo 4)

    cd "$REPO_ROOT"
    mkdir -p "$BUILD_DIR/server"
    if [ -f "$REPO_ROOT/server/build/server" ]; then
        cp "$REPO_ROOT/server/build/server" "$BUILD_DIR/server/"
        print_success "Server built successfully: $BUILD_DIR/server/server"
    elif [ -f "$REPO_ROOT/server/build/server.exe" ]; then
        cp "$REPO_ROOT/server/build/server.exe" "$BUILD_DIR/server/"
        print_success "Server built successfully: $BUILD_DIR/server/server.exe"
    else
        print_error "Server build failed"
        exit 1
    fi
}

cmd_build() {
    case "${1:-all}" in
        "all"|"-b"|"")
            build_all
            ;;
        "--server-only"|"server")
            build_server_only
            ;;
        "--client-only"|"client")
            build_all
            ;;
        "--deps"|"-d"|"deps")
            install_build_dependencies
            ;;
        "clean"|"-c")
            clean_build
            ;;
        "check")
            check_build_environment
            ;;
        "help"|"-h"|"--help")
            show_build_usage
            ;;
        *)
            print_error "Unknown build option: $1"
            show_build_usage
            exit 1
            ;;
    esac
}

# ==============================================================================
# SUBCOMMAND: SERVER
# ==============================================================================
show_server_usage() {
    cat << EOF
VideoTimeline Server Management
Usage: $PROG server [OPTIONS]

Options:
  -p, --port PORT     Specify server port (default: $DEFAULT_SERVER_PORT)
  -a, --auto-port     Automatically find an available port
  -b, --build         Force rebuild server before running
  -d, --detach        Run server in background (logs to server.log)
  -s, --status        Check if server is responding
  -k, --kill, stop    Stop all running server instances
  -h, --help          Show this server help

Examples:
  $PROG server                  # Start server in foreground
  $PROG server -p 8080          # Start server on custom port
  $PROG server --detach         # Start server in background
  $PROG server --status         # Check health of server
  $PROG server --kill           # Terminate background server
EOF
}

check_server_binary() {
    if [ ! -f "$SERVER_BINARY" ] && [ ! -f "${SERVER_BINARY}.exe" ]; then
        print_warning "Server binary not found. Building server now..."
        build_server_only
    fi
}

check_port_available() {
    local port=$1
    if command -v nc >/dev/null 2>&1; then
        if nc -z localhost "$port" 2>/dev/null; then
            return 1
        fi
    elif command -v lsof >/dev/null 2>&1; then
        if lsof -i :"$port" >/dev/null 2>&1; then
            return 1
        fi
    fi
    return 0
}

find_free_port() {
    local port=$DEFAULT_SERVER_PORT
    local max=10
    local i=1
    while [ $i -le $max ]; do
        if check_port_available "$port"; then
            echo "$port"
            return 0
        fi
        port=$((port + 1))
        i=$((i + 1))
    done
    print_error "Could not find an available port after $max attempts"
    exit 1
}

check_server_status() {
    local port=${1:-$DEFAULT_SERVER_PORT}
    print_info "Checking server status on port $port..."

    if command -v curl >/dev/null 2>&1; then
        if curl -s --max-time 2 "http://localhost:$port/api/schedule" >/dev/null 2>&1; then
            print_success "Server is running and responding on port $port"
            return 0
        else
            print_warning "Server is not responding on port $port"
            return 1
        fi
    elif command -v wget >/dev/null 2>&1; then
        if wget -q --timeout=2 -O /dev/null "http://localhost:$port/api/schedule" >/dev/null 2>&1; then
            print_success "Server is running and responding on port $port"
            return 0
        else
            print_warning "Server is not responding on port $port"
            return 1
        fi
    else
        if pgrep -f "server" >/dev/null 2>&1; then
            print_success "Server process is running"
            return 0
        else
            print_warning "No server process found"
            return 1
        fi
    fi
}

kill_server_processes() {
    print_info "Stopping VideoTimeline server processes..."
    local pids=$(pgrep -f "$SERVER_BINARY" 2>/dev/null || pgrep -f "build/server/server" 2>/dev/null || true)
    if [ -z "$pids" ]; then
        print_info "No running server processes found"
        return 0
    fi

    print_warning "Stopping PIDs: $pids"
    kill $pids 2>/dev/null || true
    sleep 1
    pids=$(pgrep -f "$SERVER_BINARY" 2>/dev/null || pgrep -f "build/server/server" 2>/dev/null || true)
    if [ -z "$pids" ]; then
        print_success "All server processes stopped"
    else
        print_warning "Forcing stop for remaining PIDs: $pids"
        kill -9 $pids 2>/dev/null || true
        print_success "Processes terminated"
    fi
}

cmd_server() {
    local port=""
    local auto_port=false
    local force_build=false
    local detach=false

    while [[ $# -gt 0 ]]; do
        case "$1" in
            -p|--port)
                port="$2"
                shift 2
                ;;
            -a|--auto-port)
                auto_port=true
                shift
                ;;
            -b|--build)
                force_build=true
                shift
                ;;
            -d|--detach)
                detach=true
                shift
                ;;
            -s|--status|status)
                check_server_status "$port"
                return $?
                ;;
            -k|--kill|kill|stop)
                kill_server_processes
                return $?
                ;;
            -h|--help)
                show_server_usage
                return 0
                ;;
            start)
                shift
                ;;
            *)
                print_error "Unknown server option: $1"
                show_server_usage
                exit 1
                ;;
        esac
    done

    if [ "$force_build" = true ]; then
        build_server_only
    fi

    check_server_binary

    if [ "$auto_port" = true ]; then
        port=$(find_free_port)
        print_info "Using available port: $port"
    elif [ -z "$port" ]; then
        port=$DEFAULT_SERVER_PORT
    fi

    if ! check_port_available "$port"; then
        if [ "$auto_port" = false ]; then
            print_error "Port $port is in use. Use -a/--auto-port or stop the running server."
            exit 1
        fi
    fi

    local bin_to_exec="$SERVER_BINARY"
    [ -f "${SERVER_BINARY}.exe" ] && bin_to_exec="${SERVER_BINARY}.exe"

    if [ "$detach" = true ]; then
        print_info "Starting VideoTimeline Server in background on port $port..."
        nohup "$bin_to_exec" "$port" > "$REPO_ROOT/server.log" 2>&1 &
        local pid=$!
        print_success "Server running with PID $pid on port $port"
        print_info "Log file: $REPO_ROOT/server.log"
    else
        print_info "Starting VideoTimeline Server on port $port (Press Ctrl+C to stop)..."
        exec "$bin_to_exec" "$port"
    fi
}

# ==============================================================================
# SUBCOMMAND: RUN (CLIENT)
# ==============================================================================
show_run_usage() {
    cat << EOF
VideoTimeline Client Runner
Usage: $PROG [run] [RUNNER_OPTIONS] [APP_OPTIONS]

Runner & Hardware Acceleration Options:
  -s, --server HOST    Server hostname (default: $DEFAULT_SERVER_HOST)
  -p, --port PORT      Server port (default: $DEFAULT_SERVER_PORT)
  -a, --auto-server    Start server automatically if not running
  -b, --build          Force rebuild client & server before running
  --hwaccel            Enable GPU hardware acceleration
  --nvidia             Force NVIDIA GPU (Prime offload + VDPAU + ffmpeg backend)
  --amd                Force AMD GPU (radeonsi + ffmpeg backend)
  --gstreamer          Force GStreamer backend with VAAPI
  --debug-media        Enable Qt multimedia verbose logging
  -h, --help           Show this run help

Application Options (forwarded to VideoTimeline):
  --auto               Auto server discovery and start video playback
  --network RANGE      Network range for scanning (e.g. 192.168.1)
  --dpi VALUE          Force specific DPI scaling (e.g. 144, 192)
  --test-time TIME     Simulate test time (e.g. '14:30')
  --log-level LEVEL    error, warn, info, debug
  --log-file           Enable rotating log file
  --cache-size SIZE    Disk cache size in GB (2-8)
  --date DD:MM:YYYY    Date for special event
  --time HH:MM         Time for special event
  --image URL          Image or video URL for special event
  --title TEXT         Title for special event
  --duration SECS      Duration in seconds (default: 180)
  --version            Show version
EOF
}

check_client_binary() {
    if [ ! -f "$APP_BINARY" ] && [ ! -f "${APP_BINARY}.exe" ]; then
        print_warning "Application binary not found at $APP_BINARY. Building project..."
        build_all
    fi
}

cmd_run() {
    local server_host="$DEFAULT_SERVER_HOST"
    local server_port="$DEFAULT_SERVER_PORT"
    local auto_server=false
    local force_build=false
    local use_hwaccel=false
    local use_nvidia=false
    local use_amd=false
    local use_gstreamer=false
    local debug_media=false
    local app_args=()

    while [[ $# -gt 0 ]]; do
        case "$1" in
            -s|--server)
                server_host="$2"
                shift 2
                ;;
            -p|--port)
                server_port="$2"
                shift 2
                ;;
            -a|--auto-server)
                auto_server=true
                shift
                ;;
            -b|--build)
                force_build=true
                shift
                ;;
            --hwaccel)
                use_hwaccel=true
                shift
                ;;
            --nvidia)
                use_nvidia=true
                use_hwaccel=true
                shift
                ;;
            --amd)
                use_amd=true
                use_hwaccel=true
                shift
                ;;
            --gstreamer)
                use_gstreamer=true
                use_hwaccel=true
                shift
                ;;
            --debug-media)
                debug_media=true
                shift
                ;;
            -h|--help)
                show_run_usage
                return 0
                ;;
            *)
                app_args+=("$1")
                shift
                ;;
        esac
    done

    # Force rebuild if requested
    if [ "$force_build" = true ]; then
        build_all
    fi

    # Verify binary exists
    check_client_binary

    # Hardware acceleration environment setup
    if [ "$use_hwaccel" = true ]; then
        print_info "Configuring hardware acceleration..."
        if [ "$use_nvidia" = true ]; then
            print_info "Hardware Acceleration: NVIDIA GPU selected"
            export __NV_PRIME_RENDER_OFFLOAD=1
            export __GLX_VENDOR_LIBRARY_NAME=nvidia
            export VDPAU_DRIVER=nvidia
            export QT_MEDIA_BACKEND=ffmpeg
        elif [ "$use_amd" = true ]; then
            print_info "Hardware Acceleration: AMD GPU selected"
            export DRI_PRIME=1
            export LIBVA_DRIVER_NAME=radeonsi
            export QT_MEDIA_BACKEND=ffmpeg
        elif [ "$use_gstreamer" = true ]; then
            print_info "Hardware Acceleration: GStreamer + VAAPI selected"
            export LIBVA_DRIVER_NAME=radeonsi
            export QT_MEDIA_BACKEND=gstreamer
            export GST_VAAPI_ALL_DRIVERS=1
        else
            # Automatic hardware acceleration configuration
            print_info "Hardware Acceleration: Auto-detecting GPU..."
            if command -v nvidia-smi >/dev/null 2>&1; then
                export __NV_PRIME_RENDER_OFFLOAD=1
                export __GLX_VENDOR_LIBRARY_NAME=nvidia
                export VDPAU_DRIVER=nvidia
                export QT_MEDIA_BACKEND=ffmpeg
            else
                export LIBVA_DRIVER_NAME=radeonsi
                export QT_MEDIA_BACKEND=gstreamer
                export GST_VAAPI_ALL_DRIVERS=1
            fi
        fi
    fi

    if [ "$debug_media" = true ]; then
        export QT_LOGGING_RULES="qt.multimedia*=true"
        print_info "Qt multimedia logging rule enabled"
    fi

    # Auto server management
    if [ "$auto_server" = true ]; then
        if ! check_server_status "$server_port" >/dev/null 2>&1; then
            print_info "Auto-starting VideoTimeline server on port $server_port..."
            cmd_server -p "$server_port" --detach
            sleep 1
        fi
    fi

    local bin_to_run="$APP_BINARY"
    [ -f "${APP_BINARY}.exe" ] && bin_to_run="${APP_BINARY}.exe"

    print_info "Launching VideoTimeline client..."
    if [ ${#app_args[@]} -gt 0 ]; then
        print_info "Options: ${app_args[*]}"
        exec "$bin_to_run" "${app_args[@]}"
    else
        exec "$bin_to_run"
    fi
}

# ==============================================================================
# SUBCOMMAND: FONTS
# ==============================================================================
show_fonts_usage() {
    cat << EOF
SF Pro Display Font Installer (All Linux Distributions)
Usage: $PROG fonts [OPTIONS]

Options:
  --system            Install system-wide (requires sudo/root) [default if root]
  --user              Install for current user only (~/.local/share/fonts)
  -h, --help          Show this fonts help

Supported Distros:
  Debian, Ubuntu, Raspberry Pi OS, Fedora, RHEL, CentOS, Arch, openSUSE, Alpine, Void
EOF
}

cmd_fonts() {
    local install_mode="auto"

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --system) install_mode="system"; shift ;;
            --user) install_mode="user"; shift ;;
            -h|--help) show_fonts_usage; return 0 ;;
            *) print_error "Unknown fonts option: $1"; show_fonts_usage; exit 1 ;;
        esac
    done

    if [ ! -d "$FONT_SRC_DIR" ]; then
        print_error "Font directory not found at $FONT_SRC_DIR"
        exit 1
    fi

    # Determine destination directory
    local dest_dir=""
    if [ "$install_mode" = "user" ]; then
        dest_dir="$HOME/.local/share/fonts/SFProDisplay"
    elif [ "$install_mode" = "system" ]; then
        dest_dir="/usr/local/share/fonts/opentype/SFProDisplay"
    else
        if [ "$(id -u)" -eq 0 ]; then
            dest_dir="/usr/local/share/fonts/opentype/SFProDisplay"
        else
            dest_dir="$HOME/.local/share/fonts/SFProDisplay"
        fi
    fi

    print_info "Installing fonts into: $dest_dir"

    # Ensure fontconfig exists
    if ! command -v fc-cache >/dev/null 2>&1; then
        print_warning "fc-cache (fontconfig) not found. Installing..."
        detect_linux_distro
        local sudo_cmd=$(get_sudo_cmd)
        if command -v apt-get >/dev/null 2>&1; then
            $sudo_cmd apt-get update && $sudo_cmd apt-get install -y fontconfig
        elif command -v dnf >/dev/null 2>&1; then
            $sudo_cmd dnf install -y fontconfig
        elif command -v pacman >/dev/null 2>&1; then
            $sudo_cmd pacman -Sy --noconfirm fontconfig
        elif command -v zypper >/dev/null 2>&1; then
            $sudo_cmd zypper --non-interactive install -y fontconfig
        elif command -v apk >/dev/null 2>&1; then
            $sudo_cmd apk add --no-cache fontconfig
        elif command -v xbps-install >/dev/null 2>&1; then
            $sudo_cmd xbps-install -Sy fontconfig
        fi
    fi

    mkdir -p "$dest_dir"
    cp -v "$FONT_SRC_DIR"/*.otf "$FONT_SRC_DIR"/*.ttf "$dest_dir/" 2>/dev/null || true
    chmod 644 "$dest_dir"/* 2>/dev/null || true
    chmod 755 "$dest_dir" 2>/dev/null || true

    print_info "Updating font cache..."
    if command -v fc-cache >/dev/null 2>&1; then
        fc-cache -fv "$dest_dir"
        print_success "Font cache refreshed!"
    fi

    print_success "SF Pro Display fonts installed successfully!"
}

# ==============================================================================
# SUBCOMMAND: INSTALL (UNIVERSAL LINUX KIOSK & SYSTEMD)
# ==============================================================================
show_install_usage() {
    cat << EOF
VideoTimeline Universal Linux Kiosk & System Installer
Usage: $PROG install [OPTIONS]

Options:
  --all               Full install (deps, fonts, binaries, kiosk, systemd) [default]
  --deps-only         Install system packages and libraries only
  --kiosk-only        Configure Openbox autostart & LightDM autologin only
  --service-only      Register and enable systemd server service only
  --user USERNAME     Target username for autologin (default: \$SUDO_USER or current)
  -h, --help          Show this install help

Supported Distros:
  Debian / Ubuntu / Raspberry Pi OS, Fedora / RHEL / CentOS / Rocky,
  Arch Linux / Manjaro, openSUSE / SLES, Alpine Linux
EOF
}

cmd_install() {
    for arg in "$@"; do
        if [ "$arg" = "-h" ] || [ "$arg" = "--help" ] || [ "$arg" = "help" ]; then
            show_install_usage
            return 0
        fi
    done

    if [ "$(id -u)" -ne 0 ]; then
        print_error "Installation requires root privileges. Please run with sudo: sudo $PROG install"
        exit 1
    fi

    local mode="all"
    local target_user="${SUDO_USER:-$USER}"

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --all) mode="all"; shift ;;
            --deps-only) mode="deps"; shift ;;
            --kiosk-only) mode="kiosk"; shift ;;
            --service-only) mode="service"; shift ;;
            --user) target_user="$2"; shift 2 ;;
            -h|--help) show_install_usage; return 0 ;;
            *) print_error "Unknown install option: $1"; show_install_usage; exit 1 ;;
        esac
    done

    local target_home=$(eval echo "~$target_user")
    local app_dir="$target_home/VideoTimeline"

    print_info "Target installation user: $target_user"
    print_info "Home directory: $target_home"
    print_info "Application directory: $app_dir"

    detect_linux_distro

    # Install packages
    if [ "$mode" = "all" ] || [ "$mode" = "deps" ]; then
        print_info "Installing distro package dependencies ($DISTRO_ID)..."
        if command -v apt-get >/dev/null 2>&1; then
            apt-get update
            apt-get install -y \
                qt6-base-dev qt6-multimedia-dev libqt6multimedia6 libqt6multimediawidgets6 libqt6network6 \
                xorg openbox lightdm unclutter \
                gstreamer1.0-tools gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav \
                ffmpeg fontconfig build-essential cmake
            apt-get install -y gstreamer1.0-omx 2>/dev/null || true
        elif command -v dnf >/dev/null 2>&1; then
            dnf install -y \
                qt6-qtbase-devel qt6-qtmultimedia-devel qt6-qtbase-gui \
                xorg-x11-server-Xorg openbox lightdm unclutter \
                gstreamer1 gstreamer1-plugins-base gstreamer1-plugins-good gstreamer1-plugins-bad-free gstreamer1-plugins-ugly-free \
                ffmpeg fontconfig gcc-c++ make cmake
        elif command -v pacman >/dev/null 2>&1; then
            pacman -Sy --noconfirm --needed \
                qt6-base qt6-multimedia xorg-server openbox lightdm unclutter \
                gstreamer gst-plugins-base gst-plugins-good gst-plugins-bad gst-plugins-ugly gst-libav \
                ffmpeg fontconfig base-devel cmake
        elif command -v zypper >/dev/null 2>&1; then
            zypper --non-interactive install -y \
                qt6-base-devel qt6-multimedia-devel xorg-x11-server openbox lightdm \
                gstreamer gstreamer-plugins-base gstreamer-plugins-good gstreamer-plugins-bad gstreamer-plugins-ugly \
                ffmpeg fontconfig gcc-c++ cmake
        elif command -v apk >/dev/null 2>&1; then
            apk add --no-cache \
                qt6-qtbase-dev qt6-qtmultimedia-dev xorg-server openbox lightdm \
                gstreamer gst-plugins-base gst-plugins-good ffmpeg fontconfig build-base cmake
        fi
        print_success "Distro packages installed successfully"
    fi

    # Install fonts
    if [ "$mode" = "all" ]; then
        print_info "Installing fonts system-wide..."
        cmd_fonts --system
    fi

    # Copy / build binaries
    if [ "$mode" = "all" ]; then
        print_info "Setting up application files in $app_dir..."
        mkdir -p "$app_dir" "$app_dir/data/media"

        check_client_binary
        check_server_binary

        cp "$BUILD_DIR/VideoTimeline" "$app_dir/"
        chmod +x "$app_dir/VideoTimeline"
        cp "$BUILD_DIR/server/server" "$app_dir/"
        chmod +x "$app_dir/server"

        if [ -d "$REPO_ROOT/data" ]; then
            cp -r "$REPO_ROOT/data/"* "$app_dir/data/" 2>/dev/null || true
        fi
        chown -R "$target_user:$target_user" "$app_dir"
        print_success "Application files deployed to $app_dir"
    fi

    # Kiosk configuration
    if [ "$mode" = "all" ] || [ "$mode" = "kiosk" ]; then
        print_info "Configuring LightDM autologin..."
        mkdir -p /etc/lightdm/lightdm.conf.d/
        cat > /etc/lightdm/lightdm.conf.d/50-videotimeline.conf << EOF
[SeatDefaults]
autologin-user=$target_user
autologin-user-timeout=0
user-session=openbox
EOF

        print_info "Configuring Openbox fullscreen kiosk autostart..."
        mkdir -p "$target_home/.config/openbox"
        cat > "$target_home/.config/openbox/autostart" << EOF
# Hide cursor after 1 second of inactivity
command -v unclutter >/dev/null 2>&1 && unclutter -idle 1 &

# Disable screen saver and DPMS power saving
xset s off 2>/dev/null || true
xset -dpms 2>/dev/null || true
xset s noblank 2>/dev/null || true

# Wait for system desktop to settle
sleep 3

# Start VideoTimeline client
$app_dir/VideoTimeline &
EOF
        chmod +x "$target_home/.config/openbox/autostart"
        chown -R "$target_user:$target_user" "$target_home/.config"
        print_success "Kiosk autostart configured"
    fi

    # Systemd service
    if [ "$mode" = "all" ] || [ "$mode" = "service" ]; then
        if command -v systemctl >/dev/null 2>&1; then
            print_info "Registering videotimeline-server.service systemd unit..."
            cat > /etc/systemd/system/videotimeline-server.service << EOF
[Unit]
Description=VideoTimeline Media Display Server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=$target_user
Group=$target_user
WorkingDirectory=$app_dir
ExecStart=$app_dir/server 3232
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF
            systemctl daemon-reload
            systemctl enable videotimeline-server.service
            print_success "videotimeline-server.service registered and enabled!"
        fi
    fi

    print_success "================================================="
    print_success "VideoTimeline installation completed for $target_user!"
    print_success "================================================="
    echo "To test or control services:"
    echo "  sudo systemctl start videotimeline-server"
    echo "  sudo systemctl status videotimeline-server"
    echo "  Reboot to launch the fullscreen kiosk automatically: sudo reboot"
}

# ==============================================================================
# SUBCOMMAND: MEDIA (RE-ENCODE, PLAYLIST, CONVERT)
# ==============================================================================
show_media_usage() {
    cat << EOF
VideoTimeline Media Utilities
Usage: $PROG media [reencode|playlist|convert-special] [OPTIONS]

Subactions:
  reencode <dir> [OPTIONS]
      Re-encode AV1/VP9 videos to H.264 (for hardware decode compatibility)
      Options:
        --crf VALUE     CRF quality (18-28, default: 23)
        --preset NAME   Preset (ultrafast|fast|medium|slow, default: medium)
        --hwaccel       Use NVIDIA h264_nvenc encoder
        --backup        Create .bak backups of original files

  playlist <folder> <date> <time> <title> [output_file]
      Generate special playlist JSON from a media folder
      Example:
        $PROG media playlist data/media/special 2025-11-10 09:05 "Ataturk Commemoration"

  convert-special
      Batch re-encode remaining special event videos in data/media/special
EOF
}

cmd_media_reencode() {
    if [ $# -lt 1 ]; then
        print_error "Input directory required for reencode"
        show_media_usage
        exit 1
    fi

    local input_dir=""
    local crf=23
    local preset="medium"
    local use_hwaccel=false
    local create_backup=false

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --crf) crf="$2"; shift 2 ;;
            --preset) preset="$2"; shift 2 ;;
            --hwaccel) use_hwaccel=true; shift ;;
            --backup) create_backup=true; shift ;;
            *)
                if [ -z "$input_dir" ]; then
                    input_dir="$1"
                else
                    print_error "Unknown argument: $1"
                    exit 1
                fi
                shift
                ;;
        esac
    done

    if [ ! -d "$input_dir" ]; then
        print_error "Directory does not exist: $input_dir"
        exit 1
    fi

    local encoder="libx264"
    local enc_opts="-preset $preset -crf $crf"

    if [ "$use_hwaccel" = true ]; then
        if ffmpeg -encoders 2>/dev/null | grep -q "h264_nvenc"; then
            encoder="h264_nvenc"
            enc_opts="-preset $preset -cq $crf"
            print_info "Using NVIDIA NVENC hardware encoder ($encoder)"
        fi
    fi

    print_info "Scanning for video files in $input_dir..."
    find "$input_dir" -type f \( -iname "*.mp4" -o -iname "*.mkv" -o -iname "*.webm" \) | while read -r video; do
        local codec=$(ffprobe -v error -select_streams v:0 -show_entries stream=codec_name -of default=noprint_wrappers=1:nokey=1 "$video" 2>/dev/null || true)
        if [ "$codec" = "av1" ] || [ "$codec" = "vp9" ]; then
            print_info "Converting $codec video: $video"
            local tmp="${video}.tmp.$$.mp4"
            if ffmpeg -y -hide_banner -loglevel error -stats -i "$video" -c:v "$encoder" $enc_opts -c:a aac -b:a 192k -movflags +faststart "$tmp"; then
                [ "$create_backup" = true ] && cp "$video" "${video}.bak"
                mv "$tmp" "$video"
                print_success "Converted: $video"
            else
                print_error "Failed to convert: $video"
                rm -f "$tmp"
            fi
        fi
    done
}

cmd_media_convert_special() {
    local target_dir="$REPO_ROOT/data/media/special"
    if [ ! -d "$target_dir" ]; then
        print_error "Directory not found: $target_dir"
        exit 1
    fi
    cmd_media_reencode "$target_dir" --hwaccel --backup
}

cmd_media_playlist() {
    if [ $# -lt 4 ]; then
        print_error "Missing required arguments: <folder> <date> <time> <title> [output_file]"
        show_media_usage
        exit 1
    fi

    local media_folder="$1"
    local event_date="$2"
    local event_time="$3"
    local event_title="$4"
    local output_file="${5:-$REPO_ROOT/data/special_playlist.json}"

    if [ ! -d "$media_folder" ]; then
        print_error "Folder '$media_folder' does not exist"
        exit 1
    fi

    print_info "Generating special playlist JSON from $media_folder..."
    local items=()

    find "$media_folder" -maxdepth 1 -type f \( -iname "*.jpg" -o -iname "*.png" -o -iname "*.mp4" -o -iname "*.mov" \) | sort | while read -r file; do
        local fname=$(basename "$file")
        local ext="${fname##*.}"
        local ext_lower=$(echo "$ext" | tr '[:upper:]' '[:lower:]')
        local type="video"
        local dur=300000
        local muted=false

        if [[ "$ext_lower" =~ ^(jpg|jpeg|png|webp|bmp)$ ]]; then
            type="image"
            dur=180000
            muted=true
        fi

        echo "    {\"type\": \"$type\", \"url\": \"$file\", \"duration\": $dur, \"muted\": $muted}"
    done > "$REPO_ROOT/data/.tmp_playlist_items"

    cat << EOF > "$output_file"
{
  "event": {
    "title": "$event_title",
    "date": "$event_date",
    "time": "$event_time"
  },
  "items": [
$(paste -sd, "$REPO_ROOT/data/.tmp_playlist_items")
  ]
}
EOF
    rm -f "$REPO_ROOT/data/.tmp_playlist_items"
    print_success "Special playlist generated at: $output_file"
}

cmd_media() {
    local action="${1:-help}"
    shift || true

    case "$action" in
        reencode)
            cmd_media_reencode "$@"
            ;;
        playlist)
            cmd_media_playlist "$@"
            ;;
        convert-special)
            cmd_media_convert_special
            ;;
        help|-h|--help)
            show_media_usage
            ;;
        *)
            print_error "Unknown media action: $action"
            show_media_usage
            exit 1
            ;;
    esac
}

# ==============================================================================
# MAIN ROUTER
# ==============================================================================
main() {
    local command="${1:-run}"

    case "$command" in
        run)
            shift
            cmd_run "$@"
            ;;
        server)
            shift
            cmd_server "$@"
            ;;
        build)
            shift
            cmd_build "$@"
            ;;
        install)
            shift
            cmd_install "$@"
            ;;
        fonts)
            shift
            cmd_fonts "$@"
            ;;
        media)
            shift
            cmd_media "$@"
            ;;
        help|-h|--help)
            show_usage
            exit 0
            ;;
        -*)
            # Options passed without explicit command default to 'run'
            cmd_run "$@"
            ;;
        *)
            # Check if user passed app option directly or unknown command
            if [[ "$command" =~ ^-- ]]; then
                cmd_run "$@"
            else
                print_error "Unknown command: $command"
                show_usage
                exit 1
            fi
            ;;
    esac
}

main "$@"
