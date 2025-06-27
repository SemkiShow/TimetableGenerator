#include <iostream>
#include <string>
#include <thread>
#include <vector>

extern std::string latestVersion;
extern std::vector<std::string> releaseNotes;

void GetLatestVesionName();
void CheckForUpdates(bool showWindow = true);
