/**
 * @file Logger.cpp
 * @brief Implementation of logging system
 */

#include "Logger.hpp"
#include <filesystem>

Logger& Logger::GetInstance()
{
    static Logger instance;
    return instance;
}

void Logger::Initialize(const ResourceMonitorConfig& config)
{
    auto& logger = GetInstance();
    std::lock_guard<std::mutex> lock(logger.m_mutex);

    logger.m_minLogLevel = config.logLevel;
    logger.m_logToConsole = config.logToConsole;
    logger.m_logToFile = config.logToFile;

    if (config.logToFile)
    {
        // Create logs directory if it doesn't exist
        try
        {
            std::filesystem::path logPath(config.logFilePath);
            std::filesystem::create_directories(logPath.parent_path());
            logger.m_fileStream.open(config.logFilePath, std::ios::app);

            if (!logger.m_fileStream.is_open())
            {
                throw std::runtime_error("Failed to open log file: " + config.logFilePath);
            }
        }
        catch (const std::exception& e)
        {
            throw std::runtime_error(std::string("Failed to initialize file logging: ") + e.what());
        }
    }

    logger.m_initialized = true;
}

Logger::~Logger()
{
    if (m_fileStream.is_open())
    {
        m_fileStream.close();
    }
}

void Logger::Trace(const std::string& message)
{
    GetInstance().LogImpl(LogLevel::Trace, message);
}

void Logger::Debug(const std::string& message)
{
    GetInstance().LogImpl(LogLevel::Debug, message);
}

void Logger::Info(const std::string& message)
{
    GetInstance().LogImpl(LogLevel::Info, message);
}

void Logger::Warn(const std::string& message)
{
    GetInstance().LogImpl(LogLevel::Warn, message);
}

void Logger::Error(const std::string& message)
{
    GetInstance().LogImpl(LogLevel::Error, message);
}

void Logger::Critical(const std::string& message)
{
    GetInstance().LogImpl(LogLevel::Critical, message);
}

void Logger::Flush()
{
    auto& logger = GetInstance();
    std::lock_guard<std::mutex> lock(logger.m_mutex);

    if (logger.m_logToConsole)
    {
        std::cout.flush();
    }

    if (logger.m_fileStream.is_open())
    {
        logger.m_fileStream.flush();
    }
}

void Logger::LogImpl(LogLevel level, const std::string& message)
{
    if (level < m_minLogLevel)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_initialized)
    {
        return;
    }

    std::string formatted = FormatMessage(level, message);

    if (m_logToConsole)
    {
        std::cout << formatted << '\n';
    }

    if (m_logToFile && m_fileStream.is_open())
    {
        m_fileStream << formatted << '\n';
    }
}

std::string Logger::GetTimestamp()
{
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::FormatMessage(LogLevel level, const std::string& message)
{
    std::ostringstream oss;
    oss << "[" << GetTimestamp() << "] "
        << "[" << ResourceMonitorConfig::LogLevelToString(level) << "] "
        << message;
    return oss.str();
}
