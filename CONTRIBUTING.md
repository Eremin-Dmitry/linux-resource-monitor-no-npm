# Contributing

Thank you for your interest in contributing to Linux Resource Monitor!

## Development Setup

### Clone Repository

```bash
git clone https://github.com/yourusername/linux-resource-monitor.git
cd linux-resource-monitor
```

### Install Development Dependencies

Ubuntu 22.04:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  libssl-dev \
  clang-format \
  git
```

### Build for Development

```bash
./scripts/build.sh debug
```

## Code Standards

### C++ Style Guide

- **Standard**: C++17
- **Formatting**: clang-format (use `./scripts/build.sh format`)
- **Naming**: 
  - Classes: `PascalCase`
  - Functions: `camelCase`
  - Member variables: `m_camelCase`
  - Constants: `SCREAMING_SNAKE_CASE`
  - Private functions: `camelCase` with leading underscore

### Documentation

- Add Doxygen-style comments to all public APIs
- Include `@brief`, `@param`, `@return`, and `@throws` tags
- Provide examples in `@example` blocks when helpful
- Document non-obvious algorithms or trade-offs

Example:

```cpp
/**
 * @brief Calculate CPU usage percentage
 *
 * Compares current CPU sample with previous sample to calculate
 * usage percentage for the interval.
 *
 * @param current Current CPU time counters
 * @return double CPU usage percentage (0.0 to 100.0)
 * @throws std::runtime_error if calculation fails
 *
 * @example
 * @code
 * double usage = CalculateCpuUsagePercent(currentSample);
 * @endcode
 */
[[nodiscard]] double CalculateCpuUsagePercent(const CpuTimeSample& current);
```

## Making Changes

### 1. Create Feature Branch

```bash
git checkout -b feature/your-feature-name
# or for bugfixes:
git checkout -b fix/your-bugfix-name
```

### 2. Make Changes

- Write code following style guide
- Add tests if applicable
- Update documentation
- Format code: `./scripts/build.sh format`

### 3. Test Your Changes

```bash
# Test build with different configurations
./scripts/build.sh debug
./scripts/build.sh release

# Test with Docker
./scripts/build.sh docker

# Manual testing
./backend/build/resource-monitor
```

### 4. Commit with Clear Messages

```bash
git commit -m "Add feature: description of what you did

- Bullet point explaining the change
- Another detail if needed

Fixes #123"
```

### 5. Push and Create Pull Request

```bash
git push origin feature/your-feature-name
```

Then create a Pull Request on GitHub.

## Pull Request Guidelines

### Before Submitting

- [ ] Code follows style guide (run `./scripts/build.sh format`)
- [ ] All builds pass (debug, release, docker)
- [ ] Documentation is updated
- [ ] Commit messages are clear and descriptive
- [ ] No unnecessary changes or debug code

### PR Description Template

```markdown
## Description
Brief description of what this PR does.

## Related Issues
Fixes #123

## Changes
- Change 1
- Change 2
- Change 3

## Testing
How did you test this?

## Checklist
- [ ] Code follows style guide
- [ ] Documentation updated
- [ ] Builds successfully
- [ ] No breaking changes
```

## Architecture Overview

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed system design.

### Key Components

- **SystemResourceMonitor**: Reads CPU, memory, and system metrics
- **ProcessResourceMonitor**: Tracks per-process resource usage
- **ResourceWebSocketServer**: Handles WebSocket client connections
- **Config**: Environment-based configuration management
- **Logger**: Centralized logging system

## Testing

### Building Tests

Currently no automated test framework, but you can verify:

```bash
# Build succeeds
./scripts/build.sh debug
./scripts/build.sh release

# Application runs without errors
./backend/build/resource-monitor

# Logs are generated
cat logs/resource-monitor.log
```

### Contributing Tests

If you add significant logic, consider:

1. Adding unit test framework (e.g., Google Test)
2. Creating test data files
3. Documenting test procedures

## Debugging

### Debug Build

```bash
./scripts/build.sh debug
gdb ./backend/build-debug/resource-monitor
(gdb) run
```

### Address Sanitizer

Detects memory errors:

```bash
./scripts/build.sh sanitizer
./backend/build-sanitizer/resource-monitor
```

### Logging

Increase log level for debugging:

```bash
export RESOURCE_MONITOR_LOG_LEVEL=DEBUG
./backend/build/resource-monitor
```

## Documentation

### Updating README

Update [README.md](README.md) if you change:
- Features or capabilities
- Build/run instructions
- Architecture

### Adding Architecture Docs

Create [ARCHITECTURE.md](ARCHITECTURE.md) documenting:
- Component interactions
- Data flow
- Design decisions

### API Documentation

Doxygen comments in headers are converted to documentation:

```bash
doxygen Doxyfile  # Once Doxyfile is created
```

## Reporting Issues

### Bug Reports

Include:
- OS and version
- Steps to reproduce
- Expected vs actual behavior
- Relevant logs

### Feature Requests

Include:
- Use case and motivation
- Proposed solution (if any)
- Alternatives considered

## Code Review Process

1. GitHub Actions runs automated checks
2. Maintainers review code
3. Address feedback and update PR
4. Merge once approved

## License

By contributing, you agree to license your work under the same license as the project.

## Questions?

- Open an issue for discussions
- Check existing documentation first
- Review closed issues for similar questions

Thank you for contributing!
