#!/bin/bash
# Universal VideoTimeline Linux Installation & Kiosk Setup Script
# Supports: Debian, Ubuntu, Raspberry Pi OS, Fedora, Arch, Manjaro, openSUSE, Alpine, RHEL/Rocky

set -e

# Styling & colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

echo "=========================================================="
echo "VideoTimeline Universal Linux Installation & Kiosk Setup"
echo "=========================================================="

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    print_error "Please run this script as root (use sudo ./scripts/install_linux.sh)"
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Identify target user (the user who invoked sudo)
TARGET_USER="${SUDO_USER:-$USER}"
if [ "$TARGET_USER" = "root" ]; then
    # If root invoked directly, try to find the first interactive desktop user
    FIRST_USER=$(awk -F: '$3 >= 1000 && $3 < 60000 {print $1}' /etc/passwd | head -n1)
    if [ -n "$FIRST_USER" ]; then
        TARGET_USER="$FIRST_USER"
    fi
fi

USER_HOME=$(eval echo "~$TARGET_USER")
APP_DIR="$USER_HOME/VideoTimeline"

print_info "Installing for user: $TARGET_USER"
print_info "Home directory:      $USER_HOME"
print_info "Target install dir:  $APP_DIR"

# Detect distribution
detect_distro() {
    if [ -f /etc/os-release ]; then
        . /etc/os-release
        DISTRO_ID="${ID:-unknown}"
        DISTRO_LIKE="${ID_LIKE:-}"
        DISTRO_NAME="${NAME:-Linux}"
    else
        DISTRO_ID="unknown"
        DISTRO_LIKE=""
        DISTRO_NAME="Linux"
    fi
}

detect_distro
print_info "Detected OS: $DISTRO_NAME ($DISTRO_ID)"

# Install dependencies based on distribution package manager
install_dependencies() {
    print_info "Installing dependencies for $DISTRO_NAME..."

    case "$DISTRO_ID" in
        debian|ubuntu|raspbian|linuxmint|pop|elementary)
            apt-get update
            apt-get install -y \
                qt6-base-dev \
                qt6-multimedia-dev \
                libqt6multimedia6 \
                libqt6multimediawidgets6 \
                libqt6network6 \
                xorg \
                openbox \
                lightdm \
                unclutter \
                ffmpeg \
                libgstreamer1.0-0 \
                libgstreamer-plugins-base1.0-0 \
                gstreamer1.0-tools \
                gstreamer1.0-plugins-base \
                gstreamer1.0-plugins-good \
                gstreamer1.0-plugins-bad \
                gstreamer1.0-plugins-ugly \
                gstreamer1.0-libav \
                gstreamer1.0-alsa \
                gstreamer1.0-gl \
                fontconfig \
                cmake \
                build-essential
            # Try legacy OMX for older Raspberry Pi OS if available
            apt-get install -y gstreamer1.0-omx 2>/dev/null || true
            ;;

        fedora|rhel|centos|rocky|almalinux)
            dnf install -y \
                qt6-qtbase-devel \
                qt6-qtmultimedia-devel \
                xorg-x11-server-Xorg \
                xorg-x11-xinit \
                openbox \
                lightdm \
                unclutter \
                ffmpeg \
                gstreamer1 \
                gstreamer1-plugins-base \
                gstreamer1-plugins-good \
                gstreamer1-plugins-bad-free \
                gstreamer1-plugins-ugly-free \
                fontconfig \
                cmake \
                gcc-c++ \
                make
            ;;

        arch|manjaro|endeavouros)
            pacman -Syu --needed --noconfirm \
                qt6-base \
                qt6-multimedia \
                xorg-server \
                xorg-xinit \
                openbox \
                lightdm \
                unclutter \
                ffmpeg \
                gstreamer \
                gst-plugins-base \
                gst-plugins-good \
                gst-plugins-bad \
                gst-plugins-ugly \
                gst-libav \
                fontconfig \
                cmake \
                base-devel
            ;;

        opensuse*|suse)
            zypper refresh
            zypper install -y \
                libQt6Core-devel \
                libQt6Widgets-devel \
                libQt6Network-devel \
                libQt6Multimedia-devel \
                libQt6MultimediaWidgets-devel \
                xorg-x11-server \
                openbox \
                lightdm \
                unclutter \
                ffmpeg \
                gstreamer \
                gstreamer-plugins-base \
                gstreamer-plugins-good \
                gstreamer-plugins-bad \
                gstreamer-plugins-ugly \
                fontconfig \
                cmake \
                gcc-c++
            ;;

        alpine)
            apk update
            apk add \
                qt6-qtbase-dev \
                qt6-qtmultimedia-dev \
                xorg-server \
                openbox \
                lightdm \
                unclutter \
                ffmpeg \
                gst-plugins-base \
                gst-plugins-good \
                gst-plugins-bad \
                gst-plugins-ugly \
                fontconfig \
                cmake \
                make \
                g++
            ;;

        *)
            print_warning "Distribution '$DISTRO_ID' not directly recognized."
            print_info "Attempting installation using standard package managers..."
            if command -v apt-get >/dev/null 2>&1; then
                apt-get update && apt-get install -y qt6-base-dev qt6-multimedia-dev xorg openbox lightdm unclutter ffmpeg fontconfig cmake build-essential
            elif command -v dnf >/dev/null 2>&1; then
                dnf install -y qt6-qtbase-devel qt6-qtmultimedia-devel openbox lightdm unclutter ffmpeg fontconfig cmake gcc-c++
            elif command -v pacman >/dev/null 2>&1; then
                pacman -S --needed --noconfirm qt6-base qt6-multimedia openbox lightdm unclutter ffmpeg fontconfig cmake base-devel
            else
                print_warning "Please ensure Qt6, Openbox, LightDM, FFmpeg, and fontconfig are installed."
            fi
            ;;
    esac
}

install_dependencies

# Install fonts
install_fonts() {
    print_info "Installing fonts system-wide..."
    if [ -f "$REPO_ROOT/SFProDisplayFonts/install_fonts.sh" ]; then
        chmod +x "$REPO_ROOT/SFProDisplayFonts/install_fonts.sh"
        "$REPO_ROOT/SFProDisplayFonts/install_fonts.sh"
    fi
}

install_fonts

# Ensure application binaries are built
ensure_binaries() {
    print_info "Checking application binaries..."
    
    CLIENT_BIN=""
    SERVER_BIN=""

    # Check potential client binary paths
    for p in "$REPO_ROOT/build/bin/VideoTimeline" "$REPO_ROOT/build/VideoTimeline" "$REPO_ROOT/build/cmake/bin/VideoTimeline"; do
        if [ -f "$p" ]; then
            CLIENT_BIN="$p"
            break
        fi
    done

    # Check potential server binary paths
    for p in "$REPO_ROOT/build/bin/server" "$REPO_ROOT/build/server/server" "$REPO_ROOT/build/server" "$REPO_ROOT/server/build/server" "$REPO_ROOT/build/cmake/bin/server"; do
        if [ -f "$p" ]; then
            SERVER_BIN="$p"
            break
        fi
    done

    # If binaries are missing, run build script as the target user
    if [ -z "$CLIENT_BIN" ] || [ -z "$SERVER_BIN" ]; then
        print_info "Binaries not found. Building now via ./scripts/build.sh..."
        if [ -f "$REPO_ROOT/scripts/build.sh" ]; then
            chmod +x "$REPO_ROOT/scripts/build.sh"
            su - "$TARGET_USER" -c "cd '$REPO_ROOT' && ./scripts/build.sh build"
            
            # Re-check after build
            for p in "$REPO_ROOT/build/bin/VideoTimeline" "$REPO_ROOT/build/VideoTimeline"; do
                if [ -f "$p" ]; then CLIENT_BIN="$p"; break; fi
            done
            for p in "$REPO_ROOT/build/bin/server" "$REPO_ROOT/build/server/server" "$REPO_ROOT/build/server"; do
                if [ -f "$p" ]; then SERVER_BIN="$p"; break; fi
            done
        fi
    fi

    if [ -z "$CLIENT_BIN" ] || [ ! -f "$CLIENT_BIN" ]; then
        print_error "Could not find or build VideoTimeline client binary."
        exit 1
    fi

    if [ -z "$SERVER_BIN" ] || [ ! -f "$SERVER_BIN" ]; then
        print_error "Could not find or build VideoTimeline server binary."
        exit 1
    fi

    print_success "Found Client binary: $CLIENT_BIN"
    print_success "Found Server binary: $SERVER_BIN"
}

ensure_binaries

# Set up application directory
setup_app_dir() {
    print_info "Creating application directory: $APP_DIR"
    mkdir -p "$APP_DIR"
    mkdir -p "$APP_DIR/data/media"

    print_info "Copying binaries..."
    cp "$CLIENT_BIN" "$APP_DIR/VideoTimeline"
    cp "$SERVER_BIN" "$APP_DIR/server"
    chmod +x "$APP_DIR/VideoTimeline" "$APP_DIR/server"

    # Copy run script if present
    if [ -f "$REPO_ROOT/run.sh" ]; then
        cp "$REPO_ROOT/run.sh" "$APP_DIR/"
        chmod +x "$APP_DIR/run.sh"
    fi

    # Copy data directory
    if [ -d "$REPO_ROOT/data" ]; then
        print_info "Copying data assets..."
        cp -rn "$REPO_ROOT/data/"* "$APP_DIR/data/" 2>/dev/null || true
    fi

    chown -R "$TARGET_USER:$TARGET_USER" "$APP_DIR"
}

setup_app_dir

# Configure Auto-Login for Display Session (LightDM)
setup_autologin() {
    if [ -d /etc/lightdm ]; then
        print_info "Configuring LightDM auto-login for $TARGET_USER..."
        mkdir -p /etc/lightdm/lightdm.conf.d/
        cat > /etc/lightdm/lightdm.conf.d/50-videotimeline.conf << EOF
[SeatDefaults]
autologin-user=$TARGET_USER
autologin-user-timeout=0
user-session=openbox
EOF
    fi
}

setup_autologin

# Configure Openbox Autostart for Kiosk Fullscreen
setup_openbox_kiosk() {
    print_info "Configuring Openbox kiosk autostart for $TARGET_USER..."
    OPENBOX_DIR="$USER_HOME/.config/openbox"
    mkdir -p "$OPENBOX_DIR"

    cat > "$OPENBOX_DIR/autostart" << EOF
# Hide cursor after 1 second of inactivity
command -v unclutter >/dev/null 2>&1 && unclutter -idle 1 &

# Disable screen blanking and power management
command -v xset >/dev/null 2>&1 && {
    xset s off
    xset -dpms
    xset s noblank
}

# Wait for system and network services
sleep 3

# Start VideoTimeline in fullscreen kiosk mode
$APP_DIR/VideoTimeline --auto &
EOF

    chmod +x "$OPENBOX_DIR/autostart"
    chown -R "$TARGET_USER:$TARGET_USER" "$USER_HOME/.config"
}

setup_openbox_kiosk

# Configure Systemd Services
setup_systemd() {
    if command -v systemctl >/dev/null 2>&1; then
        print_info "Configuring systemd service for VideoTimeline Server..."

        cat > /etc/systemd/system/videotimeline-server.service << EOF
[Unit]
Description=VideoTimeline Server
After=network-online.target
Wants=network-online.target
StartLimitIntervalSec=60
StartLimitBurst=5

[Service]
Type=simple
User=$TARGET_USER
Group=$TARGET_USER
WorkingDirectory=$APP_DIR
ExecStart=$APP_DIR/server
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

        systemctl daemon-reload
        systemctl enable videotimeline-server.service
        print_success "Enabled videotimeline-server.service"

        # Ask or automatically start server service
        systemctl restart videotimeline-server.service || true
    fi
}

setup_systemd

echo ""
print_success "=========================================================="
print_success "Installation & Kiosk Setup Complete!"
print_success "=========================================================="
echo ""
echo "Installation Details:"
echo "  Target User:     $TARGET_USER"
echo "  Install Path:    $APP_DIR"
echo "  Server Port:     3232"
echo ""
echo "To manage the server service:"
echo "  sudo systemctl status videotimeline-server"
echo "  sudo systemctl restart videotimeline-server"
echo ""
echo "To run the client manually:"
echo "  cd $APP_DIR && ./VideoTimeline"
echo ""
echo "Reboot to launch kiosk mode automatically on desktop start:"
echo "  sudo reboot"
echo ""
