// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Time.hpp"
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <string>

static FILE* g_logFile;

static void OpenLogFile()
{
    const char* const LOGS_DIR = "logs/";
    if (!std::filesystem::exists(LOGS_DIR)) std::filesystem::create_directories(LOGS_DIR);
    if (g_logFile != nullptr) fclose(g_logFile);
    const std::string FILE_NAME = LOGS_DIR + Time::Now().ToString(Time::Format::Path) + ".log";
    g_logFile = fopen(FILE_NAME.c_str(), "w");
}

#define OPEN_LOG_FILE                                                                              \
    do                                                                                             \
    {                                                                                              \
        if (g_logFile == nullptr) OpenLogFile();                                                   \
        if (g_logFile == nullptr)                                                                  \
        {                                                                                          \
            int error = errno;                                                                     \
            printf("\x1b[31mFailed to open g_logFile\x1b[0m: %s\n", strerror(error));              \
            return;                                                                                \
        }                                                                                          \
    } while (0)

void LogInfo(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VLogInfo(format, args);
    va_end(args);
}

void LogWarning(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VLogWarning(format, args);
    va_end(args);
}

void LogError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VLogError(format, args);
    va_end(args);
}

void VLogInfo(const char* format, va_list args)
{
    OPEN_LOG_FILE;

    va_list argsCopy;

    va_copy(argsCopy, args);
    fprintf(g_logFile, "%s: [INFO] ", Time::Now().ToString().c_str());
    vfprintf(g_logFile, format, argsCopy);
    fprintf(g_logFile, "\n");
    fflush(g_logFile);
    va_end(argsCopy);

    va_copy(argsCopy, args);
    printf("[INFO] ");
    vprintf(format, argsCopy);
    printf("\n");
    fflush(stdout);
    va_end(argsCopy);
}

void VLogWarning(const char* format, va_list args)
{
    OPEN_LOG_FILE;

    va_list argsCopy;

    va_copy(argsCopy, args);
    fprintf(g_logFile, "%s: [WARN] ", Time::Now().ToString().c_str());
    vfprintf(g_logFile, format, argsCopy);
    fprintf(g_logFile, "\n");
    fflush(g_logFile);
    va_end(argsCopy);

    va_copy(argsCopy, args);
    printf("[\x1b[33mWARN\x1b[0m] ");
    vprintf(format, argsCopy);
    printf("\n");
    fflush(stdout);
    va_end(argsCopy);
}

// FAIL was used instead of ERROR to align with the rest of the logging functions' output
void VLogError(const char* format, va_list args)
{
    OPEN_LOG_FILE;

    va_list argsCopy;

    va_copy(argsCopy, args);
    fprintf(g_logFile, "%s: [FAIL] ", Time::Now().ToString().c_str());
    vfprintf(g_logFile, format, argsCopy);
    fprintf(g_logFile, "\n");
    fflush(g_logFile);
    va_end(argsCopy);

    va_copy(argsCopy, args);
    printf("[\x1b[31mFAIL\x1b[0m] ");
    vprintf(format, argsCopy);
    printf("\n");
    fflush(stdout);
    va_end(argsCopy);
}

void LogInfoLine(const char* file, int line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VLogInfoLine(file, line, format, args);
    va_end(args);
}

void LogWarningLine(const char* file, int line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VLogWarningLine(file, line, format, args);
    va_end(args);
}

void LogErrorLine(const char* file, int line, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    VLogErrorLine(file, line, format, args);
    va_end(args);
}

static std::string GetLineStr(const char* file, int line, const char* format, va_list args)
{
    std::string prefix = std::string(file) + ":" + std::to_string(line) + ": ";

    va_list argsCopy;

    int formatLen = 0;
    va_copy(argsCopy, args);
    formatLen = vsnprintf(nullptr, 0, format, argsCopy);
    va_end(argsCopy);

    if (formatLen < 0) return prefix + "[Format Error]";

    std::string message(formatLen, '\0');
    va_copy(argsCopy, args);
    vsnprintf(message.data(), formatLen + 1, format, argsCopy);
    va_end(argsCopy);

    return prefix + message;
}

void VLogInfoLine(const char* file, int line, const char* format, va_list args)
{
    LogInfo("%s", GetLineStr(file, line, format, args).c_str());
}

void VLogWarningLine(const char* file, int line, const char* format, va_list args)
{
    LogWarning("%s", GetLineStr(file, line, format, args).c_str());
}

void VLogErrorLine(const char* file, int line, const char* format, va_list args)
{
    LogError("%s", GetLineStr(file, line, format, args).c_str());
}
