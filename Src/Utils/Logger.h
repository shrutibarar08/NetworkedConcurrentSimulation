#pragma once

#include <windows.h>

#include "FileManager/FileLoader/FileSystem.h"

namespace Barar
{
	namespace Log
	{
        inline constexpr const char* DEFAULT_PATH = "Logs/";
	}
}

class Logger
{
public:
	static void Init();

    // Logging methods
    static bool Info(const std::string& message, const char* file, int line, const char* func);
    static bool Print(const std::string& message, const char* file, int line, const char* func);
    static bool Warning(const std::string& message, const char* file, int line, const char* func);
    static bool Error(const std::string& message, const char* file, int line, const char* func);
    static bool Success(const std::string& message, const char* file, int line, const char* func);
    static bool Fail(const std::string& message, const char* file, int line, const char* func);
    static std::string GetTimestampForLogPath();
    static void Close();
private:
    static void EnableTerminal();

    static bool Log(const std::string& prefix, const std::string& message, WORD color,
        const char* file = nullptr, int line = -1, const char* func = nullptr);

private:
    static HANDLE s_ConsoleHandle;
    static HANDLE m_MutexHandle;
    static bool s_Initialized;

    inline static FileSystem mFileSystem{};
};

#define LOG_INFO(msg) Logger::Info(msg, __FILE__, __LINE__, __func__)
#define LOG_PRINT(msg) Logger::Print(msg, __FILE__, __LINE__, __func__)
#define LOG_WARNING(msg) Logger::Warning(msg, __FILE__, __LINE__, __func__)
#define LOG_ERROR(msg) Logger::Error(msg, __FILE__, __LINE__, __func__)
#define LOG_SUCCESS(msg) Logger::Success(msg, __FILE__, __LINE__, __func__)
#define LOG_FAIL(msg) Logger::Fail(msg, __FILE__, __LINE__, __func__)
