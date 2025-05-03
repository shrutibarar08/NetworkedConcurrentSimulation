#pragma once

#include "Core/DefineDefault.h"

#include <windows.h>
#include "FileManager/FileLoader/FileSystem.h"


class Logger
{
public:
	Logger(const LOGGER_INITIALIZE_DESC* desc);
	~Logger();

	Logger(const Logger&) = delete;
	Logger(Logger&&) = delete;
	Logger& operator=(const Logger&) = delete;
	Logger& operator=(Logger&&) = delete;

	// Logging methods
    bool Info(const std::string& message, const char* file, int line, const char* func);
    bool Print(const std::string& message, const char* file, int line, const char* func);
    bool Warning(const std::string& message, const char* file, int line, const char* func);
	bool Error(const std::string& message, const char* file, int line, const char* func);
	bool Success(const std::string& message, const char* file, int line, const char* func);
	bool Fail(const std::string& message, const char* file, int line, const char* func);
	std::string GetTimestampForLogPath();
	void Close();
private:
	void EnableTerminal();
	bool Log(const std::string& prefix, const std::string& message, WORD color,
		const char* file = nullptr, int line = -1, const char* func = nullptr);

private:
	LOGGER_INITIALIZE_DESC mLoggerDesc;
	HANDLE mConsoleHandle;
	HANDLE mMutexHandle;
	FileSystem mFileSystem{};
};

// Declare global Logger pointer
extern Logger* gLogger;

// Define once globally
#define INIT_GLOBAL_LOGGER(desc) \
    do { gLogger = new Logger(desc); } while(0)

#define LOG_INFO(msg)    (gLogger ? gLogger->Info(msg, __FILE__, __LINE__, __func__) : false)
#define LOG_PRINT(msg)   (gLogger ? gLogger->Print(msg, __FILE__, __LINE__, __func__) : false)
#define LOG_WARNING(msg) (gLogger ? gLogger->Warning(msg, __FILE__, __LINE__, __func__) : false)
#define LOG_ERROR(msg)   (gLogger ? gLogger->Error(msg, __FILE__, __LINE__, __func__) : false)
#define LOG_SUCCESS(msg) (gLogger ? gLogger->Success(msg, __FILE__, __LINE__, __func__) : false)
#define LOG_FAIL(msg)    (gLogger ? gLogger->Fail(msg, __FILE__, __LINE__, __func__) : false)
