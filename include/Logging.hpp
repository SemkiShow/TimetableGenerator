#pragma once

#include <string>

void BeginLogging();
void LogInfo(std::string data);
void LogError(std::string data);
void EndLogging();
