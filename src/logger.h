#pragma once

#include <fstream>
#include <string>
#include <unordered_map>
#include <chrono>

class Logger
{
public:

    static bool Open(const std::string& filename);
    static void Close();

    static void Enable(bool enable);
    static bool IsEnabled();

    static void Log(const std::string& message);

    template<typename T>
    static void Log(const std::string& label, const T& value)
    {
        if (!enabled || !file.is_open())
            return;

        file << label << ": " << value << std::endl;
    }

    static void Flush();

    static void BeginTimer(const std::string& name);
    static double EndTimer(const std::string& name);

private:

    static bool enabled;

    static std::ofstream file;

    static std::unordered_map<
        std::string,
        std::chrono::high_resolution_clock::time_point> timers;
};