#include "Logger.h"

#include <maya/MGlobal.h>

std::ofstream Logger::file;
bool Logger::enabled = false;
std::unordered_map<
    std::string,
    std::chrono::high_resolution_clock::time_point>
    Logger::timers;

bool Logger::Open(const std::string& filename)
{
    if (!enabled)
        return true;

    if (file.is_open())
        file.close();

    file.open(filename, std::ios::out | std::ios::app);

    return file.is_open();
}

void Logger::Close()
{
    if (!enabled)
        return;

    if (file.is_open())
        file.close();
}

void Logger::Log(const std::string& message)
{
    if (!enabled || !file.is_open())
        return;

    file << message << std::endl;
}

void Logger::Flush()
{
    if (!enabled || !file.is_open())
        return;

    file.flush();
}

void Logger::BeginTimer(const std::string& name)
{
    timers[name] =
        std::chrono::high_resolution_clock::now();
}

double Logger::EndTimer(const std::string& name)
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

void Logger::Enable(bool enable)
{
    enabled = enable;
}

bool Logger::IsEnabled()
{
    return enabled;
}