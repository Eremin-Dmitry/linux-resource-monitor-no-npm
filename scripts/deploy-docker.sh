#!/bin/bash
# Docker deployment script
# Usage: ./scripts/deploy-docker.sh [start|stop|logs|rebuild|clean]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

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

# Check if docker and docker-compose are available
check_docker() {
    if ! command -v docker &> /dev/null; then
        print_error "Docker is not installed"
        exit 1
    fi
    
    if ! command -v docker-compose &> /dev/null; then
        print_warning "docker-compose not found, trying docker compose..."
        COMPOSE_CMD="docker compose"
    else
        COMPOSE_CMD="docker-compose"
    fi
}

# Start containers
start() {
    print_info "Starting Resource Monitor with docker-compose..."
    cd "$PROJECT_ROOT"
    
    $COMPOSE_CMD up -d
    
    print_success "Containers started"
    print_info "Frontend: http://localhost"
    print_info "WebSocket: ws://localhost/ws"
    print_info "View logs: ./scripts/deploy-docker.sh logs"
}

# Stop containers
stop() {
    print_info "Stopping containers..."
    cd "$PROJECT_ROOT"
    
    $COMPOSE_CMD down
    
    print_success "Containers stopped"
}

# Show logs
show_logs() {
    print_info "Showing application logs..."
    cd "$PROJECT_ROOT"
    
    $COMPOSE_CMD logs -f resource-monitor
}

# Rebuild images
rebuild() {
    print_info "Rebuilding Docker images..."
    cd "$PROJECT_ROOT"
    
    $COMPOSE_CMD build --no-cache
    
    print_success "Images rebuilt"
}

# Clean up everything
cleanup() {
    print_warning "This will remove all containers, images, and volumes"
    read -p "Are you sure? (y/N) " -n 1 -r
    echo
    
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_info "Cleaning up..."
        cd "$PROJECT_ROOT"
        
        $COMPOSE_CMD down -v
        docker rmi linux-resource-monitor:latest || true
        
        print_success "Cleanup complete"
    else
        print_info "Cleanup cancelled"
    fi
}

# Show status
status() {
    print_info "Container status:"
    cd "$PROJECT_ROOT"
    
    $COMPOSE_CMD ps
}

# Show help
show_help() {
    cat << EOF
Usage: $0 [COMMAND]

Commands:
    start       Start containers
    stop        Stop containers
    logs        Show application logs (follow mode)
    rebuild     Rebuild Docker images
    clean       Remove all containers and images
    status      Show container status
    help        Show this help message

Examples:
    ./scripts/deploy-docker.sh start
    ./scripts/deploy-docker.sh logs
    ./scripts/deploy-docker.sh rebuild && ./scripts/deploy-docker.sh start

EOF
}

# Main
main() {
    local command="${1:-help}"
    
    check_docker
    
    case "$command" in
        start)
            start
            ;;
        stop)
            stop
            ;;
        logs)
            show_logs
            ;;
        rebuild)
            rebuild
            ;;
        clean)
            cleanup
            ;;
        status)
            status
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
