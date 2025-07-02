#pragma once

#include <string>
#include <vector>

extern std::string latestVersion;
extern std::vector<std::string> releaseNotes;

void GetLatestVesionName();
void CheckForUpdates(bool showWindow = true);
