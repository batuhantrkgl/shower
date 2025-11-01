#!/bin/bash

# VideoTimeline Server Runner Script
# Provides colored output and easy server management

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Configuration
DEFAULT_PORT=3232
SERVER_BINARY="./build/server/server"
BUILD_SCRIPT="../scripts/server-build.sh"

# Function to print colored output
print_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_server() {
    echo -e "${CYAN}[SERVER]${NC} $1"
}

# Function to check if server binary exists
check_server_binary() {
    if [ ! -f "$SERVER_BINARY" ]; then
        print_warning "Server binary not found at $SERVER_BINARY"
        if [ -f "$BUILD_SCRIPT" ]; then
            print_info "Attempting to build server..."
            if $BUILD_SCRIPT; then
                print_success "Server built successfully"
            else
                print_error "Failed to build server"
                exit 1
            fi
        else
            print_error "Build script not found. Please build the server first."
            exit 1
        fi
    fi
}

# Function to check if port is available
check_port() {
    local port=$1
    if command -v nc >/dev/null 2>&1; then
        if nc -z localhost $port 2>/dev/null; then
            print_warning "Port $port appears to be in use"
            return 1
        fi
    elif command -v lsof >/dev/null 2>&1; then
        if lsof -i :$port >/dev/null 2>&1; then
            print_warning "Port $port appears to be in use"
            return 1
        fi
    else
        print_warning "Cannot check port availability (nc or lsof not found)"
    fi
    return 0
}

# Function to find available port
find_available_port() {
    local port=$DEFAULT_PORT
    local max_attempts=10
    local attempt=1

    while [ $attempt -le $max_attempts ]; do
        if check_port $port; then
            echo $port
            return 0
        fi
        port=$((port + 1))
        attempt=$((attempt + 1))
    done

    print_error "Could not find an available port after $max_attempts attempts"
    exit 1
}

# Function to show usage
show_usage() {
    echo "VideoTimeline Server Runner"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -p, --port PORT     Specify server port (default: $DEFAULT_PORT)"
    echo "  -a, --auto-port     Automatically find an available port"
    echo "  -b, --build         Force rebuild before running"
    echo "  -d, --detach        Run server in background"
    echo "  -s, --status        Check server status"
    echo "  -k, --kill          Kill running server instances"
    echo "  -h, --help          Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                   # Run server on default port"
    echo "  $0 -p 8080          # Run server on port 8080"
    echo "  $0 --auto-port      # Run server on first available port"
    echo "  $0 --detach         # Run server in background"
    echo "  $0 --status         # Check if server is running"
    echo "  $0 --kill           # Stop all running server instances"
}

# Function to check server status
check_status() {
    local port=$1
    if [ -z "$port" ]; then
        port=$DEFAULT_PORT
    fi

    print_info "Checking server status on port $port..."

    if command -v curl >/dev/null 2>&1; then
        if curl -s --max-time 2 http://localhost:$port/api/schedule >/dev/null 2>&1; then
            print_success "Server is running and responding on port $port"
            return 0
        else
            print_warning "Server is not responding on port $port"
            return 1
        fi
    elif command -v wget >/dev/null 2>&1; then
        if wget -q --timeout=2 -O /dev/null http://localhost:$port/api/schedule >/dev/null 2>&1; then
            print_success "Server is running and responding on port $port"
            return 0
        else
            print_warning "Server is not responding on port $port"
            return 1
        fi
    else
        # Check if process is running
        if pgrep -f "server" >/dev/null 2>&1; then
            print_success "Server process is running (cannot verify HTTP response without curl/wget)"
            return 0
        else
            print_warning "No server process found"
            return 1
        fi
    fi
}

# Function to kill running servers
kill_servers() {
    print_info "Looking for running server processes..."

    local pids=$(pgrep -f "server" 2>/dev/null || true)

    if [ -z "$pids" ]; then
        print_info "No running server processes found"
        return 0
    fi

    print_warning "Found server processes: $pids"
    echo "Killing server processes..."
    kill $pids 2>/dev/null || true

    # Wait a moment and check again
    sleep 1
    pids=$(pgrep -f "server" 2>/dev/null || true)
    if [ -z "$pids" ]; then
        print_success "All server processes killed"
    else
        print_warning "Some processes may still be running: $pids"
    fi
}

# Main function
main() {
    local port=""
    local auto_port=false
    local force_build=false
    local detach=false

    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
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
            -s|--status)
                check_status "$port"
                exit $?
                ;;
            -k|--kill)
                kill_servers
                exit $?
                ;;
            -h|--help)
                show_usage
                exit 0
                ;;
            *)
                print_error "Unknown option: $1"
                show_usage
                exit 1
                ;;
        esac
    done

    # Force build if requested
    if [ "$force_build" = true ]; then
        print_info "Forcing rebuild..."
        if [ -f "$BUILD_SCRIPT" ]; then
            $BUILD_SCRIPT
        else
            print_error "Build script not found"
            exit 1
        fi
    fi

    # Check server binary
    check_server_binary

    # Determine port
    if [ "$auto_port" = true ]; then
        port=$(find_available_port)
        print_info "Using available port: $port"
    elif [ -z "$port" ]; then
        port=$DEFAULT_PORT
    fi

    # Check if port is available
    if ! check_port $port; then
        if [ "$auto_port" = false ]; then
            print_error "Port $port is not available. Use --auto-port to find an available port."
            exit 1
        fi
    fi

    # Run server
    print_info "Starting VideoTimeline Server on port $port..."

    if [ "$detach" = true ]; then
        print_info "Running server in background..."
        nohup $SERVER_BINARY $port > server.log 2>&1 &
        local pid=$!
        print_success "Server started in background (PID: $pid)"
        print_info "Logs are being written to server.log"
        print_info "Use '$0 --status' to check server status"
        print_info "Use '$0 --kill' to stop the server"
    else
        print_info "Press Ctrl+C to stop the server"
        print_server "VideoTimeline Server starting..."
        exec $SERVER_BINARY $port
    fi
}

# Run main function
main "$@"
