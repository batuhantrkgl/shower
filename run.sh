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

print_app() {
    echo -e "${CYAN}[APP]${NC} $1"
}

# Function to check if application binary exists
check_app_binary() {
    if [ ! -f "$APP_BINARY" ]; then
        print_warning "Application binary not found at $APP_BINARY"
        print_info "Attempting to build application..."
        if [ -f "./build/build.sh" ]; then
            if ./build/build.sh; then
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

# Function to check server status
check_server() {
    local host=$1
    local port=$2

    print_info "Checking server status at $host:$port..."

    if command -v curl >/dev/null 2>&1; then
        if curl -s --max-time 2 http://$host:$port/api/schedule >/dev/null 2>&1; then
            print_success "Server is running and responding at $host:$port"
            return 0
        else
            print_warning "Server is not responding at $host:$port"
            return 1
        fi
    elif command -v wget >/dev/null 2>&1; then
        if wget -q --timeout=2 -O /dev/null http://$host:$port/api/schedule >/dev/null 2>&1; then
            print_success "Server is running and responding at $host:$port"
            return 0
        else
            print_warning "Server is not responding at $host:$port"
            return 1
        fi
    else
        print_warning "Cannot verify server status (curl or wget not found)"
        return 0  # Assume it's running
    fi
}

# Function to start server if needed
start_server_if_needed() {
    local host=$1
    local port=$2

    if ! check_server $host $port; then
        print_info "Server not running. Starting server..."
        if [ -f "$SERVER_RUN_SCRIPT" ]; then
            $SERVER_RUN_SCRIPT -p $port -d
            sleep 2  # Give server time to start
            if check_server $host $port; then
                print_success "Server started successfully"
            else
                print_warning "Server may still be starting..."
            fi
        else
            print_warning "Server run script not found. Please start the server manually."
        fi
    fi
}

# Function to show usage
show_usage() {
    echo "VideoTimeline Application Runner"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -s, --server HOST    Server hostname (default: $DEFAULT_SERVER_HOST)"
    echo "  -p, --port PORT      Server port (default: $DEFAULT_SERVER_PORT)"
    echo "  -a, --auto-server    Start server automatically if not running"
    echo "  -b, --build          Force rebuild before running"
    echo "  -h, --help           Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                          # Run app connecting to default server"
    echo "  $0 -s myserver -p 8080     # Connect to specific server"
    echo "  $0 --auto-server           # Start server if needed"
}

# Main function
main() {
    local server_host=$DEFAULT_SERVER_HOST
    local server_port=$DEFAULT_SERVER_PORT
    local auto_server=false
    local force_build=false

    # Parse command line arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            -s|--server)
                server_host="$2"
                shift 2
                ;;
            -p|--port)
                server_port="$2"
                shift 2
                ;;
            -a|--auto-server)
                auto_server=true
                shift
                ;;
            -b|--build)
                force_build=true
                shift
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
        if [ -f "./build/build.sh" ]; then
            ./build/build.sh
        else
            print_error "Build script not found"
            exit 1
        fi
    fi

    # Check application binary
    check_app_binary

    # Handle server connection
    if [ "$auto_server" = true ]; then
        start_server_if_needed $server_host $server_port
    else
        check_server $server_host $server_port || true  # Don't exit on failure
    fi

    # Set environment variable for server connection
    export VIDEOTIMELINE_SERVER_HOST=$server_host
    export VIDEOTIMELINE_SERVER_PORT=$server_port

    # Run application
    print_info "Starting VideoTimeline Application..."
    print_info "Connecting to server at $server_host:$server_port"
    print_app "VideoTimeline Application starting..."

    exec $APP_BINARY
}

# Run main function
main "$@"