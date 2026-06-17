/**
 * @file ProcessResourceMonitor.hpp
 * @brief Process-level resource metrics collection
 *
 * Collects per-process resource metrics from /proc/[pid]/* files
 * and identifies the top consumers of CPU and memory.
 */

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @struct ProcessResourceInfo
 * @brief Resource information for a single process
 */
struct ProcessResourceInfo
{
    /// Process identifier
    int pid{};
    /// Process name/command
    std::string name;
    /// Process state (R=running, S=sleeping, Z=zombie, etc.)
    char state{'?'};
    /// CPU usage percentage (0.0 to 100.0 * num_cores on systems with multiple CPUs)
    double cpuUsagePercent{};
    /// Memory usage in megabytes
    std::uint64_t memoryMegabytes{};
};

/**
 * @class ProcessResourceMonitor
 * @brief Monitors per-process resource usage
 *
 * Tracks CPU and memory usage for all processes on the system.
 * Maintains historical data to calculate accurate CPU usage percentages.
 * Thread-safe for concurrent metric collection.
 *
 * @example
 * @code
 * ProcessResourceMonitor monitor;
 * auto processes = monitor.CollectTopProcesses(25); // Top 25 CPU consumers
 * @endcode
 */
class ProcessResourceMonitor
{
public:
    /**
     * @brief Collect top processes by resource usage
     *
     * Scans all processes in /proc and returns those consuming the most CPU.
     * CPU usage is calculated based on time deltas from the previous sample.
     * Returns 0% CPU usage on the first call (no previous data).
     *
     * @param maxProcessCount Maximum number of processes to return (default: 20)
     * @return std::vector<ProcessResourceInfo> Top processes sorted by CPU usage
     * @throws std::runtime_error if /proc cannot be accessed
     */
    [[nodiscard]] std::vector<ProcessResourceInfo> CollectTopProcesses(std::size_t maxProcessCount = 20);

private:
    /// Previous per-process CPU tick counts
    std::unordered_map<int, std::uint64_t> m_previousProcessCpuTicks;
    /// Total system CPU ticks from previous sample
    std::uint64_t m_previousTotalCpuTicks{};
    /// Flag indicating if a previous sample exists
    bool m_hasPreviousSample{false};

    /**
     * @brief Check if a directory name is a valid PID
     *
     * @param directoryName Directory name to check
     * @return bool True if the name consists only of digits
     */
    [[nodiscard]] static bool IsProcessDirectoryName(const std::string& directoryName);

    /**
     * @brief Read total CPU ticks across all CPUs
     *
     * @return std::uint64_t Total system CPU ticks from /proc/stat
     * @throws std::runtime_error if /proc/stat cannot be read
     */
    [[nodiscard]] static std::uint64_t ReadTotalCpuTicks();

    /**
     * @brief Read process information from /proc/[pid]/stat
     *
     * Extracts process name, state, and CPU tick information.
     *
     * @param pid Process identifier
     * @param processInfo Output parameter for process information
     * @param processCpuTicks Output parameter for CPU ticks
     * @return bool True if successfully read, false if process no longer exists
     * @throws std::runtime_error if file access fails
     */
    [[nodiscard]] static bool ReadProcessStat(int pid, ProcessResourceInfo& processInfo, std::uint64_t& processCpuTicks);

    /**
     * @brief Read memory usage for a process
     *
     * Extracts RSS memory from /proc/[pid]/status.
     *
     * @param pid Process identifier
     * @return std::uint64_t Memory usage in megabytes
     * @throws std::runtime_error if /proc/[pid]/status cannot be read
     */
    [[nodiscard]] static std::uint64_t ReadProcessMemoryMegabytes(int pid);
};
