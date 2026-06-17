# Frontend Documentation

This is the web-based dashboard for Linux Resource Monitor.

## Overview

The frontend is a zero-dependency web application that connects to the backend via WebSocket and displays system metrics in real-time.

**Technology Stack**:
- **HTML5**: Semantic markup
- **CSS3**: Responsive grid layout, flexbox
- **JavaScript (ES6+)**: WebSocket client, DOM manipulation
- **TypeScript**: Source code (optional, for development)

**Key Features**:
- ✅ No npm, Node.js, or build tools required
- ✅ No external dependencies
- ✅ Responsive design
- ✅ Real-time updates (1 second intervals)
- ✅ Connection status indicator
- ✅ Auto-reconnect on disconnection

## File Structure

```
frontend/
├── index.html           # HTML markup and structure
├── app.js              # JavaScript runtime (compiled from TypeScript)
├── styles.css          # CSS styling
└── src/
    └── main.ts         # TypeScript source (for development)
```

## Running the Frontend

### Option 1: Python Simple Server (Recommended)

```bash
cd frontend
python3 -m http.server 5173
```

Then open: http://localhost:5173

### Option 2: Python 2 (Legacy)

```bash
cd frontend
python -m SimpleHTTPServer 8000
```

Then open: http://localhost:8000

### Option 3: Node.js http-server

```bash
npm install -g http-server
cd frontend
http-server -p 5173
```

### Option 4: Direct File Open (Limited)

Open `frontend/index.html` directly in your browser. Note: WebSocket connections may fail due to CORS restrictions with `file://` protocol.

### Option 5: Docker + Nginx

```bash
docker-compose up -d
```

Frontend available at: http://localhost

## Dashboard Components

### Header Section
- **Title**: "Linux Resource Monitor"
- **Connection Status**: Shows "Connected" (green) or "Disconnected" (red)
- Auto-reconnect indicator

### System Metrics

#### CPU Usage
- Circular gauge showing percentage
- Visual indicator of system load
- Real-time updates

#### Memory
- Horizontal progress bar
- Shows: used/total in MB
- Percentage indicator

#### Swap
- Horizontal progress bar
- Shows: used/total in MB
- Percentage indicator

#### Load Average
- Three values: 1-minute, 5-minute, 15-minute averages
- Formatted display

#### Uptime
- System uptime in human-readable format
- Format: "Xd Xh Xm" (days, hours, minutes)

#### Process Table
- Top 25 processes by CPU usage
- Columns:
  - **PID**: Process identifier
  - **Name**: Process name/command
  - **State**: Process state (R=running, S=sleeping, Z=zombie)
  - **CPU%**: CPU usage percentage
  - **Memory (MB)**: Memory usage in megabytes

## Connection Handling

### WebSocket Endpoint

Default: `ws://localhost:9002/ws`

Configurable in `app.js`:

```javascript
const WEB_SOCKET_URL = "ws://localhost:9002/ws";
```

### Auto-Reconnect

- Automatically attempts reconnection on disconnect
- Reconnection interval: 1.5 seconds
- Retries indefinitely until connection succeeds

### Error Handling

- Connection errors logged to browser console
- Graceful UI updates during disconnections
- Status indicator reflects connection state

## Styling

### Responsive Design

- Mobile-first approach
- Flexbox and CSS Grid for layout
- Responsive font sizing
- Adapts to different screen sizes

### Colors

- **Primary**: Blue shades
- **Success**: Green (connected)
- **Error**: Red (disconnected)
- **Neutral**: Gray/white backgrounds

### Accessibility

- Semantic HTML structure
- Good color contrast
- Clear labels and indicators
- Keyboard navigation support

## Configuration

### Customizing Endpoint

Edit `app.js` and change:

```javascript
const WEB_SOCKET_URL = "ws://your-server:port/ws";
```

### Customizing Styles

Edit `styles.css`:

```css
/* Change primary color */
:root {
  --primary-color: #0066cc;
  --success-color: #28a745;
  --error-color: #dc3545;
}
```

### Customizing Refresh Interval

Note: The refresh interval is controlled by the backend (see `RESOURCE_MONITOR_UPDATE_INTERVAL_MS`). The frontend simply renders updates as they arrive.

## Development

### TypeScript Source

The TypeScript source code is in `src/main.ts`. For development:

```bash
# Install TypeScript (optional, for IDE support)
npm install --save-dev typescript

# Compile TypeScript to JavaScript
npx tsc src/main.ts --target ES2020 --module ES2020 --outDir .
```

### Browser DevTools

Open browser DevTools (F12):

- **Console**: Check for WebSocket connection messages
- **Network**: Monitor WebSocket frames
- **Elements**: Inspect DOM and styles
- **Application**: Check local storage/cache

### Debug Logging

The frontend logs connection events to console:

```javascript
// In browser console:
// Look for messages like:
// "Attempting to connect to WebSocket..."
// "WebSocket connected"
// "Received metrics: ..."
```

## Performance Considerations

### Network

- **Payload Size**: ~2-5 KB per update
- **Frequency**: 1 per second by default
- **Total Bandwidth**: ~7-18 MB per hour

### Rendering

- DOM updates are efficient (only changed elements)
- CSS animations are GPU-accelerated
- No memory leaks from event listeners (proper cleanup)

### Browser Compatibility

- **Chrome/Edge**: Full support
- **Firefox**: Full support
- **Safari**: Full support
- **IE11**: Not supported (requires ES6+ features)

## Troubleshooting

### WebSocket Connection Fails

**Problem**: "WebSocket is closed before the connection is established"

**Solutions**:
1. Ensure backend is running: `./backend/build/resource-monitor`
2. Check port is correct in `app.js` (default: 9002)
3. Check firewall allows port 9002
4. Try opening with localhost instead of 127.0.0.1

### Metrics Not Updating

**Problem**: Dashboard shows data but not updating

**Solutions**:
1. Check browser console for errors
2. Verify WebSocket connection is active (DevTools → Network)
3. Check backend logs for errors
4. Restart both frontend and backend

### Page Loads But Shows Disconnected

**Problem**: Browser shows "Disconnected" status

**Solutions**:
1. Backend may not be running
2. WebSocket port may be blocked by firewall
3. Check browser console for error messages
4. Verify network connectivity

### Styling Issues

**Problem**: Dashboard looks broken or misaligned

**Solutions**:
1. Clear browser cache (Ctrl+Shift+Delete)
2. Disable browser extensions (especially ad blockers)
3. Try different browser
4. Check that `styles.css` is loaded (DevTools → Network)

## Customization

### Change Dashboard Title

Edit `index.html`:

```html
<h1>My Resource Monitor</h1>
```

### Add New Metrics Display

Edit `app.js` in `renderResourceMetrics()` function:

```javascript
dashboardElements.newMetric.textContent = metrics.yourNewField;
```

### Modify Color Scheme

Edit `styles.css`:

```css
:root {
  --primary-color: #your-color;
  --text-color: #your-color;
}
```

## Security Notes

### No Data Encryption (By Default)

WebSocket connection is in plaintext by default. For production:

1. Use WSS (WebSocket Secure) via HTTPS
2. Deploy behind Nginx with SSL/TLS
3. Use authentication/authorization

### No Authentication

Frontend has no built-in authentication. For production:

1. Add OAuth via reverse proxy
2. Use basic auth
3. Implement session tokens

See [SECURITY.md](../SECURITY.md) for more details.

## Browser Compatibility Matrix

| Browser | Version | Status |
|---------|---------|--------|
| Chrome | 60+ | ✅ Full Support |
| Firefox | 55+ | ✅ Full Support |
| Safari | 11+ | ✅ Full Support |
| Edge | 79+ | ✅ Full Support |
| IE | Any | ❌ Not Supported |

## API Reference

### WebSocket Message Format

**Incoming (from backend)**:

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

### JavaScript Functions

#### Main Functions

```javascript
// Connect to WebSocket
function connectWebSocket()

// Handle incoming messages
function handleWebSocketMessage(event)

// Disconnect from WebSocket
function disconnectWebSocket()

// Update connection status UI
function setConnectionStatus(isConnected)

// Render metrics on dashboard
function renderResourceMetrics(metrics)

// Render process table
function renderProcessTable(processes)
```

## Next Steps

- See [README.md](../README.md) for overall project documentation
- See [BUILD.md](../BUILD.md) for backend build instructions
- See [DEPLOYMENT.md](../DEPLOYMENT.md) for production deployment

---

**Version**: 1.0.0  
**Last Updated**: 2024
