#pragma once

#include <string>
#include <vector>

extern std::string latestVersion;
extern std::vector<std::string> releaseNotes;
extern std::string downloadStatus;

void GetLatestVesionName();
void CheckForUpdates(bool showWindow = true);
void UpdateToLatestVersion();
void UpdateCACertificate();
