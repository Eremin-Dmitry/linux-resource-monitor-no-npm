/**
 * @file SystemResourceMonitor.hpp
 * @brief System resource metrics collection from Linux /proc filesystem
 *
 * This module is responsible for collecting system-level resource metrics
 * including CPU usage, memory, swap, load average, and uptime from the
 * Linux /proc virtual filesystem.
 */

#pragma once

#include <array>
#include <cstdint>

/**
 * @struct CpuTimeSample
 * @brief CPU time counters snapshot from /proc/stat
 *
 * Contains the CPU time spent in various states, used for calculating
 * CPU usage percentage over time intervals.
 */
struct CpuTimeSample
{
    /// Time spent in user mode
    std::uint64_t userTime{};
    /// Time spent in nice user mode
    std::uint64_t niceTime{};
    /// Time spent in system mode
    std::uint64_t systemTime{};
    /// Time spent in idle state
    std::uint64_t idleTime{};
    /// Time spent waiting for I/O
    std::uint64_t ioWaitTime{};
    /// Time spent handling interrupts
    std::uint64_t irqTime{};
    /// Time spent handling software interrupts
    std::uint64_t softIrqTime{};
    /// Time spent in steal mode (for virtual machines)
    std::uint64_t stealTime{};

    /**
     * @brief Calculate idle time
     * @return std::uint64_t Total idle time (idle + ioWait)
     */
    [[nodiscard]] std::uint64_t GetIdleTime() const;

    /**
     * @brief Calculate total CPU time
     * @return std::uint64_t Sum of all CPU time counters
     */
    [[nodiscard]] std::uint64_t GetTotalTime() const;
};

/**
 * @struct MemoryUsageInfo
 * @brief Memory statistics from /proc/meminfo
 */
struct MemoryUsageInfo
{
    /// Total memory in megabytes
    std::uint64_t totalMegabytes{};
    /// Used memory in megabytes
    std::uint64_t usedMegabytes{};
    /// Memory usage percentage (0.0 to 100.0)
    double usagePercent{};
};

/**
 * @struct SwapUsageInfo
 * @brief Swap space statistics from /proc/meminfo
 */
struct SwapUsageInfo
{
    /// Total swap space in megabytes
    std::uint64_t totalMegabytes{};
    /// Used swap space in megabytes
    std::uint64_t usedMegabytes{};
    /// Swap usage percentage (0.0 to 100.0)
    double usagePercent{};
};

/**
 * @struct SystemResourceMetrics
 * @brief Complete system resource metrics snapshot
 *
 * Aggregates all system-level metrics for a single collection interval.
 */
struct SystemResourceMetrics
{
    /// CPU usage percentage (0.0 to 100.0)
    double cpuUsagePercent{};
    /// Memory usage information
    MemoryUsageInfo memoryUsage;
    /// Swap usage information
    SwapUsageInfo swapUsage;
    /// Load average for 1, 5, and 15 minutes
    std::array<double, 3> loadAverage{};
    /// System uptime in seconds since boot
    std::uint64_t uptimeSeconds{};
};

/**
 * @class SystemResourceMonitor
 * @brief Collects system-level resource metrics
 *
 * Reads metrics from Linux /proc virtual filesystem and calculates
 * CPU usage based on time deltas between consecutive samples.
 * Thread-safe for simultaneous metric collection.
 *
 * @example
 * @code
 * SystemResourceMonitor monitor;
 * auto metrics = monitor.CollectMetrics(); // First sample (0% CPU)
 * std::this_thread::sleep_for(std::chrono::seconds(1));
 * metrics = monitor.CollectMetrics(); // Second sample (accurate CPU)
 * @endcode
 */
class SystemResourceMonitor
{
public:
    /**
     * @brief Constructor
     *
     * Initializes the monitor. First call to CollectMetrics()
     * will return 0% CPU usage as there's no previous sample to compare.
     */
    SystemResourceMonitor();

    /**
     * @brief Collect current system resource metrics
     *
     * Reads current metrics from /proc filesystem. CPU usage is calculated
     * based on the delta from the previous sample. Returns 0% CPU on first call.
     *
     * @return SystemResourceMetrics Current system metrics
     * @throws std::runtime_error if /proc files cannot be read
     */
    [[nodiscard]] SystemResourceMetrics CollectMetrics();

private:
    /// Previous CPU time sample for delta calculation
    CpuTimeSample m_previousCpuTimeSample{};
    /// Flag indicating if a previous sample exists
    bool m_hasPreviousCpuTimeSample{false};

    /**
     * @brief Read CPU time counters from /proc/stat
     * @return CpuTimeSample Current CPU time sample
     * @throws std::runtime_error if /proc/stat cannot be read
     */
    [[nodiscard]] static CpuTimeSample ReadCpuTimeSample();

    /**
     * @brief Read memory usage from /proc/meminfo
     * @return MemoryUsageInfo Current memory usage
     * @throws std::runtime_error if /proc/meminfo cannot be read
     */
    [[nodiscard]] static MemoryUsageInfo ReadMemoryUsageInfo();

    /**
     * @brief Read swap usage from /proc/meminfo
     * @return SwapUsageInfo Current swap usage
     * @throws std::runtime_error if /proc/meminfo cannot be read
     */
    [[nodiscard]] static SwapUsageInfo ReadSwapUsageInfo();

    /**
     * @brief Read system load average from /proc/loadavg
     * @return std::array<double, 3> Load averages [1min, 5min, 15min]
     * @throws std::runtime_error if /proc/loadavg cannot be read
     */
    [[nodiscard]] static std::array<double, 3> ReadLoadAverage();

    /**
     * @brief Read system uptime from /proc/uptime
     * @return std::uint64_t Uptime in seconds
     * @throws std::runtime_error if /proc/uptime cannot be read
     */
    [[nodiscard]] static std::uint64_t ReadUptimeSeconds();

    /**
     * @brief Calculate CPU usage percentage from time deltas
     *
     * Compares the current CPU sample with the previous sample
     * to calculate usage percentage for the interval.
     *
     * @param currentCpuTimeSample Current CPU time counters
     * @return double CPU usage percentage (0.0 to 100.0)
     */
    [[nodiscard]] double CalculateCpuUsagePercent(const CpuTimeSample& currentCpuTimeSample);
};
