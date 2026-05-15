#include "ProcessResourceMonitor.hpp"
#include "ResourceWebSocketServer.hpp"
#include "SystemResourceMonitor.hpp"

#include <exception>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

namespace
{
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

int main(int argc, char** argv)
{
    std::uint16_t serverPort = 9002;

    if (argc > 1)
    {
        serverPort = static_cast<std::uint16_t>(std::stoi(argv[1]));
    }

    SystemResourceMonitor systemResourceMonitor;
    ProcessResourceMonitor processResourceMonitor;
    ResourceWebSocketServer webSocketServer(serverPort);
    std::mutex resourceMonitorMutex;

    try
    {
        webSocketServer.Run([&]()
        {
            std::lock_guard<std::mutex> lock(resourceMonitorMutex);

            const auto systemMetrics = systemResourceMonitor.CollectMetrics();
            const auto topProcesses = processResourceMonitor.CollectTopProcesses(25);

            return BuildMetricsJson(systemMetrics, topProcesses);
        });
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }

    return 0;
}
