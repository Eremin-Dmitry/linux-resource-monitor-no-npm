# Deployment Guide

This guide covers deploying Linux Resource Monitor in production.

## Docker Deployment (Recommended)

### Using docker-compose

```bash
./scripts/deploy-docker.sh start
```

This starts both the backend service and an Nginx reverse proxy.

**Access points:**
- Frontend: http://localhost
- WebSocket: ws://localhost/ws
- Backend direct: ws://localhost:9002/ws

### Manual Docker Commands

```bash
# Build image
docker build -t linux-resource-monitor:latest .

# Run container
docker run -d \
  --name resource-monitor \
  -p 9002:9002 \
  -v /proc:/proc:ro \
  -v /sys:/sys:ro \
  -e RESOURCE_MONITOR_LOG_LEVEL=INFO \
  linux-resource-monitor:latest

# View logs
docker logs -f resource-monitor

# Stop container
docker stop resource-monitor
docker rm resource-monitor
```

### Docker Resource Limits

The `docker-compose.yml` includes resource limits:

```yaml
deploy:
  resources:
    limits:
      cpus: '1'
      memory: 256M
    reservations:
      cpus: '0.5'
      memory: 128M
```

Adjust based on your environment.

## Bare Metal Deployment

### 1. Build from Source

```bash
./scripts/build.sh release
```

### 2. Create Service Directory

```bash
sudo mkdir -p /opt/resource-monitor
sudo cp backend/build/resource-monitor /opt/resource-monitor/
sudo cp -r frontend /opt/resource-monitor/
sudo mkdir -p /var/log/resource-monitor
sudo chown -R nobody:nogroup /opt/resource-monitor
sudo chown -R nobody:nogroup /var/log/resource-monitor
```

### 3. Create systemd Service

Create `/etc/systemd/system/resource-monitor.service`:

```ini
[Unit]
Description=Linux Resource Monitor
Documentation=https://github.com/yourusername/linux-resource-monitor
After=network.target

[Service]
Type=simple
User=nobody
Group=nogroup
WorkingDirectory=/opt/resource-monitor

# Configuration
Environment="RESOURCE_MONITOR_PORT=9002"
Environment="RESOURCE_MONITOR_LOG_LEVEL=INFO"
Environment="RESOURCE_MONITOR_LOG_FILE=/var/log/resource-monitor/resource-monitor.log"

ExecStart=/opt/resource-monitor/resource-monitor

# Auto-restart on failure
Restart=on-failure
RestartSec=10
StartLimitInterval=300
StartLimitBurst=5

# Security
NoNewPrivileges=true
ProtectSystem=strict
ProtectHome=true
PrivateTmp=true
ReadWritePaths=/var/log/resource-monitor

# Process tracking
KillMode=mixed
KillSignal=SIGTERM
TimeoutStopSec=30

[Install]
WantedBy=multi-user.target
```

### 4. Enable and Start Service

```bash
sudo systemctl daemon-reload
sudo systemctl enable resource-monitor
sudo systemctl start resource-monitor

# Check status
sudo systemctl status resource-monitor
sudo journalctl -u resource-monitor -f
```

## Nginx Reverse Proxy Setup

### Installation

```bash
sudo apt-get install nginx
```

### Configuration

Create `/etc/nginx/sites-available/resource-monitor`:

```nginx
upstream resource_monitor_backend {
    server localhost:9002;
}

server {
    listen 80;
    listen [::]:80;
    server_name monitor.example.com;

    # Redirect to HTTPS (optional)
    return 301 https://$server_name$request_uri;
}

server {
    listen 443 ssl http2;
    listen [::]:443 ssl http2;
    server_name monitor.example.com;

    ssl_certificate /etc/letsencrypt/live/monitor.example.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/monitor.example.com/privkey.pem;

    # Security headers
    add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-XSS-Protection "1; mode=block" always;

    # Serve static files
    location / {
        root /opt/resource-monitor/frontend;
        try_files $uri $uri/ /index.html;
        expires 1d;
        add_header Cache-Control "public, immutable";
    }

    # WebSocket proxy
    location /ws {
        proxy_pass http://resource_monitor_backend/ws;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_read_timeout 86400;
    }

    # Health check
    location /health {
        access_log off;
        return 200 "healthy\n";
    }
}
```

Enable the site:

```bash
sudo ln -s /etc/nginx/sites-available/resource-monitor /etc/nginx/sites-enabled/
sudo nginx -t
sudo systemctl restart nginx
```

## SSL/TLS with Let's Encrypt

```bash
sudo apt-get install certbot python3-certbot-nginx

# Obtain certificate
sudo certbot certonly --nginx -d monitor.example.com

# Auto-renewal (usually automatic)
sudo systemctl enable certbot.timer
sudo systemctl start certbot.timer
```

## Kubernetes Deployment

### Create Deployment

```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: resource-monitor-config
data:
  RESOURCE_MONITOR_LOG_LEVEL: "INFO"
  RESOURCE_MONITOR_TOP_PROCESSES_COUNT: "25"

---
apiVersion: apps/v1
kind: Deployment
metadata:
  name: resource-monitor
spec:
  replicas: 1
  selector:
    matchLabels:
      app: resource-monitor
  template:
    metadata:
      labels:
        app: resource-monitor
    spec:
      containers:
      - name: resource-monitor
        image: linux-resource-monitor:latest
        imagePullPolicy: Always
        ports:
        - containerPort: 9002
          name: websocket
        envFrom:
        - configMapRef:
            name: resource-monitor-config
        resources:
          requests:
            memory: "128Mi"
            cpu: "500m"
          limits:
            memory: "256Mi"
            cpu: "1000m"
        volumeMounts:
        - name: proc
          mountPath: /proc
          readOnly: true
        - name: sys
          mountPath: /sys
          readOnly: true
        - name: logs
          mountPath: /app/logs
        livenessProbe:
          tcpSocket:
            port: 9002
          initialDelaySeconds: 5
          periodSeconds: 10
        readinessProbe:
          tcpSocket:
            port: 9002
          initialDelaySeconds: 2
          periodSeconds: 5
      volumes:
      - name: proc
        hostPath:
          path: /proc
      - name: sys
        hostPath:
          path: /sys
      - name: logs
        emptyDir: {}

---
apiVersion: v1
kind: Service
metadata:
  name: resource-monitor
spec:
  selector:
    app: resource-monitor
  ports:
  - port: 9002
    targetPort: 9002
    name: websocket
  type: ClusterIP
```

Deploy:

```bash
kubectl apply -f deployment.yaml
kubectl port-forward svc/resource-monitor 9002:9002
```

## Monitoring and Logging

### View Logs

**Docker:**
```bash
./scripts/deploy-docker.sh logs
```

**Systemd:**
```bash
sudo journalctl -u resource-monitor -f
```

**File:**
```bash
tail -f logs/resource-monitor.log
```

### Log Rotation

For systemd service, create `/etc/logrotate.d/resource-monitor`:

```
/var/log/resource-monitor/*.log {
    daily
    rotate 14
    compress
    delaycompress
    notifempty
    create 0644 nobody nogroup
    sharedscripts
    postrotate
        systemctl reload resource-monitor > /dev/null 2>&1 || true
    endscript
}
```

### Performance Monitoring

Monitor resource usage:

```bash
# Docker
docker stats resource-monitor

# Systemd
systemctl status resource-monitor
```

## Backup and Recovery

### Backup Configuration

```bash
tar -czf resource-monitor-backup.tar.gz \
  /opt/resource-monitor \
  /etc/systemd/system/resource-monitor.service \
  /var/log/resource-monitor
```

### Restore from Backup

```bash
tar -xzf resource-monitor-backup.tar.gz -C /
sudo systemctl daemon-reload
sudo systemctl restart resource-monitor
```

## Troubleshooting

### Port Already in Use

```bash
lsof -i :9002
# Kill the process if needed
kill -9 <PID>
```

### Permission Denied Errors

Ensure the service has access to `/proc`:

```bash
# For Docker: /proc is already mounted
# For systemd: add to [Service] section:
# ProtectSystem=off or adjust ReadWritePaths
```

### High Memory Usage

- Reduce `RESOURCE_MONITOR_TOP_PROCESSES_COUNT`
- Increase `RESOURCE_MONITOR_UPDATE_INTERVAL_MS`
- Check logs for errors

### WebSocket Connection Refused

- Verify firewall rules allow port 9002
- Check if service is running: `systemctl status resource-monitor`
- Verify configuration is correct

## Security Considerations

1. **Network Security**
   - Use HTTPS/WSS in production
   - Restrict access to localhost or internal network
   - Use reverse proxy (Nginx) for public exposure

2. **Process Isolation**
   - Run as unprivileged user (done in templates above)
   - Use AppArmor or SELinux profiles
   - Restrict filesystem access

3. **Secrets Management**
   - Use environment files (not in git)
   - Rotate logs regularly
   - Monitor for sensitive data leaks

4. **Updates**
   - Monitor for security patches
   - Regular testing of updates in staging
   - Automated update pipelines

See [Security Policy](SECURITY.md) for more details.

## Next Steps

- Set up monitoring and alerting
- Configure automated backups
- Implement disaster recovery plan
- Document runbook for operations team
