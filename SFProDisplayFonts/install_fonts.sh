#!/bin/bash
# Universal SF Pro Display Font Installation Script for all Linux Distributions
# Supports: Ubuntu, Debian, Fedora, Arch, openSUSE, Alpine, RHEL, CentOS, Rocky, Void, etc.

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }

echo "================================================="
echo "SF Pro Display Universal Font Installer for Linux"
echo "================================================="

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

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

# Ensure fontconfig (fc-cache) is available
ensure_fontconfig() {
    if command -v fc-cache >/dev/null 2>&1; then
        return 0
    fi

    print_warning "'fc-cache' (fontconfig) not found. Attempting to install..."

    if [ "$EUID" -ne 0 ]; then
        print_error "fontconfig is required but not installed. Please run as root or install fontconfig manually:"
        case "$DISTRO_ID" in
            ubuntu|debian|raspbian|linuxmint|pop)
                echo "  sudo apt-get install -y fontconfig" ;;
            fedora|rhel|centos|rocky|almalinux)
                echo "  sudo dnf install -y fontconfig" ;;
            arch|manjaro|endeavouros)
                echo "  sudo pacman -S --needed --noconfirm fontconfig" ;;
            opensuse*|suse)
                echo "  sudo zypper install -y fontconfig" ;;
            alpine)
                echo "  sudo apk add fontconfig" ;;
            *)
                echo "  Install 'fontconfig' using your distribution's package manager" ;;
        esac
        exit 1
    fi

    # Install as root based on available package manager
    if command -v dnf >/dev/null 2>&1; then
        dnf install -y fontconfig
    elif command -v apt-get >/dev/null 2>&1; then
        apt-get update && apt-get install -y fontconfig
    elif command -v pacman >/dev/null 2>&1; then
        pacman -S --needed --noconfirm fontconfig
    elif command -v zypper >/dev/null 2>&1; then
        zypper install -y fontconfig
    elif command -v apk >/dev/null 2>&1; then
        apk add fontconfig
    elif command -v xbps-install >/dev/null 2>&1; then
        xbps-install -Sy fontconfig
    else
        print_error "Could not detect package manager to install fontconfig. Please install it manually."
        exit 1
    fi
}

ensure_fontconfig

# Determine installation directory
if [ "$EUID" -eq 0 ]; then
    print_info "Installing fonts system-wide..."
    if [ -d "/usr/local/share/fonts" ]; then
        FONT_DIR="/usr/local/share/fonts/sf-pro-display"
    else
        FONT_DIR="/usr/share/fonts/truetype/sf-pro-display"
    fi
    INSTALL_SYSTEM=true
else
    print_info "Installing fonts for current user ($USER)..."
    XDG_DATA_HOME="${XDG_DATA_HOME:-$HOME/.local/share}"
    FONT_DIR="$XDG_DATA_HOME/fonts/sf-pro-display"
    INSTALL_SYSTEM=false
fi

print_info "Target directory: $FONT_DIR"
mkdir -p "$FONT_DIR"

# Copy fonts
TTF_COUNT=0
if compgen -G "$SCRIPT_DIR/*.ttf" >/dev/null 2>&1; then
    cp "$SCRIPT_DIR"/*.ttf "$FONT_DIR/"
    TTF_COUNT=$(find "$SCRIPT_DIR" -maxdepth 1 -name "*.ttf" | wc -l)
    print_info "Copied $TTF_COUNT .ttf files"
fi

OTF_COUNT=0
if compgen -G "$SCRIPT_DIR/*.otf" >/dev/null 2>&1; then
    cp "$SCRIPT_DIR"/*.otf "$FONT_DIR/"
    OTF_COUNT=$(find "$SCRIPT_DIR" -maxdepth 1 -name "*.otf" | wc -l)
    print_info "Copied $OTF_COUNT .otf files"
fi

TOTAL_COUNT=$((TTF_COUNT + OTF_COUNT))
if [ $TOTAL_COUNT -eq 0 ]; then
    print_error "No font files (.ttf or .otf) found in $SCRIPT_DIR"
    exit 1
fi

# Set proper permissions
chmod 755 "$FONT_DIR"
chmod 644 "$FONT_DIR"/*

# Update font cache
print_info "Updating font cache with fc-cache..."
fc-cache -f "$FONT_DIR"

# Verify installation
if command -v fc-list >/dev/null 2>&1; then
    if fc-list : family | grep -iq "SF Pro"; then
        print_success "✓ SF Pro Display fonts are successfully installed and active!"
    else
        print_warning "Fonts copied to $FONT_DIR. Run 'fc-cache -f -v' if applications do not detect them."
    fi
fi

print_success "Installed $TOTAL_COUNT font files to: $FONT_DIR"
echo ""
