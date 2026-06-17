# Security Policy

## Supported Versions

| Version | Supported          |
|---------|-------------------|
| 1.x     | :white_check_mark: |
| < 1.0   | :x:               |

## Reporting a Vulnerability

**Do not open a GitHub issue for security vulnerabilities.**

Please email security concerns to: [security@example.com](mailto:security@example.com)

Include:
- Description of vulnerability
- Steps to reproduce
- Potential impact
- Suggested fix (if any)

You will receive an acknowledgment within 48 hours.

## Security Best Practices

### For Users

1. **Keep Updated**
   - Run on supported Ubuntu versions (22.04 LTS+)
   - Apply security patches regularly
   - Monitor for updates

2. **Network Security**
   - Use HTTPS/WSS in production (via reverse proxy)
   - Restrict access to trusted networks
   - Use firewall rules to limit port access
   - Run behind a reverse proxy (Nginx, Apache)

3. **Process Isolation**
   - Run as unprivileged user (never as root)
   - Use container/VM isolation in production
   - Apply OS-level security controls (AppArmor, SELinux)

4. **Monitoring**
   - Monitor application logs
   - Set up alerts for errors
   - Track resource usage anomalies

### For Developers

1. **Code Review**
   - Security-focused code review process
   - Regular dependency updates
   - Static analysis checks

2. **Dependencies**
   - Minimal external dependencies (only OpenSSL, Threads)
   - Keep dependencies updated
   - Monitor for vulnerability advisories

3. **Input Validation**
   - Validate all configuration inputs
   - Proper error handling and bounds checking
   - JSON escaping for output

4. **Memory Safety**
   - C++17 with modern practices
   - Address Sanitizer testing
   - Valgrind memory checks

## Known Security Considerations

### /proc Filesystem Access

The application reads from Linux `/proc` filesystem:
- Accessible only to root and processes running as root/owner
- Contains process-level information (typically world-readable)
- No sensitive data written by this application

**Mitigation**: Run as unprivileged user; OS permissions will restrict access to appropriate data.

### WebSocket Security

- No authentication mechanism built-in
- Suitable for local/internal networks
- For public exposure, use reverse proxy with authentication

**Recommended**: Deploy behind Nginx with basic auth, OAuth, or mTLS.

### Information Disclosure

- Metrics are sent to all connected clients
- Process names and resource usage are visible
- No encryption of WebSocket traffic by default

**Mitigation**: Use WSS (WebSocket Secure) via reverse proxy.

## Security Checklist for Production

- [ ] Run as non-root user
- [ ] Use reverse proxy (Nginx/Apache)
- [ ] Enable HTTPS/WSS with valid certificates
- [ ] Restrict network access (firewall)
- [ ] Set up monitoring and alerting
- [ ] Configure log rotation
- [ ] Regular backups enabled
- [ ] Security patches applied
- [ ] Container scanning enabled (if using Docker)
- [ ] Network policy configured (if using Kubernetes)

## Third-Party Security Scanning

The project uses:

- **GitHub Security Scanning**: Dependency vulnerability checks
- **Trivy**: Container image vulnerability scanning
- **Static Analysis**: In CI/CD pipeline

## Compliance

This application:
- Does not collect user data
- Does not store sensitive information
- Does not connect to external services
- Runs entirely on-premise

## Changelog

### Version 1.0.0
- Initial release
- Security audit completed
- All known vulnerabilities addressed

---

**Last Updated**: 2024
**Policy Version**: 1.0
