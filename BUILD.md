# Building Linux Resource Monitor

This guide covers building and running the Linux Resource Monitor application.

## Quick Start

### Prerequisites

For Ubuntu 22.04+:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libssl-dev
```

For other distributions, install equivalent packages.

### Build Release Version

```bash
./scripts/build.sh release
./scripts/build.sh run
```

The application will start on port 9002 by default.

## Build Configurations

### Debug Build (with symbols and debugging info)

```bash
./scripts/build.sh debug
./scripts/build.sh run  # Uses release build by default
# Or explicitly:
./backend/build-debug/resource-monitor 9002
```

**Environment:** Full debug symbols, no optimizations, assertions enabled.

### Release Build (optimized)

```bash
./scripts/build.sh release
./backend/build/resource-monitor 9002
```

**Environment:** Optimizations enabled (-O3), NDEBUG enabled, smaller binary.

### Address Sanitizer Build (memory debugging)

```bash
./scripts/build.sh sanitizer
./backend/build-sanitizer/resource-monitor 9002
```

**Environment:** Detects memory leaks and use-after-free errors. Slower execution.

## Manual Build with CMake Presets

CMake presets are configured in `CMakePresets.json`:

```bash
cd backend

# Configure with default (Release) preset
cmake --preset=default
cmake --build build

# Or configure with debug preset
cmake --preset=debug
cmake --build build-debug

# Or with sanitizer preset
cmake --preset=sanitizer
cmake --build build-sanitizer
```

## Environment Configuration

Configure the application using environment variables:

```bash
# Set all configuration via environment
export RESOURCE_MONITOR_PORT=9002
export RESOURCE_MONITOR_UPDATE_INTERVAL_MS=1000
export RESOURCE_MONITOR_TOP_PROCESSES_COUNT=25
export RESOURCE_MONITOR_LOG_LEVEL=DEBUG
export RESOURCE_MONITOR_LOG_FILE=logs/resource-monitor.log
export RESOURCE_MONITOR_LOG_TO_CONSOLE=true
export RESOURCE_MONITOR_LOG_TO_FILE=true

./backend/build/resource-monitor
```

Or create a `.env` file (copy from `.env.example`):

```bash
cp .env.example .env
# Edit .env with your configuration
```

## Configuration Options

| Variable | Default | Description |
|----------|---------|-------------|
| `RESOURCE_MONITOR_PORT` | 9002 | WebSocket server port |
| `RESOURCE_MONITOR_UPDATE_INTERVAL_MS` | 1000 | Metrics update interval in milliseconds |
| `RESOURCE_MONITOR_TOP_PROCESSES_COUNT` | 25 | Number of top processes to report |
| `RESOURCE_MONITOR_LOG_LEVEL` | INFO | Log level: TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL |
| `RESOURCE_MONITOR_LOG_FILE` | logs/resource-monitor.log | Log file path |
| `RESOURCE_MONITOR_LOG_TO_CONSOLE` | true | Enable console logging |
| `RESOURCE_MONITOR_LOG_TO_FILE` | true | Enable file logging |

## Building with Docker

```bash
./scripts/build.sh docker
```

Or manually:

```bash
docker build -t linux-resource-monitor:latest .
```

## Troubleshooting

### CMake not found

```bash
sudo apt-get install cmake
```

### Missing OpenSSL development files

```bash
sudo apt-get install libssl-dev
```

### Permission denied when running script

```bash
chmod +x ./scripts/build.sh
chmod +x ./scripts/deploy-docker.sh
```

### Build fails with "Cannot open source file"

Ensure you're running CMake from the `backend` directory:

```bash
cd backend
cmake --preset=default
```

### Address Sanitizer build fails

Address Sanitizer requires additional compiler flags. Ensure you have a recent GCC or Clang:

```bash
gcc --version  # Should be >= 5.0
```

## Code Quality

### Format code

```bash
./scripts/build.sh format
```

Requires `clang-format`.

### Clean build artifacts

```bash
./scripts/build.sh clean
```

## Continuous Integration

The project includes GitHub Actions CI/CD pipeline (`.github/workflows/ci-cd.yml`):

- **Build**: Compiles in Debug and Release modes
- **Docker**: Builds Docker image
- **Lint**: Checks code formatting
- **Security**: Runs vulnerability scans
- **Deploy Dry-Run**: Validates docker-compose configuration

See GitHub Actions tab in your repository for run history.

## Performance Notes

- **Release build** is significantly faster than Debug
- **Update interval** of 1000ms (1 second) provides good balance
- **Top 25 processes** is reasonable; adjust per `RESOURCE_MONITOR_TOP_PROCESSES_COUNT`
- Memory footprint is typically < 50MB

## Next Steps

- See [DEPLOYMENT.md](DEPLOYMENT.md) for production deployment
- Check [README.md](README.md) for architecture and features
- Review [frontend documentation](frontend/README.md) for UI details
