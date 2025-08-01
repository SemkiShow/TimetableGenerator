#include "Logging.hpp"
#include <ctime>
#include <fstream>
#include <string>

std::ofstream logFile;

void BeginLogging()
{
    time_t now = time(0);
    std::string time = asctime(localtime(&now));
    if (!time.empty() && time.back() == '\n') time.pop_back();
    logFile.open(std::string("logs/") + time + ".log");
}

void LogInfo(std::string data)
{
    time_t now = time(0);
    std::string time = asctime(localtime(&now));
    if (!time.empty() && time.back() == '\n') time.pop_back();
    logFile << "[INFO] " << time << ": " << data << '\n';
}

void LogError(std::string data)
{
    time_t now = time(0);
    std::string time = asctime(localtime(&now));
    if (!time.empty() && time.back() == '\n') time.pop_back();
    logFile << "[ERROR] " << time << ": " << data << '\n';
}

void EndLogging() { logFile.close(); }
