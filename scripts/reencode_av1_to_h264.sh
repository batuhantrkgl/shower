#!/bin/bash
# Re-encode AV1/VP9 videos to H.264
[ -d "/usr/bin" ] && [[ ":$PATH:" != *":/usr/bin:"* ]] && export PATH="/usr/bin:$PATH"
SCRIPT_DIR="${BASH_SOURCE[0]%/*}"
[ "$SCRIPT_DIR" = "${BASH_SOURCE[0]}" ] && SCRIPT_DIR="."
REPO_ROOT="$(cd "$SCRIPT_DIR/.." 2>/dev/null && pwd)"
exec "$REPO_ROOT/videotimeline.sh" media reencode "$@"
