// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>
#include <vector>

extern std::string g_latestVersion;
extern std::vector<std::string> g_releaseNotes;
extern std::string g_downloadStatus;

void GetLatestVesionName();
void CheckForUpdates(bool showWindow = true);
void UpdateToLatestVersion();
