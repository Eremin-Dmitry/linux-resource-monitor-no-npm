#include "SystemResourceMonitor.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

namespace
{
std::uint64_t ConvertKilobytesToMegabytes(std::uint64_t kilobytes)
{
    return kilobytes / 1024;
}

double CalculatePercent(std::uint64_t usedValue, std::uint64_t totalValue)
{
    if (totalValue == 0)
    {
        return 0.0;
    }

    return static_cast<double>(usedValue) * 100.0 / static_cast<double>(totalValue);
}

std::unordered_map<std::string, std::uint64_t> ReadMemoryInfoValues()
{
    std::ifstream memoryInfoFile("/proc/meminfo");
    std::unordered_map<std::string, std::uint64_t> memoryInfoValues;

    std::string key;
    std::uint64_t value = 0;
    std::string unit;

    while (memoryInfoFile >> key >> value >> unit)
    {
        if (!key.empty() && key.back() == ':')
        {
            key.pop_back();
        }

        memoryInfoValues[key] = value;
    }

    return memoryInfoValues;
}

std::uint64_t GetValueOrZero(const std::unordered_map<std::string, std::uint64_t>& values, const std::string& key)
{
    const auto iterator = values.find(key);
    if (iterator == values.end())
    {
        return 0;
    }

    return iterator->second;
}
}

std::uint64_t CpuTimeSample::GetIdleTime() const
{
    return idleTime + ioWaitTime;
}

std::uint64_t CpuTimeSample::GetTotalTime() const
{
    return userTime + niceTime + systemTime + idleTime + ioWaitTime + irqTime + softIrqTime + stealTime;
}

SystemResourceMonitor::SystemResourceMonitor() = default;

SystemResourceMetrics SystemResourceMonitor::CollectMetrics()
{
    SystemResourceMetrics metrics;
    const auto currentCpuTimeSample = ReadCpuTimeSample();

    metrics.cpuUsagePercent = CalculateCpuUsagePercent(currentCpuTimeSample);
    metrics.memoryUsage = ReadMemoryUsageInfo();
    metrics.swapUsage = ReadSwapUsageInfo();
    metrics.loadAverage = ReadLoadAverage();
    metrics.uptimeSeconds = ReadUptimeSeconds();

    return metrics;
}

CpuTimeSample SystemResourceMonitor::ReadCpuTimeSample()
{
    std::ifstream statFile("/proc/stat");
    std::string cpuLabel;
    CpuTimeSample cpuTimeSample;

    statFile >> cpuLabel >> cpuTimeSample.userTime >> cpuTimeSample.niceTime >> cpuTimeSample.systemTime
             >> cpuTimeSample.idleTime >> cpuTimeSample.ioWaitTime >> cpuTimeSample.irqTime
             >> cpuTimeSample.softIrqTime >> cpuTimeSample.stealTime;

    return cpuTimeSample;
}

MemoryUsageInfo SystemResourceMonitor::ReadMemoryUsageInfo()
{
    const auto memoryInfoValues = ReadMemoryInfoValues();
    const auto totalKilobytes = GetValueOrZero(memoryInfoValues, "MemTotal");
    const auto availableKilobytes = GetValueOrZero(memoryInfoValues, "MemAvailable");
    const auto usedKilobytes = totalKilobytes > availableKilobytes ? totalKilobytes - availableKilobytes : 0;

    return MemoryUsageInfo{
        ConvertKilobytesToMegabytes(totalKilobytes),
        ConvertKilobytesToMegabytes(usedKilobytes),
        CalculatePercent(usedKilobytes, totalKilobytes)};
}

SwapUsageInfo SystemResourceMonitor::ReadSwapUsageInfo()
{
    const auto memoryInfoValues = ReadMemoryInfoValues();
    const auto totalKilobytes = GetValueOrZero(memoryInfoValues, "SwapTotal");
    const auto freeKilobytes = GetValueOrZero(memoryInfoValues, "SwapFree");
    const auto usedKilobytes = totalKilobytes > freeKilobytes ? totalKilobytes - freeKilobytes : 0;

    return SwapUsageInfo{
        ConvertKilobytesToMegabytes(totalKilobytes),
        ConvertKilobytesToMegabytes(usedKilobytes),
        CalculatePercent(usedKilobytes, totalKilobytes)};
}

std::array<double, 3> SystemResourceMonitor::ReadLoadAverage()
{
    std::ifstream loadAverageFile("/proc/loadavg");
    std::array<double, 3> loadAverage{};

    loadAverageFile >> loadAverage[0] >> loadAverage[1] >> loadAverage[2];

    return loadAverage;
}

std::uint64_t SystemResourceMonitor::ReadUptimeSeconds()
{
    std::ifstream uptimeFile("/proc/uptime");
    double uptimeSeconds = 0.0;

    uptimeFile >> uptimeSeconds;

    return static_cast<std::uint64_t>(uptimeSeconds);
}

double SystemResourceMonitor::CalculateCpuUsagePercent(const CpuTimeSample& currentCpuTimeSample)
{
    if (!m_hasPreviousCpuTimeSample)
    {
        m_previousCpuTimeSample = currentCpuTimeSample;
        m_hasPreviousCpuTimeSample = true;
        return 0.0;
    }

    const auto previousTotalTime = m_previousCpuTimeSample.GetTotalTime();
    const auto previousIdleTime = m_previousCpuTimeSample.GetIdleTime();
    const auto currentTotalTime = currentCpuTimeSample.GetTotalTime();
    const auto currentIdleTime = currentCpuTimeSample.GetIdleTime();

    const auto totalTimeDelta = currentTotalTime > previousTotalTime ? currentTotalTime - previousTotalTime : 0;
    const auto idleTimeDelta = currentIdleTime > previousIdleTime ? currentIdleTime - previousIdleTime : 0;

    m_previousCpuTimeSample = currentCpuTimeSample;

    if (totalTimeDelta == 0)
    {
        return 0.0;
    }

    const auto busyTimeDelta = totalTimeDelta > idleTimeDelta ? totalTimeDelta - idleTimeDelta : 0;
    const auto cpuUsagePercent = static_cast<double>(busyTimeDelta) * 100.0 / static_cast<double>(totalTimeDelta);

    return std::clamp(cpuUsagePercent, 0.0, 100.0);
}
