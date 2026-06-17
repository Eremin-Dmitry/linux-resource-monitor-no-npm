# Multi-stage Dockerfile for Linux Resource Monitor
# Stage 1: Builder
FROM ubuntu:22.04 AS builder

LABEL maintainer="Resource Monitor Team"

# Install build dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    libssl-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

# Copy source code
WORKDIR /build
COPY . .

# Build the application using CMake presets
RUN cd backend && \
    cmake --preset=default && \
    cmake --build build --config Release

# Stage 2: Runtime
FROM ubuntu:22.04

LABEL maintainer="Resource Monitor Team" \
      description="Linux Resource Monitor - System metrics over WebSocket"

# Install only runtime dependencies
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libssl3 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy frontend files
COPY frontend /app/frontend

# Copy built binary from builder stage
COPY --from=builder /build/backend/build/resource-monitor /app/

# Create logs directory
RUN mkdir -p /app/logs && \
    chmod 755 /app && \
    chmod 755 /app/logs

# Create non-root user for security
RUN useradd -m -u 1000 monitor && \
    chown -R monitor:monitor /app

USER monitor

# Health check
HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD wget -q -O- http://localhost:9002/ws || exit 1

# Expose WebSocket port
EXPOSE 9002

# Environment variables with defaults
ENV RESOURCE_MONITOR_PORT=9002 \
    RESOURCE_MONITOR_UPDATE_INTERVAL_MS=1000 \
    RESOURCE_MONITOR_TOP_PROCESSES_COUNT=25 \
    RESOURCE_MONITOR_LOG_LEVEL=INFO \
    RESOURCE_MONITOR_LOG_FILE=/app/logs/resource-monitor.log \
    RESOURCE_MONITOR_LOG_TO_CONSOLE=true \
    RESOURCE_MONITOR_LOG_TO_FILE=true

# Run the application
CMD ["/app/resource-monitor"]
