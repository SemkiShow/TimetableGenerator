// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Crashes.hpp"
#include "Logging.hpp"
#include "System.hpp"
#include "SystemInfo.hpp"
#include "Utils.hpp"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <iostream>
#include <raylib.h>
#include <string>
#include <zip.h>

bool hasCrashed = false;

bool sendLogs = true;
bool sendTimetables = true;
bool sendSettings = true;
bool sendSystemInfo = true;

int ZipFile(zip_t* archive, std::string path)
{
    // Read the file
    std::ifstream fileStream(path);
    std::ifstream file(path, std::ios::binary);
    if (!fileStream)
    {
        std::cerr << "Failed to open file " << path << "!\n";
        LogError("Failed to open file " + path);
        return 1;
    }
    std::string fileContents = "";
    std::string buf = "";
    while (std::getline(fileStream, buf)) fileContents += buf + "\n";
    char* buffer = new char[fileContents.size()];
    memcpy(buffer, fileContents.data(), fileContents.size());

    // Add a new file to the archive
    zip_source_t* source = zip_source_buffer(archive, buffer, fileContents.size(), 1);
    if (source == nullptr)
    {
        std::cerr << "Failed to create crash report zip source: " << zip_strerror(archive)
                  << std::endl;
        LogError(std::string("Failed to create crash report zip source: ") + zip_strerror(archive));
        return 1;
    }

    // Add the source as a new file entry
    if (zip_file_add(archive, path.c_str(), source, ZIP_FL_OVERWRITE) < 0)
    {
        std::cerr << "Failed to add file to crash report zip: " << zip_strerror(archive)
                  << std::endl;
        LogError(std::string("Failed to add file to crash report zip: ") + zip_strerror(archive));
        zip_source_free(source);
        return 1;
    }

    return 0;
}

void ZipLogs(zip_t* archive)
{
    auto logFiles = ListFiles("logs/");
    for (size_t i = 0; i < logFiles.size(); i++)
    {
        ZipFile(archive, logFiles[i]);
    }
}

void ZipTimetables(zip_t* archive)
{
    // Zip templates
    auto templateFiles = ListFiles("templates/");
    for (size_t i = 0; i < templateFiles.size(); i++)
    {
        ZipFile(archive, templateFiles[i]);
    }

    // Zip timetables
    auto timetableFiles = ListFiles("timetables/");
    for (size_t i = 0; i < timetableFiles.size(); i++)
    {
        ZipFile(archive, timetableFiles[i]);
    }
}

int ZipSystemInfo(zip_t* archive)
{
    // Get system info
    std::string systemInfo = "";
    systemInfo += "OS: " + GetOS() + '\n';
    systemInfo += "CPU: " + GetCPU() + '\n';
    systemInfo += "RAM: " + std::to_string(GetRAMMegabytes()) + " MB\n";
    std::vector<std::string> gpus = GetGPUs();
    for (size_t i = 0; i < gpus.size(); i++)
    {
        if (gpus[i].empty()) continue;
        if (!gpus[i].empty() && gpus[i].back() == '\n') gpus[i].pop_back();
        systemInfo += "GPU " + std::to_string(i) + ": " + gpus[i] + '\n';
    }
    for (int i = 0; i < GetMonitorCount(); i++)
    {
        float monitorDiagonal =
            sqrt(pow(GetMonitorPhysicalWidth(i), 2) + pow(GetMonitorPhysicalHeight(i), 2));
        monitorDiagonal /= 25.4;
        char monitorDiagonalString[50];
        snprintf(monitorDiagonalString, sizeof(monitorDiagonalString), "%.1f", monitorDiagonal);
        systemInfo +=
            "Display " + std::to_string(i) + " (" + GetMonitorName(i) +
            "): " + std::to_string(GetMonitorWidth(i)) + "x" + std::to_string(GetMonitorHeight(i)) +
            "@" + std::to_string(GetMonitorRefreshRate(i)) + "Hz " + monitorDiagonalString + "'\n";
    }
    std::vector<std::string> mounts = GetAllMountPoints();
    for (size_t i = 0; i < mounts.size(); i++)
    {
        std::filesystem::space_info spaceInfo = GetDiskInfo(mounts[i]);
        systemInfo += "Disk " + std::to_string(i) + " at " + mounts[i] +
                      " capacity: " + std::to_string(spaceInfo.capacity / 1024 / 1024) + " MB\n";
        systemInfo += "Disk " + std::to_string(i) + " at " + mounts[i] +
                      " free: " + std::to_string(spaceInfo.free / 1024 / 1024) + " MB\n";
        systemInfo += "Disk " + std::to_string(i) + " at " + mounts[i] +
                      " available: " + std::to_string(spaceInfo.available / 1024 / 1024) + " MB\n";
    }

    // Copy the system info into a temporary buffer
    char* buffer = new char[systemInfo.size()];
    memcpy(buffer, systemInfo.data(), systemInfo.size());

    // Add a new file to the archive
    zip_source_t* source = zip_source_buffer(archive, buffer, systemInfo.size(), 1);
    if (source == nullptr)
    {
        std::cerr << "Failed to create crash report zip source: " << zip_strerror(archive)
                  << std::endl;
        LogError(std::string("Failed to create crash report zip source: ") + zip_strerror(archive));
        return 1;
    }

    // Add the source as a new file entry
    if (zip_file_add(archive, "system_info.txt", source, ZIP_FL_OVERWRITE) < 0)
    {
        std::cerr << "Failed to add file to crash report zip: " << zip_strerror(archive)
                  << std::endl;
        LogError(std::string("Failed to add file to crash report zip: ") + zip_strerror(archive));
        zip_source_free(source);
        return 1;
    }

    return 0;
}

int CreateCrashReport()
{
    int error = 0;

    // Create crash_report.zip
    zip_t* archive = zip_open("crash_report.zip", ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!archive)
    {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, error);
        std::cerr << "Failed to create crash report zip archive: " << zip_error_strerror(&ziperror)
                  << std::endl;
        LogError(std::string("Failed to create crash report zip archive: ") +
                 zip_error_strerror(&ziperror));
        zip_error_fini(&ziperror);
        return 1;
    }

    ZipFile(archive, "version.txt");
    if (sendLogs) ZipLogs(archive);
    if (sendTimetables) ZipTimetables(archive);
    if (sendSettings) ZipFile(archive, "settings.txt");
    if (sendSystemInfo) ZipSystemInfo(archive);

    // Close the archive to write changes
    if (zip_close(archive) < 0)
    {
        std::cerr << "Failed to close crash report zip archive: " << zip_strerror(archive)
                  << std::endl;
        LogError(std::string("Failed to close crash report zip archive: ") + zip_strerror(archive));
        return 1;
    }

    std::cout << "Crash report zip file created successfully!" << std::endl;
    LogInfo("Crash report zip file created successfully!");
    return 0;
}

void OpenCrashReport()
{
    sendLogs = true;
    sendTimetables = true;
    sendSettings = true;
    sendSystemInfo = true;

    isCrashReport = true;
}

bool isCrashReport = false;
void ShowCrashReport(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Crash report"), isOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::Text(
        "%s",
        gettext(
            "The program has crashed last time it was opened!\nIf you would like to send an anonymous crash report,\nselect the data you want to share\n(by sending a crash report you can\nhelp to make Timetable Generator even better)"));

    ImGui::Checkbox(gettext("send logs"), &sendLogs);
    ImGui::Checkbox(gettext("send timetables"), &sendTimetables);
    ImGui::Checkbox(gettext("send settings"), &sendSettings);
    ImGui::Checkbox(gettext("send basic system information"), &sendSystemInfo);

    if (ImGui::Button(gettext("Create crash_report.zip")))
    {
        CreateCrashReport();
        OpenInFileManager(".");
    }

    ImGui::Text(
        "%s",
        gettext(
            "After pressing Create crash_report.zip\nemail crash_report.zip to mgdeveloper123@gmail.com\nThe email theme should be Timetable Generator crash report\nYou can also provide a crash report description"));

    ImGui::End();
}
