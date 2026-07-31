#include "Logger.h"

#include <maya/MGlobal.h>

std::ofstream IMDLogger::file;
bool IMDLogger::enabled = false;
std::unordered_map<
    std::string,
    std::chrono::high_resolution_clock::time_point>
    IMDLogger::timers;

bool IMDLogger::Open(const std::string& filename)
{
    if (!enabled)
        return true;

    if (file.is_open())
        file.close();

    file.open(filename, std::ios::out | std::ios::app);

    return file.is_open();
}

void IMDLogger::Close()
{
    if (!enabled)
        return;

    if (file.is_open())
        file.close();
}

void IMDLogger::Log(const std::string& message)
{
    if (!enabled || !file.is_open())
        return;

    file << message << std::endl;
}

void IMDLogger::Flush()
{
    if (!enabled || !file.is_open())
        return;

    file.flush();
}

void IMDLogger::BeginTimer(const std::string& name)
{
    timers[name] =
        std::chrono::high_resolution_clock::now();
}

double IMDLogger::EndTimer(const std::string& name)
{
    auto it = timers.find(name);

    if (it == timers.end())
        return -1.0;

    auto end = std::chrono::high_resolution_clock::now();

    double ms =
        std::chrono::duration<double, std::milli>(
            end - it->second).count();

    if(enabled)
    {
        Log(name + " Time (ms)", ms);
    }

    timers.erase(it);

    return ms;
}

void IMDLogger::Enable(bool enable)
{
    enabled = enable;
}

bool IMDLogger::IsEnabled()
{
    return enabled;
}