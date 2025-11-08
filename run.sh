#!/bin/bash

# VideoTimeline Application Runner Script
# Provides easy application startup with server connection

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
DEFAULT_SERVER_HOST="localhost"
DEFAULT_SERVER_PORT=3232
APP_BINARY="./build/VideoTimeline"
SERVER_RUN_SCRIPT="./server/run.sh"

# Printer helpers
print_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_app() { echo -e "${CYAN}[APP]${NC} $1"; }

# Usage
show_usage() {
    echo "VideoTimeline Application Runner"
    echo ""
    echo "Usage: $0 [OPTIONS] [APP_OPTIONS]"
    echo ""
    echo "Runner Options:"
    echo "  -s, --server HOST    Server hostname (default: $DEFAULT_SERVER_HOST)"
    echo "  -p, --port PORT      Server port (default: $DEFAULT_SERVER_PORT)"
    echo "  -a, --auto-server    Start server automatically if not running"
    echo "  -b, --build          Force rebuild before running"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Application Options (passed to VideoTimeline):"
    echo "  --auto               Start video playback automatically"
    echo "  --network RANGE      Network range for scanning (e.g., 192.168.1)"
    echo "  --dpi VALUE          Force specific DPI scaling"
    echo "  --test-time TIME     Set test time (e.g., '14:30')"
    echo "  --log-level LEVEL    Set log level: error, warn, info, debug"
    echo "  --log-file           Enable file logging with rotation"
    echo "  --cache-size SIZE    Set cache size in GB (2-8, default: 4)"
    echo "  --version            Show application version"
    echo ""
    echo "Special Event Options:"
    echo "  --date DD:MM:YYYY    Date for special event (e.g., 10:11:2025)"
    echo "  --time HH:MM         Time for special event (e.g., 09:05)"
    echo "  --image URL          Image/video URL for event"
    echo "  --title TEXT         Title for special event"
    echo "  --duration SECS      Duration in seconds (default: 180)"
    echo ""
    echo "Examples:"
    echo "  $0                          # Run app connecting to default server"
    echo "  $0 -s myserver -p 8080     # Connect to specific server"
    echo "  $0 --auto-server           # Start server if needed"
    echo "  $0 --auto --test-time 14:30  # Auto-start with test time"
    echo "  $0 -a --auto --dpi 192     # Auto server + auto play + custom DPI"
    echo "  $0 --date 10:11:2025 --time 09:05  # Test special event"
}

# Build if binary missing
check_app_binary() {
    if [ ! -f "$APP_BINARY" ]; then
        print_warning "Application binary not found at $APP_BINARY"
        print_info "Attempting to build application..."
        if [ -f "./scripts/build.sh" ]; then
            if ./scripts/build.sh; then
                print_success "Application built successfully"
            else
                print_error "Failed to build application"
                exit 1
            fi
        else
            print_error "Build script not found. Please build the application first."
            exit 1
        fi
    fi
}

# Check server health
check_server() {
    local host=$1
    local port=$2
    print_info "Checking server status at $host:$port..."
    if command -v curl >/dev/null 2>&1; then
        if curl -s --max-time 2 "http://$host:$port/api/schedule" >/dev/null 2>&1; then
            print_success "Server is running and responding at $host:$port"
            return 0
        else
            print_warning "Server is not responding at $host:$port"
            return 1
        fi
    elif command -v wget >/dev/null 2>&1; then
        if wget -q --timeout=2 -O /dev/null "http://$host:$port/api/schedule" >/dev/null 2>&1; then
            print_success "Server is running and responding at $host:$port"
            return 0
        else
            print_warning "Server is not responding at $host:$port"
            return 1
        fi
    else
        print_warning "Cannot verify server status (curl or wget not found)"
        return 0
    fi
}

# Start server if needed
start_server_if_needed() {
    local host=$1
    local port=$2
    if ! check_server "$host" "$port"; then
        print_info "Server not running. Starting server..."
        if [ -f "$SERVER_RUN_SCRIPT" ]; then
            "$SERVER_RUN_SCRIPT" -p "$port" -d
            sleep 2
            check_server "$host" "$port" || print_warning "Server may still be starting..."
        else
            print_warning "Server run script not found. Please start the server manually."
        fi
    fi
}

# Main
main() {
    local server_host=$DEFAULT_SERVER_HOST
    local server_port=$DEFAULT_SERVER_PORT
    local auto_server=false
    local force_build=false
    local app_args=()

    # Parse runner options; pass the rest to the app
    while [[ $# -gt 0 ]]; do
        case "$1" in
            -s|--server)
                server_host="$2"; shift 2 ;;
            -p|--port)
                server_port="$2"; shift 2 ;;
            -a|--auto-server)
                auto_server=true; shift ;;
            -b|--build)
                force_build=true; shift ;;
            -h|--help)
                show_usage; exit 0 ;;
            --)
                shift; app_args+=("$@"); break ;;
            *)
                # First unknown option: treat remaining args as app args
                app_args+=("$@"); break ;;
        esac
    done

    # Optional rebuild
    if [ "$force_build" = true ]; then
        print_info "Forcing rebuild..."
        ./scripts/build.sh
    fi

    check_app_binary

    # Server handling
    if [ "$auto_server" = true ]; then
        start_server_if_needed "$server_host" "$server_port"
    else
        check_server "$server_host" "$server_port" || true
    fi

    export VIDEOTIMELINE_SERVER_HOST="$server_host"
    export VIDEOTIMELINE_SERVER_PORT="$server_port"

    print_info "Starting VideoTimeline Application..."
    print_info "Connecting to server at $server_host:$server_port"
    print_app "VideoTimeline Application starting..."

    exec "$APP_BINARY" "${app_args[@]}"
}

main "$@"