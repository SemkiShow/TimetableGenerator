#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NODRAWTEXT
#define NOUSER

#include <iostream>
#include <string>
#include <thread>
#include <vector>
#define CURL_STATICLIB
#include <curl/curl.h>

extern std::string latestVersion;
extern std::vector<std::string> releaseNotes;

void GetLatestVesionName();
void CheckForUpdates(bool showWindow = true);
