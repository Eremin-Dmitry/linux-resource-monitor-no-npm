#pragma once

#include <array>
#include <cstdint>

struct CpuTimeSample
{
    std::uint64_t userTime{};
    std::uint64_t niceTime{};
    std::uint64_t systemTime{};
    std::uint64_t idleTime{};
    std::uint64_t ioWaitTime{};
    std::uint64_t irqTime{};
    std::uint64_t softIrqTime{};
    std::uint64_t stealTime{};

    [[nodiscard]] std::uint64_t GetIdleTime() const;
    [[nodiscard]] std::uint64_t GetTotalTime() const;
};

struct MemoryUsageInfo
{
    std::uint64_t totalMegabytes{};
    std::uint64_t usedMegabytes{};
    double usagePercent{};
};

struct SwapUsageInfo
{
    std::uint64_t totalMegabytes{};
    std::uint64_t usedMegabytes{};
    double usagePercent{};
};

struct SystemResourceMetrics
{
    double cpuUsagePercent{};
    MemoryUsageInfo memoryUsage;
    SwapUsageInfo swapUsage;
    std::array<double, 3> loadAverage{};
    std::uint64_t uptimeSeconds{};
};

class SystemResourceMonitor
{
public:
    SystemResourceMonitor();

    [[nodiscard]] SystemResourceMetrics CollectMetrics();

private:
    CpuTimeSample m_previousCpuTimeSample{};
    bool m_hasPreviousCpuTimeSample{false};

    [[nodiscard]] static CpuTimeSample ReadCpuTimeSample();
    [[nodiscard]] static MemoryUsageInfo ReadMemoryUsageInfo();
    [[nodiscard]] static SwapUsageInfo ReadSwapUsageInfo();
    [[nodiscard]] static std::array<double, 3> ReadLoadAverage();
    [[nodiscard]] static std::uint64_t ReadUptimeSeconds();

    [[nodiscard]] double CalculateCpuUsagePercent(const CpuTimeSample& currentCpuTimeSample);
};
