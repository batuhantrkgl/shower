#!/bin/bash
# VideoTimeline Raspberry Pi Installation Wrapper
# Forwards execution to the universal Linux installer (install_linux.sh)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "VideoTimeline Raspberry Pi Installation"
echo "======================================"
echo "Note: Using universal Linux installer (supports Raspberry Pi OS, Debian, Ubuntu, Fedora, Arch, openSUSE, Alpine)."
echo ""

exec "$SCRIPT_DIR/install_linux.sh" "$@"
