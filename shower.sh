#!/bin/bash
# VideoTimeline / Shower Unified Launcher
[ -d "/usr/bin" ] && [[ ":$PATH:" != *":/usr/bin:"* ]] && export PATH="/usr/bin:$PATH"
SCRIPT_DIR="${BASH_SOURCE[0]%/*}"
[ "$SCRIPT_DIR" = "${BASH_SOURCE[0]}" ] && SCRIPT_DIR="."
SCRIPT_DIR="$(cd "$SCRIPT_DIR" 2>/dev/null && pwd)"
exec "$SCRIPT_DIR/videotimeline.sh" "$@"
