#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

struct ProcessResourceInfo
{
    int pid{};
    std::string name;
    char state{'?'};
    double cpuUsagePercent{};
    std::uint64_t memoryMegabytes{};
};

class ProcessResourceMonitor
{
public:
    [[nodiscard]] std::vector<ProcessResourceInfo> CollectTopProcesses(std::size_t maxProcessCount = 20);

private:
    std::unordered_map<int, std::uint64_t> m_previousProcessCpuTicks;
    std::uint64_t m_previousTotalCpuTicks{};
    bool m_hasPreviousSample{false};

    [[nodiscard]] static bool IsProcessDirectoryName(const std::string& directoryName);
    [[nodiscard]] static std::uint64_t ReadTotalCpuTicks();
    [[nodiscard]] static bool ReadProcessStat(int pid, ProcessResourceInfo& processInfo, std::uint64_t& processCpuTicks);
    [[nodiscard]] static std::uint64_t ReadProcessMemoryMegabytes(int pid);
};
