// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Time.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

static std::ofstream g_logFile;
static std::mutex g_logMutex;

void BeginLogging()
{
    const std::string LOGS_DIR = "logs/";
    if (!std::filesystem::exists(LOGS_DIR)) std::filesystem::create_directories(LOGS_DIR);
    g_logFile.open(LOGS_DIR + GetTimeString(GetCurrentTime()) + ".log");
}

void LogInfo(const std::string& data)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_logFile << "[INFO] " << GetTimeString(GetCurrentTime()) << ": " << data << '\n';
    std::cout << "[INFO] " << data << '\n';
}

void LogError(const std::string& data)
{
    std::lock_guard<std::mutex> lock(g_logMutex);
    g_logFile << "[ERROR] " << GetTimeString(GetCurrentTime()) << ": " << data << '\n';
    std::cerr << "[\033[31mERROR\033[0m] " << data << '\n';
}

void EndLogging() { g_logFile.close(); }
