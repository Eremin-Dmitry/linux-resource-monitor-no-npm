# Architecture

This document describes the architecture of Linux Resource Monitor.

## System Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Resource Monitor System                   │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│              Frontend (Web Browser)                          │
│  ┌──────────────────────────────────────────────────────┐   │
│  │  HTML/CSS/JavaScript Dashboard                       │   │
│  │  • CPU/Memory/Swap Gauges                           │   │
│  │  • Load Average Display                             │   │
│  │  • Process Table (sorted by CPU)                    │   │
│  │  • Real-time Updates (1-second intervals)           │   │
│  └──────────────────────────────────────────────────────┘   │
│                        ↓ WebSocket                            │
│                  ws://localhost:9002/ws                       │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│         C++ Backend (Resource Monitor Application)          │
│                                                              │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ ResourceMonitorApplication                           │  │
│  │ • Initializes components                             │  │
│  │ • Reads configuration from environment               │  │
│  │ • Sets up logging                                    │  │
│  │ • Runs WebSocket server                              │  │
│  └──────────────────────────────────────────────────────┘  │
│    ↓ Creates/Manages                                        │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ SystemResourceMonitor                                │  │
│  │ • Reads /proc/stat         → CPU usage               │  │
│  │ • Reads /proc/meminfo      → Memory/Swap             │  │
│  │ • Reads /proc/loadavg      → Load average            │  │
│  │ • Reads /proc/uptime       → System uptime           │  │
│  └──────────────────────────────────────────────────────┘  │
│    ↓ Creates/Manages                                        │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ ProcessResourceMonitor                               │  │
│  │ • Scans /proc/[pid]/stat   → Per-process metrics     │  │
│  │ • Reads /proc/[pid]/status → Memory usage            │  │
│  │ • Calculates CPU deltas     → Usage percentages      │  │
│  │ • Sorts by CPU usage        → Top 25 processes       │  │
│  └──────────────────────────────────────────────────────┘  │
│    ↓ Uses                                                    │
│  ┌──────────────────────────────────────────────────────┐  │
│  │ ResourceWebSocketServer                              │  │
│  │ • Accepts WebSocket connections                      │  │
│  │ • Sends metrics as JSON (1 update/second)            │  │
│  │ • Handles WebSocket protocol handshake               │  │
│  │ • Encodes payloads in WebSocket frames               │  │
│  └──────────────────────────────────────────────────────┘  │
│                      ↓ Network                               │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│         Linux Kernel / /proc Filesystem                     │
│                                                              │
│  /proc/stat       - CPU time counters (all CPUs)            │
│  /proc/meminfo    - Memory and swap information             │
│  /proc/loadavg    - Load average (1m, 5m, 15m)              │
│  /proc/uptime     - System uptime in seconds                │
│  /proc/[pid]/stat - Per-process CPU ticks                   │
│  /proc/[pid]/status - Per-process memory info               │
└─────────────────────────────────────────────────────────────┘
```

## Component Details

### Frontend (JavaScript/HTML/CSS)

**File**: `frontend/app.js`, `index.html`, `styles.css`

**Responsibilities**:
- Display system metrics in real-time dashboard
- Connect to WebSocket server
- Parse JSON payloads
- Render progress bars and gauges
- Handle connection state (connected/disconnected)
- Auto-reconnect on disconnection

**Key Features**:
- No npm/build tool required
- Responsive design
- Real-time updates without page reload
- Error handling and display

### Backend Components

#### ResourceMonitorApplication (main.cpp)

**Entry Point**: Creates and manages all components

**Flow**:
1. Load configuration from environment variables
2. Initialize logging system
3. Create resource monitors
4. Create WebSocket server
5. Run server (blocking until Stop is called)
6. Catch and log any exceptions

#### SystemResourceMonitor

**Responsibility**: Collect system-level metrics

**Data Sources**:
```
/proc/stat
    cpu  2379 34 2353 22330540 3675 127 456 0 0 0

    ↓ Parse
    
cpu_user = 2379
cpu_nice = 34
cpu_system = 2353
cpu_idle = 22330540
...

    ↓ Calculate Delta
    
idle_delta = current_idle - previous_idle
total_delta = current_total - previous_total
cpu_usage = 100.0 * (1.0 - idle_delta / total_delta)
```

**Metrics Collected**:
- CPU usage percentage (based on time deltas)
- Memory: total, used, percentage
- Swap: total, used, percentage
- Load average: 1min, 5min, 15min
- Uptime: seconds since boot

#### ProcessResourceMonitor

**Responsibility**: Collect per-process metrics

**Algorithm**:
```
1. Read total CPU ticks from /proc/stat
2. Scan /proc/ for directories (PID numbers)
3. For each PID:
   a. Read /proc/[pid]/stat → process name, state, CPU ticks
   b. Read /proc/[pid]/status → RSS memory
   c. Calculate CPU delta: (current_ticks - previous_ticks) / total_delta
4. Sort processes by CPU usage
5. Return top N processes
```

**First Sample Behavior**:
- CPU % = 0 (no previous data for delta)
- Memory = accurate
- Subsequent samples have accurate CPU percentages

#### ResourceWebSocketServer

**Responsibility**: Handle WebSocket protocol and connections

**WebSocket Flow**:
```
1. Listen on TCP socket (port 9002)
2. Accept client connection
3. Read HTTP handshake request
4. Extract Sec-WebSocket-Key header
5. Calculate: SHA1(key + GUID) → base64
6. Send HTTP 101 Switching Protocols response
7. Enter message loop:
   a. Call payloadProvider() to get JSON
   b. Encode as WebSocket text frame
   c. Send to all connected clients
   d. Sleep for update interval
```

**WebSocket Frame Format** (RFC 6455):
```
FIN=1, RSV=0, Opcode=1 (text)
[Mask bit] [Payload length] [Payload data]
```

#### Config

**Responsibility**: Configuration management

**Sources** (in order):
1. Environment variables (with `RESOURCE_MONITOR_` prefix)
2. Defaults in struct initialization

**Configuration Options**:
- `RESOURCE_MONITOR_PORT` - Server listening port
- `RESOURCE_MONITOR_UPDATE_INTERVAL_MS` - Metrics refresh rate
- `RESOURCE_MONITOR_TOP_PROCESSES_COUNT` - Number of processes to report
- `RESOURCE_MONITOR_LOG_LEVEL` - Logging verbosity
- `RESOURCE_MONITOR_LOG_FILE` - Log file path
- `RESOURCE_MONITOR_LOG_TO_CONSOLE` - Console output enable
- `RESOURCE_MONITOR_LOG_TO_FILE` - File output enable

#### Logger

**Responsibility**: Centralized logging

**Features**:
- Thread-safe (mutex protected)
- Multiple output targets (console, file)
- Severity levels (TRACE, DEBUG, INFO, WARN, ERROR, CRITICAL)
- Timestamps and formatting
- Directory creation for log files

## Data Flow

### Typical Update Cycle (1 second)

```
1. WebSocket server waits for update interval (1000ms)

2. Server calls payloadProvider callback

3. Callback:
   a. Lock resource monitor mutex
   b. Call systemResourceMonitor.CollectMetrics()
      - Read /proc files
      - Calculate CPU delta from previous sample
   c. Call processResourceMonitor.CollectTopProcesses(25)
      - Scan /proc/[pid]
      - Calculate per-process CPU usage
      - Sort and return top 25
   d. Build JSON payload
   e. Unlock mutex
   f. Return JSON string

4. Server encodes JSON as WebSocket frame

5. Server sends frame to all connected clients

6. Frontend receives JSON

7. Frontend updates HTML DOM elements

8. User sees updated dashboard

(repeat from step 1)
```

## Thread Safety

**Threading Model**:
- Main thread: Runs WebSocket server (accepts connections, receives data)
- Network threads: Handle individual client connections
- Metric collection: Protected by mutex in application

**Synchronization**:
- `std::mutex resourceMonitorMutex` protects access to:
  - `SystemResourceMonitor` state
  - `ProcessResourceMonitor` state
  - Metric collection operation

**No Concurrent Access**:
- Each metric collection is atomic (locked)
- Clients never read partially-updated metrics
- No data corruption possible

## Performance Characteristics

### CPU Usage

- **Update cycle**: ~50-200ms on typical system
- **Overhead**: < 5% of one CPU core
- **Sleep time**: ~ 800-950ms per second

### Memory Usage

- **Baseline**: ~10-20 MB
- **Top 25 processes**: +5-10 MB
- **Total**: ~15-30 MB typical

### Network Traffic

- **Per update**: ~2-5 KB JSON payload
- **Frequency**: 1 per second
- **Total**: ~2-5 MB/hour per client

## Scalability

### Horizontal Scaling

Not applicable - runs on single Linux system monitoring that system.

### Vertical Scaling

Can be improved:
- Increase `RESOURCE_MONITOR_UPDATE_INTERVAL_MS` to reduce CPU usage
- Decrease `RESOURCE_MONITOR_TOP_PROCESSES_COUNT` to reduce payload size
- Run multiple instances with different focus areas (not typical)

### Client Connections

- Tested with multiple simultaneous connections
- No hard limit enforced
- Scales to hundreds of clients
- Network bandwidth becomes limiting factor

## Error Handling

### Graceful Degradation

If metric collection fails:
- Return error JSON instead of crashing
- Log error with details
- Continue running
- Try again on next update

### Exception Handling

- Application-level try-catch
- Resource monitor operations throw on /proc read failures
- Server catch-all on socket errors
- Graceful shutdown on signals

## Future Enhancements

### Potential Improvements

1. **Multi-system monitoring**
   - Network-based metrics collection from remote systems
   - Central aggregation dashboard

2. **Historical data**
   - Store metrics in database
   - Trend analysis and graphing
   - Alerts on threshold breaches

3. **Advanced filtering**
   - Filter processes by name/regex
   - Custom metric calculations
   - Process group aggregation

4. **Authentication/Authorization**
   - OAuth integration
   - Role-based access control
   - Audit logging

5. **Performance optimization**
   - Cache metric parsing results
   - Reduce /proc scanning frequency
   - Batch client updates

---

**Document Version**: 1.0
**Last Updated**: 2024
