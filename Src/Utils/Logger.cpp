#include "Logger.h"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <string>
#include <fstream>
#include <iostream>

HANDLE Logger::s_ConsoleHandle = nullptr;
HANDLE Logger::m_MutexHandle = nullptr;
bool Logger::s_Initialized = false;

void Logger::Init()
{
    if (!s_Initialized)
        EnableTerminal();

    mFileSystem.OpenForWrite(GetTimestampForLogPath());

    m_MutexHandle = CreateMutex(nullptr,
        FALSE,
        nullptr);
}

void Logger::EnableTerminal()
{
    // Create a new console window if one doesn't exist
    if (!AllocConsole()) return;

    // Redirect standard output to the console
    FILE* dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    freopen_s(&dummy, "CONIN$", "r", stdin);

    s_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    s_Initialized = true;
}

bool Logger::Log(const std::string& prefix, const std::string& message, WORD color,
    const char* file, int line, const char* func)
{
    if (!s_Initialized)
        EnableTerminal();

    std::ostringstream oss;

    // Optional: Add timestamp
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_s(&localTime, &now);
    oss << "[" << std::put_time(&localTime, "%H:%M:%S") << "] ";

    // Prefix and location
    oss << "[" << prefix << "] ";
    if (file && func && line >= 0)
    {
        oss << "(" << file << ":" << line << " | " << func << ") ";
    }

    // Actual message
    oss << "- " << message << "\n";

    std::string refinedMessage = oss.str();
    bool saved = false;

    DWORD res = WaitForSingleObject(m_MutexHandle, 2000);

    if (res == WAIT_OBJECT_0)
    {
        // Got the lock
        SetConsoleTextAttribute(s_ConsoleHandle, color);
        std::cout << refinedMessage;
        SetConsoleTextAttribute(s_ConsoleHandle, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);

        saved = mFileSystem.WritePlainText(refinedMessage);

        ReleaseMutex(m_MutexHandle);
    }
    else if (res == WAIT_TIMEOUT)
    {
        // Couldn't acquire the lock in time (don't ReleaseMutex!)
        saved = mFileSystem.WritePlainText("[FAILURE] Logger timeout.\n");
        return false;
    }
    else
    {
        // Some other error (OPTIONAL: You can log it too)
        return false;
    }
    return saved;
}

bool Logger::Info(const std::string& message, const char* file, int line, const char* func)
{
    if (!s_Initialized) return false;
    return Log("INFO", message, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY, file, line, func);
}

bool Logger::Print(const std::string& message, const char* file, int line, const char* func)
{
    if (!s_Initialized) return false;
    return Log("", message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, file, line, func);
}

bool Logger::Warning(const std::string& message, const char* file, int line, const char* func)
{
    if (!s_Initialized) return false;
    return Log("WARNING", message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, file, line, func);
}

bool Logger::Error(const std::string& message, const char* file, int line, const char* func)
{
    if (!s_Initialized) return false;
    return Log("ERROR", message, FOREGROUND_RED | FOREGROUND_INTENSITY, file, line, func);
}

bool Logger::Success(const std::string& message, const char* file, int line, const char* func)
{
    if (!s_Initialized) return false;
    return Log("SUCCESS", message, FOREGROUND_GREEN | FOREGROUND_INTENSITY, file, line, func);
}

bool Logger::Fail(const std::string& message, const char* file, int line, const char* func)
{
    if (!s_Initialized) return false;
    return Log("FAIL", message, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY, file, line, func);
}

std::string Logger::GetTimestampForLogPath()
{
    auto now = std::chrono::system_clock::now();
    std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
    std::tm localTime;

    localtime_s(&localTime, &timeNow);

    std::ostringstream oss;
    oss << Barar::Log::DEFAULT_PATH
        <<"Log_"
        << std::put_time(&localTime, "%Y-%m-%d_%H-%M-%S")
        << ".txt";

    return oss.str();
}

void Logger::Close()
{
    mFileSystem.Close();
}
