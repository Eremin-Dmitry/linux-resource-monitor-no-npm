# Linux Resource Monitor

A lightweight Linux resource monitoring application with a C++ backend and a WebUI frontend built with HTML, CSS and TypeScript source code.

The application is similar in spirit to `top`/`htop`: it displays CPU load, memory usage, swap usage, load average, uptime and a table of active processes. Data is updated dynamically once per second.

## Features

- C++17 backend for Linux
- WebUI frontend: HTML + CSS + JavaScript runtime, TypeScript source included
- No npm install is required to run the frontend
- WebSocket-based IPC between backend and frontend
- CPU usage calculation based on `/proc/stat` deltas
- Memory and swap statistics from `/proc/meminfo`
- Load average from `/proc/loadavg`
- Uptime from `/proc/uptime`
- Process list from `/proc/[pid]/stat` and `/proc/[pid]/status`
- CMake-based backend build

## Architecture

```text
+----------------------+        WebSocket JSON        +----------------------+
| C++ backend          |  -------------------------->  | WebUI frontend       |
|                      |                              |                      |
| Reads Linux /proc    |                              | HTML/CSS/JS          |
| Builds metrics JSON  |                              | Dynamic dashboard    |
+----------------------+                              +----------------------+
```

The backend is responsible for collecting system metrics from Linux virtual files under `/proc`. It exposes a minimal WebSocket server on port `9002` and periodically sends JSON snapshots to connected frontend clients.

The frontend connects to `ws://localhost:9002/ws`, receives JSON payloads and updates the UI without page reloads.

## Project structure

```text
linux-resource-monitor/
├── backend/
│   ├── CMakeLists.txt
│   └── src/
│       ├── ResourceMonitorApplication.cpp
│       ├── SystemResourceMonitor.cpp
│       ├── SystemResourceMonitor.hpp
│       ├── ProcessResourceMonitor.cpp
│       ├── ProcessResourceMonitor.hpp
│       ├── ResourceWebSocketServer.cpp
│       └── ResourceWebSocketServer.hpp
├── frontend/
│   ├── index.html
│   ├── styles.css
│   ├── app.js
│   └── src/
│       └── main.ts
├── .gitignore
└── README.md
```

## Requirements

Target environment: Ubuntu 22.04.5 LTS.

Install backend build dependencies:

```bash
sudo apt update
sudo apt install -y build-essential cmake libssl-dev
```

`nodejs`, `npm`, `vite` and external frontend packages are not required.

## Build and run backend

```bash
cd backend
cmake -S . -B build
cmake --build build
./build/resource-monitor
```

By default, the backend listens on:

```text
ws://localhost:9002/ws
```

A custom port can be passed as the first argument:

```bash
./build/resource-monitor 9010
```

If a custom port is used, update `WEB_SOCKET_URL` in `frontend/app.js`.

## Run frontend without npm

Open another terminal:

```bash
cd frontend
python3 -m http.server 5173
```

Then open in a browser:

```text
http://localhost:5173
```

You can also open `frontend/index.html` directly in the browser, but using a small local static server is more convenient and closer to a real launch scenario.

## TypeScript note

The file `frontend/src/main.ts` is kept as the readable TypeScript source. The browser uses the already prepared `frontend/app.js`, so the reviewer does not need to install npm packages or run a TypeScript build step.

This is intentional: the project remains compatible with restricted environments where access to npm registry is unavailable.

## Data format

Example backend payload:

```json
{
  "cpu": {
    "usage": 37.42
  },
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

## Implementation notes

### CPU usage

CPU usage is calculated by reading two consecutive snapshots from `/proc/stat` and comparing the total and idle CPU times. This is necessary because CPU usage is not stored directly as a ready-to-use percentage.

### Process CPU usage

Per-process CPU usage is calculated from the difference between process CPU ticks and total system CPU ticks between two samples.

### IPC choice

WebSocket was selected as the IPC mechanism because the monitoring data changes continuously and the frontend needs to receive updates without manual polling. It also keeps the backend and frontend loosely coupled: the backend only sends JSON snapshots, while the frontend is responsible for presentation.

## Limitations and possible improvements

- Add process filtering and sorting in UI
- Add CPU history chart
- Add network and disk I/O metrics
- Add unit tests for parsing `/proc` files
- Replace the minimal built-in WebSocket implementation with a production-ready library if the project grows
- Serve static frontend files directly from the C++ backend

## AI usage note

AI tools may be used for routine assistance such as README structuring, formatting checks and refactoring suggestions. The architecture, Linux `/proc` data collection logic and backend/frontend integration are expected to be understood and explainable by the author.
