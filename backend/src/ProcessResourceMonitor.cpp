#include "ProcessResourceMonitor.hpp"

#include <algorithm>
#include <cctype>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

bool ProcessResourceMonitor::IsProcessDirectoryName(const std::string& directoryName)
{
    return !directoryName.empty() && std::all_of(directoryName.begin(), directoryName.end(), [](unsigned char character)
    {
        return std::isdigit(character) != 0;
    });
}

std::uint64_t ProcessResourceMonitor::ReadTotalCpuTicks()
{
    std::ifstream statFile("/proc/stat");
    std::string cpuLabel;
    std::uint64_t currentTickValue = 0;
    std::uint64_t totalCpuTicks = 0;

    statFile >> cpuLabel;

    while (statFile >> currentTickValue)
    {
        totalCpuTicks += currentTickValue;

        if (statFile.peek() == '\n')
        {
            break;
        }
    }

    return totalCpuTicks;
}

bool ProcessResourceMonitor::ReadProcessStat(
    int pid,
    ProcessResourceInfo& processInfo,
    std::uint64_t& processCpuTicks)
{
    std::ifstream processStatFile("/proc/" + std::to_string(pid) + "/stat");
    std::string statLine;

    if (!std::getline(processStatFile, statLine))
    {
        return false;
    }

    const auto nameStartPosition = statLine.find('(');
    const auto nameEndPosition = statLine.rfind(')');

    if (nameStartPosition == std::string::npos ||
        nameEndPosition == std::string::npos ||
        nameEndPosition <= nameStartPosition)
    {
        return false;
    }

    processInfo.pid = pid;
    processInfo.name = statLine.substr(nameStartPosition + 1, nameEndPosition - nameStartPosition - 1);

    std::istringstream statFieldsStream(statLine.substr(nameEndPosition + 2));
    statFieldsStream >> processInfo.state;

    std::vector<std::string> statFields;
    std::string currentField;

    while (statFieldsStream >> currentField)
    {
        statFields.push_back(currentField);
    }

    // In /proc/[pid]/stat, after the state field, utime and stime are located at indexes 11 and 12 here.
    if (statFields.size() <= 12)
    {
        return false;
    }

    const auto userModeTicks = static_cast<std::uint64_t>(std::stoull(statFields[11]));
    const auto kernelModeTicks = static_cast<std::uint64_t>(std::stoull(statFields[12]));

    processCpuTicks = userModeTicks + kernelModeTicks;
    processInfo.memoryMegabytes = ReadProcessMemoryMegabytes(pid);

    return true;
}

std::uint64_t ProcessResourceMonitor::ReadProcessMemoryMegabytes(int pid)
{
    std::ifstream processStatusFile("/proc/" + std::to_string(pid) + "/status");
    std::string key;
    std::uint64_t valueKilobytes = 0;
    std::string unit;

    while (processStatusFile >> key >> valueKilobytes >> unit)
    {
        if (key == "VmRSS:")
        {
            return valueKilobytes / 1024;
        }
    }

    return 0;
}

std::vector<ProcessResourceInfo> ProcessResourceMonitor::CollectTopProcesses(std::size_t maxProcessCount)
{
    std::vector<ProcessResourceInfo> processes;
    std::unordered_map<int, std::uint64_t> currentProcessCpuTicks;

    const auto totalCpuTicks = ReadTotalCpuTicks();
    const auto totalCpuTicksDelta = m_hasPreviousSample && totalCpuTicks > m_previousTotalCpuTicks
                                    ? totalCpuTicks - m_previousTotalCpuTicks
                                    : 0;
    const auto processorCount = std::max<long>(1, sysconf(_SC_NPROCESSORS_ONLN));

    DIR* procDirectory = opendir("/proc");
    if (procDirectory == nullptr)
    {
        return processes;
    }

    while (const auto* directoryEntry = readdir(procDirectory))
    {
        const std::string directoryName = directoryEntry->d_name;

        if (!IsProcessDirectoryName(directoryName))
        {
            continue;
        }

        const int pid = std::stoi(directoryName);
        ProcessResourceInfo processInfo;
        std::uint64_t processCpuTicks = 0;

        if (!ReadProcessStat(pid, processInfo, processCpuTicks))
        {
            continue;
        }

        currentProcessCpuTicks[pid] = processCpuTicks;

        if (m_hasPreviousSample && totalCpuTicksDelta > 0 && m_previousProcessCpuTicks.count(pid) != 0)
        {
            const auto previousProcessCpuTicks = m_previousProcessCpuTicks.at(pid);
            const auto processCpuTicksDelta = processCpuTicks > previousProcessCpuTicks
                                             ? processCpuTicks - previousProcessCpuTicks
                                             : 0;

            processInfo.cpuUsagePercent = static_cast<double>(processCpuTicksDelta) * 100.0 *
                                          static_cast<double>(processorCount) /
                                          static_cast<double>(totalCpuTicksDelta);
        }

        processes.push_back(std::move(processInfo));
    }

    closedir(procDirectory);

    m_previousProcessCpuTicks = std::move(currentProcessCpuTicks);
    m_previousTotalCpuTicks = totalCpuTicks;
    m_hasPreviousSample = true;

    std::sort(processes.begin(), processes.end(), [](const ProcessResourceInfo& leftProcess, const ProcessResourceInfo& rightProcess)
    {
        if (leftProcess.cpuUsagePercent == rightProcess.cpuUsagePercent)
        {
            return leftProcess.memoryMegabytes > rightProcess.memoryMegabytes;
        }

        return leftProcess.cpuUsagePercent > rightProcess.cpuUsagePercent;
    });

    if (processes.size() > maxProcessCount)
    {
        processes.resize(maxProcessCount);
    }

    return processes;
}
