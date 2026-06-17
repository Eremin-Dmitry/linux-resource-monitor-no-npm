/**
 * @file Config.hpp
 * @brief Configuration management for Resource Monitor Application
 *
 * This file defines the configuration system for the Resource Monitor,
 * including settings for server port, update intervals, and logging.
 */

#pragma once

#include <cstdint>
#include <string>

/**
 * @enum LogLevel
 * @brief Defines logging severity levels
 */
enum class LogLevel
{
    Trace,     ///< Detailed trace information
    Debug,     ///< Debug-level messages
    Info,      ///< Informational messages
    Warn,      ///< Warning messages
    Error,     ///< Error messages
    Critical   ///< Critical error messages
};

/**
 * @struct ResourceMonitorConfig
 * @brief Configuration for Resource Monitor Application
 *
 * Encapsulates all configuration parameters needed for the application,
 * loaded from environment variables or defaults.
 */
struct ResourceMonitorConfig
{
    /// WebSocket server listening port (default: 9002)
    std::uint16_t serverPort{9002};

    /// Update interval in milliseconds (default: 1000)
    std::uint32_t updateIntervalMs{1000};

    /// Number of top processes to report (default: 25)
    std::size_t topProcessesCount{25};

    /// Logging level (default: Info)
    LogLevel logLevel{LogLevel::Info};

    /// Path to log file (default: "logs/resource-monitor.log")
    std::string logFilePath{"logs/resource-monitor.log"};

    /// Enable console logging (default: true)
    bool logToConsole{true};

    /// Enable file logging (default: true)
    bool logToFile{true};

    /**
     * @brief Load configuration from environment variables
     *
     * Attempts to load configuration from environment variables.
     * Falls back to defaults for any unset variables.
     *
     * @return ResourceMonitorConfig Configuration object with loaded values
     *
     * @example
     * @code
     * auto config = ResourceMonitorConfig::LoadFromEnvironment();
     * @endcode
     */
    [[nodiscard]] static ResourceMonitorConfig LoadFromEnvironment();

    /**
     * @brief Convert LogLevel enum to string
     *
     * @param level The log level to convert
     * @return std::string String representation of the log level
     */
    [[nodiscard]] static std::string LogLevelToString(LogLevel level);

    /**
     * @brief Convert string to LogLevel enum
     *
     * @param levelStr String representation of log level
     * @return LogLevel The corresponding enum value
     * @throws std::invalid_argument if levelStr is not a valid log level
     */
    [[nodiscard]] static LogLevel StringToLogLevel(const std::string& levelStr);
};
