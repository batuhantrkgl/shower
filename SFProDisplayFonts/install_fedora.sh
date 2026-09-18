#!/bin/bash
# SF Pro Display Font Installation Script for Fedora/Linux

set -e

echo "SF Pro Display Font Installation for Fedora"
echo "==========================================="

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "Note: install_fedora.sh is now powered by the universal install_fonts.sh script."
exec "$SCRIPT_DIR/install_fonts.sh" "$@"
