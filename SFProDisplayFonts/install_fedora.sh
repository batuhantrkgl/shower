#!/bin/bash
# Backward-compatible wrapper for Fedora/Linux font installation
[ -d "/usr/bin" ] && [[ ":$PATH:" != *":/usr/bin:"* ]] && export PATH="/usr/bin:$PATH"
SCRIPT_DIR="${BASH_SOURCE[0]%/*}"
[ "$SCRIPT_DIR" = "${BASH_SOURCE[0]}" ] && SCRIPT_DIR="."
REPO_ROOT="$(cd "$SCRIPT_DIR/.." 2>/dev/null && pwd)"
exec "$REPO_ROOT/videotimeline.sh" fonts "$@"
