#include "ProcessResourceMonitor.hpp"
#include "ResourceWebSocketServer.hpp"
#include "SystemResourceMonitor.hpp"
#include "Config.hpp"
#include "Logger.hpp"

#include <exception>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace
{
/**
 * @brief Escape special characters in JSON strings
 *
 * Converts special characters to their JSON-escaped equivalents
 * (e.g., quotes to \", newlines to \n, etc.)
 *
 * @param sourceText The source string to escape
 * @return std::string JSON-escaped string
 */
std::string EscapeJsonString(const std::string& sourceText)
{
    std::ostringstream escapedText;

    for (const char character : sourceText)
    {
        switch (character)
        {
            case '"':
                escapedText << "\\\"";
                break;
            case '\\':
                escapedText << "\\\\";
                break;
            case '\b':
                escapedText << "\\b";
                break;
            case '\f':
                escapedText << "\\f";
                break;
            case '\n':
                escapedText << "\\n";
                break;
            case '\r':
                escapedText << "\\r";
                break;
            case '\t':
                escapedText << "\\t";
                break;
            default:
                escapedText << character;
                break;
        }
    }

    return escapedText.str();
}

/**
 * @brief Build JSON representation of system metrics
 *
 * Constructs a JSON object containing CPU, memory, swap, load average,
 * uptime, and process information. All string values are properly escaped.
 *
 * @param systemMetrics System resource metrics to serialize
 * @param processes Process information to serialize
 * @return std::string JSON representation of metrics
 *
 * @example
 * @code
 * auto json = BuildMetricsJson(metrics, processes);
 * // Returns: {"cpu":{"usage":45.3},"memory":{...},...}
 * @endcode
 */
std::string BuildMetricsJson(
    const SystemResourceMetrics& systemMetrics,
    const std::vector<ProcessResourceInfo>& processes)
{
    std::ostringstream jsonStream;
    jsonStream << std::fixed << std::setprecision(2);

    jsonStream << "{";
    jsonStream << "\"cpu\":{\"usage\":" << systemMetrics.cpuUsagePercent << "},";
    jsonStream << "\"memory\":{\"total\":" << systemMetrics.memoryUsage.totalMegabytes
               << ",\"used\":" << systemMetrics.memoryUsage.usedMegabytes
               << ",\"usage\":" << systemMetrics.memoryUsage.usagePercent << "},";
    jsonStream << "\"swap\":{\"total\":" << systemMetrics.swapUsage.totalMegabytes
               << ",\"used\":" << systemMetrics.swapUsage.usedMegabytes
               << ",\"usage\":" << systemMetrics.swapUsage.usagePercent << "},";
    jsonStream << "\"loadAverage\":[" << systemMetrics.loadAverage[0] << ","
               << systemMetrics.loadAverage[1] << ","
               << systemMetrics.loadAverage[2] << "],";
    jsonStream << "\"uptime\":" << systemMetrics.uptimeSeconds << ",";
    jsonStream << "\"processes\":[";

    for (std::size_t processIndex = 0; processIndex < processes.size(); ++processIndex)
    {
        const auto& processInfo = processes[processIndex];

        jsonStream << "{";
        jsonStream << "\"pid\":" << processInfo.pid << ",";
        jsonStream << "\"name\":\"" << EscapeJsonString(processInfo.name) << "\",";
        jsonStream << "\"state\":\"" << processInfo.state << "\",";
        jsonStream << "\"cpu\":" << processInfo.cpuUsagePercent << ",";
        jsonStream << "\"memory\":" << processInfo.memoryMegabytes;
        jsonStream << "}";

        if (processIndex + 1 < processes.size())
        {
            jsonStream << ",";
        }
    }

    jsonStream << "]}";
    return jsonStream.str();
}
}

/**
 * @brief Application entry point
 *
 * Initializes the Resource Monitor application with configuration from
 * environment variables, sets up logging, and starts the WebSocket server
 * for serving system metrics to connected clients.
 *
 * The server periodically collects system and process metrics and broadcasts
 * them to all connected WebSocket clients as JSON payloads.
 *
 * Usage:
 *   resource-monitor [port]
 *
 * Environment variables:
 *   - RESOURCE_MONITOR_PORT: WebSocket server port (default: 9002)
 *   - RESOURCE_MONITOR_UPDATE_INTERVAL_MS: Update interval in ms (default: 1000)
 *   - RESOURCE_MONITOR_TOP_PROCESSES_COUNT: Number of processes to report (default: 25)
 *   - RESOURCE_MONITOR_LOG_LEVEL: Log level: TRACE|DEBUG|INFO|WARN|ERROR|CRITICAL (default: INFO)
 *   - RESOURCE_MONITOR_LOG_FILE: Path to log file (default: logs/resource-monitor.log)
 *   - RESOURCE_MONITOR_LOG_TO_CONSOLE: Enable console logging (default: true)
 *   - RESOURCE_MONITOR_LOG_TO_FILE: Enable file logging (default: true)
 *
 * @param argc Argument count
 * @param argv Argument vector (optional port override)
 * @return int Exit code (0 on success, 1 on error)
 *
 * @throws std::exception Caught and logged for any runtime errors
 */
int main(int argc, char** argv)
{
    try
    {
        // Load configuration from environment
        auto config = ResourceMonitorConfig::LoadFromEnvironment();

        // Override port if provided as command-line argument
        if (argc > 1)
        {
            try
            {
                config.serverPort = static_cast<std::uint16_t>(std::stoi(argv[1]));
            }
            catch (const std::exception& e)
            {
                std::cerr << "Invalid port number: " << argv[1] << '\n';
                return 1;
            }
        }

        // Initialize logging system
        Logger::Initialize(config);

        Logger::Info("Resource Monitor starting...");
        Logger::Info("Configuration: port=" + std::to_string(config.serverPort) +
                     " updateInterval=" + std::to_string(config.updateIntervalMs) + "ms" +
                     " topProcesses=" + std::to_string(config.topProcessesCount));

        // Create resource monitors
        SystemResourceMonitor systemResourceMonitor;
        ProcessResourceMonitor processResourceMonitor;
        ResourceWebSocketServer webSocketServer(config.serverPort);
        std::mutex resourceMonitorMutex;

        Logger::Info("Starting WebSocket server on port " + std::to_string(config.serverPort));

        // Run the server with metric collection callback
        webSocketServer.Run([&]()
        {
            std::lock_guard<std::mutex> lock(resourceMonitorMutex);

            try
            {
                const auto systemMetrics = systemResourceMonitor.CollectMetrics();
                const auto topProcesses = processResourceMonitor.CollectTopProcesses(
                    config.topProcessesCount);

                return BuildMetricsJson(systemMetrics, topProcesses);
            }
            catch (const std::exception& e)
            {
                Logger::Error("Error collecting metrics: " + std::string(e.what()));
                // Return empty metrics on error instead of crashing
                return std::string("{\"error\":\"Failed to collect metrics\"}");
            }
        });

        Logger::Info("Resource Monitor stopped");
        Logger::Flush();
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Fatal error: " << exception.what() << '\n';
        Logger::Critical("Fatal error: " + std::string(exception.what()));
        Logger::Flush();
        return 1;
    }

    return 0;
}
