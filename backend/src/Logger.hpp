/**
 * @file Logger.hpp
 * @brief Simple logging system for Resource Monitor
 *
 * Provides a singleton logger with support for console and file output,
 * multiple log levels, and thread-safe logging.
 */

#pragma once

#include "Config.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <mutex>
#include <ctime>
#include <iomanip>

/**
 * @class Logger
 * @brief Singleton logger for the application
 *
 * Provides thread-safe logging to console and/or file.
 * Can be configured with different log levels.
 */
class Logger
{
public:
    /**
     * @brief Initialize the logger
     *
     * Must be called before any logging operations.
     *
     * @param config Configuration for logger
     * @throws std::runtime_error if file output is enabled but file cannot be created
     */
    static void Initialize(const ResourceMonitorConfig& config);

    /**
     * @brief Log a message at TRACE level
     * @param message Message to log
     */
    static void Trace(const std::string& message);

    /**
     * @brief Log a message at DEBUG level
     * @param message Message to log
     */
    static void Debug(const std::string& message);

    /**
     * @brief Log a message at INFO level
     * @param message Message to log
     */
    static void Info(const std::string& message);

    /**
     * @brief Log a message at WARN level
     * @param message Message to log
     */
    static void Warn(const std::string& message);

    /**
     * @brief Log a message at ERROR level
     * @param message Message to log
     */
    static void Error(const std::string& message);

    /**
     * @brief Log a message at CRITICAL level
     * @param message Message to log
     */
    static void Critical(const std::string& message);

    /**
     * @brief Flush all pending log output
     */
    static void Flush();

private:
    static Logger& GetInstance();

    Logger() = default;
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void LogImpl(LogLevel level, const std::string& message);

    std::mutex m_mutex;
    std::ofstream m_fileStream;
    LogLevel m_minLogLevel{LogLevel::Info};
    bool m_logToConsole{true};
    bool m_logToFile{false};
    bool m_initialized{false};

    /**
     * @brief Get formatted timestamp for log message
     * @return std::string Formatted timestamp
     */
    [[nodiscard]] static std::string GetTimestamp();

    /**
     * @brief Format log message with level and timestamp
     * @param level Log level
     * @param message Message content
     * @return std::string Formatted log message
     */
    [[nodiscard]] static std::string FormatMessage(LogLevel level, const std::string& message);
};
