/**
 * @file Config.cpp
 * @brief Implementation of configuration management
 */

#include "Config.hpp"
#include <cstdlib>
#include <algorithm>
#include <stdexcept>

ResourceMonitorConfig ResourceMonitorConfig::LoadFromEnvironment()
{
    ResourceMonitorConfig config;

    // Load server port
    if (const char* portStr = std::getenv("RESOURCE_MONITOR_PORT"))
    {
        try
        {
            config.serverPort = static_cast<std::uint16_t>(std::stoi(portStr));
        }
        catch (const std::exception&)
        {
            // Keep default value on parse error
        }
    }

    // Load update interval
    if (const char* intervalStr = std::getenv("RESOURCE_MONITOR_UPDATE_INTERVAL_MS"))
    {
        try
        {
            config.updateIntervalMs = static_cast<std::uint32_t>(std::stoi(intervalStr));
        }
        catch (const std::exception&)
        {
            // Keep default value on parse error
        }
    }

    // Load top processes count
    if (const char* countStr = std::getenv("RESOURCE_MONITOR_TOP_PROCESSES_COUNT"))
    {
        try
        {
            config.topProcessesCount = static_cast<std::size_t>(std::stoi(countStr));
        }
        catch (const std::exception&)
        {
            // Keep default value on parse error
        }
    }

    // Load log level
    if (const char* levelStr = std::getenv("RESOURCE_MONITOR_LOG_LEVEL"))
    {
        try
        {
            config.logLevel = StringToLogLevel(levelStr);
        }
        catch (const std::exception&)
        {
            // Keep default value on parse error
        }
    }

    // Load log file path
    if (const char* pathStr = std::getenv("RESOURCE_MONITOR_LOG_FILE"))
    {
        config.logFilePath = pathStr;
    }

    // Load console logging flag
    if (const char* consoleStr = std::getenv("RESOURCE_MONITOR_LOG_TO_CONSOLE"))
    {
        std::string value(consoleStr);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        config.logToConsole = (value == "true" || value == "1" || value == "yes");
    }

    // Load file logging flag
    if (const char* fileStr = std::getenv("RESOURCE_MONITOR_LOG_TO_FILE"))
    {
        std::string value(fileStr);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        config.logToFile = (value == "true" || value == "1" || value == "yes");
    }

    return config;
}

std::string ResourceMonitorConfig::LogLevelToString(LogLevel level)
{
    switch (level)
    {
        case LogLevel::Trace:
            return "TRACE";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        case LogLevel::Critical:
            return "CRITICAL";
        default:
            return "UNKNOWN";
    }
}

LogLevel ResourceMonitorConfig::StringToLogLevel(const std::string& levelStr)
{
    std::string level(levelStr);
    std::transform(level.begin(), level.end(), level.begin(), ::toupper);

    if (level == "TRACE")
        return LogLevel::Trace;
    if (level == "DEBUG")
        return LogLevel::Debug;
    if (level == "INFO")
        return LogLevel::Info;
    if (level == "WARN")
        return LogLevel::Warn;
    if (level == "ERROR")
        return LogLevel::Error;
    if (level == "CRITICAL")
        return LogLevel::Critical;

    throw std::invalid_argument("Invalid log level: " + levelStr);
}
