# Linux Resource Monitor

[![CI/CD](https://github.com/yourusername/linux-resource-monitor/workflows/CI%2FCD/badge.svg)](https://github.com/yourusername/linux-resource-monitor/actions)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux-green.svg)](https://www.linux.org/)

A production-ready Linux resource monitoring application with a C++ backend and WebUI frontend. Displays CPU load, memory usage, swap usage, load average, uptime, and a real-time process table.

**Features**: C++17 backend, WebSocket server, zero-dependency frontend, Docker support, comprehensive logging, environment configuration, and CI/CD automation.

## Quick Start

### Docker (Recommended)

```bash
docker-compose up -d
# Frontend: http://localhost
# WebSocket: ws://localhost/ws
```

### Native Build

```bash
./scripts/build.sh release
./backend/build/resource-monitor
# Frontend: http://localhost:5173 (serve frontend separately)
```

## Key Features

✅ **Modern C++ Backend**
- C++17 with comprehensive error handling
- Real-time `/proc` filesystem monitoring
- Efficient delta-based CPU calculations

✅ **Zero-Dependency Frontend**
- Pure HTML/CSS/JavaScript
- TypeScript source included
- No npm, Node.js, or webpack required
- Responsive design

✅ **Production-Ready Infrastructure**
- Docker & docker-compose support
- GitHub Actions CI/CD pipeline
- Comprehensive logging system
- Environment-based configuration
- Security-focused design

✅ **Well-Documented**
- Extensive Doxygen documentation
- Architecture diagrams
- Deployment guide
- Contributing guidelines

## Documentation

| Document | Purpose |
|----------|---------|
| [BUILD.md](BUILD.md) | Building from source, CMake presets, configurations |
| [DEPLOYMENT.md](DEPLOYMENT.md) | Production deployment, Docker, systemd, Kubernetes |
| [ARCHITECTURE.md](ARCHITECTURE.md) | System design, data flow, performance characteristics |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Development setup, code standards, testing |
| [SECURITY.md](SECURITY.md) | Security practices, vulnerability reporting, best practices |

## Project Structure

```
linux-resource-monitor/
├── backend/                          # C++ application
│   ├── src/
│   │   ├── ResourceMonitorApplication.cpp    # Entry point with config/logging
│   │   ├── SystemResourceMonitor.[cpp/hpp]   # System metrics collection
│   │   ├── ProcessResourceMonitor.[cpp/hpp]  # Per-process metrics
│   │   ├── ResourceWebSocketServer.[cpp/hpp] # WebSocket server
│   │   ├── Config.[cpp/hpp]                  # Configuration management
│   │   └── Logger.[cpp/hpp]                  # Logging system
│   └── CMakeLists.txt               # Build configuration
├── frontend/                         # Web interface
│   ├── index.html                   # Dashboard markup
│   ├── app.js                       # JavaScript runtime
│   ├── styles.css                   # Styling
│   └── src/main.ts                  # TypeScript source
├── CMakePresets.json                # CMake presets (debug, release, sanitizer)
├── Dockerfile                       # Multi-stage Docker image
├── docker-compose.yml               # Docker Compose configuration
├── nginx.conf                       # Nginx reverse proxy config
├── scripts/
│   ├── build.sh                     # Build automation script
│   └── deploy-docker.sh             # Docker deployment helper
├── .github/workflows/ci-cd.yml      # GitHub Actions pipeline
├── BUILD.md                         # Build instructions
├── DEPLOYMENT.md                    # Deployment guide
├── ARCHITECTURE.md                  # System architecture
├── CONTRIBUTING.md                  # Contribution guidelines
└── SECURITY.md                      # Security policy
```

## Requirements

**Target**: Ubuntu 22.04 LTS or newer (any Linux distribution with modern toolchain)

**Build Dependencies**:
```bash
sudo apt-get install -y build-essential cmake libssl-dev
```

**Runtime**: GLIBC 2.31+, OpenSSL 3.0+

**Optional**: Docker, docker-compose for containerized deployment

## Installation & Usage

### Option 1: Docker (Easiest)

```bash
# Start all services (backend + Nginx reverse proxy)
./scripts/deploy-docker.sh start

# View logs
./scripts/deploy-docker.sh logs

# Stop
./scripts/deploy-docker.sh stop
```

Access at: http://localhost

### Option 2: Manual Build & Run

```bash
# Build
./scripts/build.sh release

# Run with default configuration
./backend/build/resource-monitor

# Or with custom port
./backend/build/resource-monitor 9010

# Serve frontend (in another terminal)
cd frontend
python3 -m http.server 5173
```

Access at: http://localhost:5173

### Option 3: Systemd Service

See [DEPLOYMENT.md](DEPLOYMENT.md) for production systemd setup.

## Configuration

Configure via environment variables:

```bash
# Example: custom port and debug logging
export RESOURCE_MONITOR_PORT=9002
export RESOURCE_MONITOR_LOG_LEVEL=DEBUG
./backend/build/resource-monitor
```

**Configuration Options**:

| Variable | Default | Description |
|----------|---------|-------------|
| `RESOURCE_MONITOR_PORT` | 9002 | WebSocket server port |
| `RESOURCE_MONITOR_UPDATE_INTERVAL_MS` | 1000 | Metrics refresh interval (ms) |
| `RESOURCE_MONITOR_TOP_PROCESSES_COUNT` | 25 | Number of top processes to report |
| `RESOURCE_MONITOR_LOG_LEVEL` | INFO | Log level (TRACE\|DEBUG\|INFO\|WARN\|ERROR\|CRITICAL) |
| `RESOURCE_MONITOR_LOG_FILE` | logs/resource-monitor.log | Log file path |
| `RESOURCE_MONITOR_LOG_TO_CONSOLE` | true | Enable console logging |
| `RESOURCE_MONITOR_LOG_TO_FILE` | true | Enable file logging |

Copy `.env.example` to `.env` and edit for local development.

## Build Variants

```bash
# Release (optimized, -O3)
./scripts/build.sh release

# Debug (with symbols, -g)
./scripts/build.sh debug

# Address Sanitizer (memory checking)
./scripts/build.sh sanitizer

# Docker image
./scripts/build.sh docker
```

See [BUILD.md](BUILD.md) for detailed build documentation.

## Metrics Collected

**System Metrics**:
- CPU usage (%) - calculated from `/proc/stat` deltas
- Memory - total, used, percentage
- Swap - total, used, percentage
- Load average - 1min, 5min, 15min
- Uptime - seconds since boot

**Per-Process Metrics** (top 25):
- Process ID (PID)
- Process name
- State (R=running, S=sleeping, Z=zombie, etc.)
- CPU usage (%) - calculated from process CPU ticks
- Memory (MB) - RSS memory

**Data Format**: JSON-over-WebSocket, updated every 1 second

## Architecture

```
┌──────────────────────────────────────────────────────────┐
│ Frontend (Browser)                                       │
│ HTML/CSS/JavaScript Dashboard                            │
└──────────────────────────────────────────────────────────┘
                    WebSocket ↕
                  ws://localhost/ws
                    (JSON payload)
┌──────────────────────────────────────────────────────────┐
│ C++ Backend (Resource Monitor)                           │
│                                                          │
│ SystemResourceMonitor    → /proc/stat, /proc/meminfo    │
│ ProcessResourceMonitor   → /proc/[pid]/stat             │
│ ResourceWebSocketServer  → Client distribution          │
│ Logger                   → File and console output       │
│ Config                   → Environment configuration    │
└──────────────────────────────────────────────────────────┘
                    ↓ reads
                /proc filesystem
```

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed system design.

## Performance

- **Memory**: ~15-30 MB typical
- **CPU**: <5% of one core
- **Network**: ~2-5 KB per second per client
- **Latency**: <100ms from metrics collection to UI update

## Security

✅ No external network connections
✅ Runs as unprivileged user (in Docker/systemd)
✅ Comprehensive input validation
✅ Error handling and graceful degradation
✅ Security scanning in CI/CD
✅ Address Sanitizer testing

See [SECURITY.md](SECURITY.md) for security practices and hardening guide.

## Development

### Setup Development Environment

```bash
# Install dependencies
sudo apt-get install -y build-essential cmake libssl-dev clang-format

# Clone and build
git clone https://github.com/yourusername/linux-resource-monitor.git
cd linux-resource-monitor
./scripts/build.sh debug
```

### Run Tests/Checks

```bash
# Code formatting
./scripts/build.sh format

# Build all variants
./scripts/build.sh debug && ./scripts/build.sh release

# Docker build
./scripts/build.sh docker

# Clean
./scripts/build.sh clean
```

### Contribution Guidelines

See [CONTRIBUTING.md](CONTRIBUTING.md) for:
- Code style and standards
- Pull request process
- Debugging and testing
- Documentation requirements

## Troubleshooting

### Build fails: "cmake not found"
```bash
sudo apt-get install cmake
```

### Application crashes on startup
```bash
# Check logs
cat logs/resource-monitor.log

# Enable debug logging
export RESOURCE_MONITOR_LOG_LEVEL=DEBUG
./backend/build/resource-monitor
```

### WebSocket connection fails
- Verify port is not in use: `lsof -i :9002`
- Check firewall rules
- Ensure service is running: `systemctl status resource-monitor`

See [DEPLOYMENT.md](DEPLOYMENT.md) and [BUILD.md](BUILD.md) for more troubleshooting.

## API Data Format

**Example WebSocket message**:

```json
{
  "cpu": {"usage": 37.42},
  "memory": {
    "total": 15884,
    "used": 9120,
    "usage": 57.41
  },
  "swap": {
    "total": 2048,
    "used": 120,
    "usage": 5.86
  },
  "loadAverage": [1.12, 1.30, 1.44],
  "uptime": 38422,
  "processes": [
    {
      "pid": 1234,
      "name": "firefox",
      "state": "S",
      "cpu": 4.12,
      "memory": 512
    }
  ]
}
```

## CI/CD Pipeline

GitHub Actions automatically:
- Builds in Debug and Release modes
- Builds Docker image
- Runs code formatting checks
- Performs security scanning
- Validates docker-compose configuration

See `.github/workflows/ci-cd.yml` for pipeline details.

## Roadmap

- [ ] Process filtering and custom sorting
- [ ] Historical data and trending graphs
- [ ] Network and disk I/O metrics
- [ ] Unit test framework
- [ ] REST API endpoint
- [ ] Multi-system monitoring
- [ ] Alerting system

## License

MIT License - see LICENSE file for details

## Contributing

Contributions welcome! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Support

- 📖 Read [documentation](.)
- 🐛 Report issues on [GitHub Issues](https://github.com/yourusername/linux-resource-monitor/issues)
- 🔒 Security issues: see [SECURITY.md](SECURITY.md)

---

**Version**: 1.0.0  
**Last Updated**: 2024  
**Maintainer**: Resource Monitor Team
