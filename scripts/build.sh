#!/bin/bash
# Build and deploy script for Resource Monitor
# Usage: ./scripts/build.sh [debug|release|docker]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print functions
print_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

print_success() {
    echo -e "${GREEN}✓${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

print_error() {
    echo -e "${RED}✗${NC} $1"
}

# Check dependencies
check_dependencies() {
    print_info "Checking dependencies..."
    
    local missing_deps=0
    
    if ! command -v cmake &> /dev/null; then
        print_error "cmake not found. Install it first."
        missing_deps=$((missing_deps + 1))
    fi
    
    if ! command -v make &> /dev/null; then
        print_error "make not found. Install build-essential first."
        missing_deps=$((missing_deps + 1))
    fi
    
    if ! dpkg -l | grep -q libssl-dev; then
        print_warning "libssl-dev not installed. Installing..."
        sudo apt-get install -y libssl-dev
    fi
    
    if [ $missing_deps -gt 0 ]; then
        print_error "Missing $missing_deps required dependencies."
        exit 1
    fi
    
    print_success "All dependencies satisfied"
}

# Build debug version
build_debug() {
    print_info "Building Debug configuration..."
    
    cd "$PROJECT_ROOT/backend"
    
    if [ -d "build-debug" ]; then
        print_warning "Removing previous debug build..."
        rm -rf build-debug
    fi
    
    cmake --preset=debug
    cmake --build build-debug
    
    print_success "Debug build complete: $PROJECT_ROOT/backend/build-debug/resource-monitor"
}

# Build release version
build_release() {
    print_info "Building Release configuration..."
    
    cd "$PROJECT_ROOT/backend"
    
    if [ -d "build" ]; then
        print_warning "Removing previous release build..."
        rm -rf build
    fi
    
    cmake --preset=default
    cmake --build build
    
    print_success "Release build complete: $PROJECT_ROOT/backend/build/resource-monitor"
}

# Build sanitizer version
build_sanitizer() {
    print_info "Building with Address Sanitizer..."
    
    cd "$PROJECT_ROOT/backend"
    
    if [ -d "build-sanitizer" ]; then
        print_warning "Removing previous sanitizer build..."
        rm -rf build-sanitizer
    fi
    
    cmake --preset=sanitizer
    cmake --build build-sanitizer
    
    print_success "Sanitizer build complete: $PROJECT_ROOT/backend/build-sanitizer/resource-monitor"
}

# Build Docker image
build_docker() {
    print_info "Building Docker image..."
    
    if ! command -v docker &> /dev/null; then
        print_error "Docker not found. Install Docker first."
        exit 1
    fi
    
    cd "$PROJECT_ROOT"
    docker build -t linux-resource-monitor:latest .
    
    print_success "Docker image built successfully"
}

# Run the application
run() {
    local binary="$1"
    local port="${2:-9002}"
    
    if [ ! -f "$binary" ]; then
        print_error "Binary not found: $binary"
        exit 1
    fi
    
    print_info "Starting Resource Monitor on port $port..."
    export RESOURCE_MONITOR_LOG_LEVEL=DEBUG
    export RESOURCE_MONITOR_LOG_TO_CONSOLE=true
    
    "$binary" "$port"
}

# Clean build artifacts
clean() {
    print_info "Cleaning build artifacts..."
    
    cd "$PROJECT_ROOT/backend"
    rm -rf build build-debug build-sanitizer
    
    print_success "Clean complete"
}

# Format code with clang-format
format() {
    print_info "Formatting C++ code..."
    
    if ! command -v clang-format &> /dev/null; then
        print_warning "clang-format not found. Skipping format check."
        return
    fi
    
    find "$PROJECT_ROOT/backend/src" -name "*.cpp" -o -name "*.hpp" | \
        xargs clang-format -i
    
    print_success "Code formatted"
}

# Show help
show_help() {
    cat << EOF
Usage: $0 [COMMAND]

Commands:
    debug       Build debug version with symbols
    release     Build optimized release version
    sanitizer   Build with Address Sanitizer for memory debugging
    docker      Build Docker image
    run         Run the application (use release build)
    clean       Remove build artifacts
    format      Format code with clang-format
    help        Show this help message

Environment variables:
    RESOURCE_MONITOR_PORT           WebSocket port (default: 9002)
    RESOURCE_MONITOR_LOG_LEVEL      Log level: TRACE|DEBUG|INFO|WARN|ERROR|CRITICAL
    RESOURCE_MONITOR_LOG_TO_CONSOLE Enable console logging (default: true)

Examples:
    ./scripts/build.sh debug
    ./scripts/build.sh release && ./scripts/build.sh run
    ./scripts/build.sh docker
    RESOURCE_MONITOR_LOG_LEVEL=DEBUG ./scripts/build.sh run

EOF
}

# Main script
main() {
    local command="${1:-help}"
    
    case "$command" in
        debug)
            check_dependencies
            build_debug
            ;;
        release)
            check_dependencies
            build_release
            ;;
        sanitizer)
            check_dependencies
            build_sanitizer
            ;;
        docker)
            build_docker
            ;;
        run)
            run "$PROJECT_ROOT/backend/build/resource-monitor" "${2:-9002}"
            ;;
        clean)
            clean
            ;;
        format)
            format
            ;;
        help|--help|-h)
            show_help
            ;;
        *)
            print_error "Unknown command: $command"
            show_help
            exit 1
            ;;
    esac
}

main "$@"
