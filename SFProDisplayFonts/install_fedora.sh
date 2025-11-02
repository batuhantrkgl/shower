#!/bin/bash
# SF Pro Display Font Installation Script for Fedora/Linux

set -e

echo "SF Pro Display Font Installation for Fedora"
echo "==========================================="

# Get the directory where the script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check if running as root (needed for system-wide installation)
if [ "$EUID" -eq 0 ]; then
    echo "Installing fonts system-wide..."
    FONT_DIR="/usr/share/fonts/truetype/sf-pro-display"
    INSTALL_SYSTEM=true
else
    echo "Installing fonts for current user..."
    FONT_DIR="$HOME/.local/share/fonts/sf-pro-display"
    INSTALL_SYSTEM=false
fi

# Create font directory
echo "Creating font directory: $FONT_DIR"
mkdir -p "$FONT_DIR"

# Find and copy all .ttf files
echo "Copying font files..."
TTF_COUNT=0
if ls "$SCRIPT_DIR"/*.ttf >/dev/null 2>&1; then
    cp "$SCRIPT_DIR"/*.ttf "$FONT_DIR/"
    TTF_COUNT=$(ls "$SCRIPT_DIR"/*.ttf | wc -l)
    echo "Copied $TTF_COUNT .ttf files"
fi

# Find and copy all .otf files if they exist
OTF_COUNT=0
if ls "$SCRIPT_DIR"/*.otf >/dev/null 2>&1; then
    cp "$SCRIPT_DIR"/*.otf "$FONT_DIR/"
    OTF_COUNT=$(ls "$SCRIPT_DIR"/*.otf | wc -l)
    echo "Copied $OTF_COUNT .otf files"
fi

# Check if any fonts were copied
TOTAL_COUNT=$((TTF_COUNT + OTF_COUNT))
if [ $TOTAL_COUNT -eq 0 ]; then
    echo "Error: No font files (.ttf or .otf) found in $SCRIPT_DIR"
    exit 1
fi

# Set proper permissions
if [ "$INSTALL_SYSTEM" = true ]; then
    chmod 644 "$FONT_DIR"/*
else
    chmod 644 "$FONT_DIR"/*
fi

# Update font cache
echo "Updating font cache..."
fc-cache -f -v "$FONT_DIR"

echo ""
echo "Installation complete!"
echo "====================="
echo ""
echo "Installed $TOTAL_COUNT font files to: $FONT_DIR"
echo ""
echo "To verify installation, run:"
echo "  fc-list | grep 'SF Pro Display'"
echo ""
echo "You may need to restart applications to see the new fonts."
echo ""

# Verify installation
echo "Verifying installation..."
if fc-list | grep -q "SF Pro Display"; then
    echo "✓ SF Pro Display fonts are now available!"
    fc-list | grep "SF Pro Display" | head -5
else
    echo "⚠ Warning: Fonts installed but not detected in font cache."
    echo "Try running: fc-cache -f -v"
fi
