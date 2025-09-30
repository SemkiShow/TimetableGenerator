// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

void BeginLogging();
void LogInfo(std::string data);
void LogError(std::string data);
void EndLogging();
