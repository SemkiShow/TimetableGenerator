// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Time.hpp"
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <string>

static FILE* g_logFile;

static void OpenLogFile()
{
    const char* const LOGS_DIR = "logs/";
    if (!std::filesystem::exists(LOGS_DIR)) std::filesystem::create_directories(LOGS_DIR);
    if (g_logFile != nullptr) fclose(g_logFile);
    const std::string FILE_NAME = LOGS_DIR + GetCurrentTime().ToString(Time::Format::Path) + ".log";
    g_logFile = fopen(FILE_NAME.c_str(), "w");
}

void LogInfo(const char* format, ...)
{
    if (g_logFile == nullptr) OpenLogFile();

    va_list args;
    va_start(args, format);
    va_list argsCopy;

    va_copy(argsCopy, args);
    fprintf(g_logFile, "%s: [INFO] ", GetCurrentTime().ToString().c_str());
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

    va_end(args);
}

void LogWarning(const char* format, ...)
{
    if (g_logFile == nullptr) OpenLogFile();

    va_list args;
    va_start(args, format);
    va_list argsCopy;

    va_copy(argsCopy, args);
    fprintf(g_logFile, "%s: [WARNING] ", GetCurrentTime().ToString().c_str());
    vfprintf(g_logFile, format, argsCopy);
    fprintf(g_logFile, "\n");
    fflush(g_logFile);
    va_end(argsCopy);

    va_copy(argsCopy, args);
    printf("[\x1b[33mWARNING\x1b[0m] ");
    vprintf(format, argsCopy);
    printf("\n");
    fflush(stdout);
    va_end(argsCopy);

    va_end(args);
}

void LogError(const char* format, ...)
{
    if (g_logFile == nullptr) OpenLogFile();

    va_list args;
    va_start(args, format);
    va_list argsCopy;

    va_copy(argsCopy, args);
    fprintf(g_logFile, "%s: [ERROR] ", GetCurrentTime().ToString().c_str());
    vfprintf(g_logFile, format, argsCopy);
    fprintf(g_logFile, "\n");
    fflush(g_logFile);
    va_end(argsCopy);

    va_copy(argsCopy, args);
    printf("[\x1b[31mERROR\x1b[0m] ");
    vprintf(format, argsCopy);
    printf("\n");
    fflush(stdout);
    va_end(argsCopy);

    va_end(args);
}
