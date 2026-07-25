// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Crashes.hpp"
#include "Logging.hpp"
#include "System.hpp"
#include "SystemInfo.hpp"
#include "Translations.hpp"
#include "Utils.hpp"
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <ios>
#include <memory>
#include <raylib.h>
#include <string>
#include <vector>
#include <zip.h>

std::shared_ptr<CrashesMenu> g_crashesMenu;

static int ZipFile(zip_t* archive, const std::filesystem::path& path)
{
    // Read the file
    std::ifstream fileStream(path);
    std::ifstream file(path, std::ios::binary);
    if (!fileStream)
    {
        LOG_ERROR("Failed to open file %s", path.string().c_str());
        return 1;
    }
    std::string fileContents;
    std::string buf;
    while (std::getline(fileStream, buf)) fileContents += buf + "\n";
    char* buffer = new char[fileContents.size() + 1];
    strcpy(buffer, fileContents.data());

    // Add a new file to the archive
    zip_source_t* source = zip_source_buffer(archive, buffer, fileContents.size(), 1);
    if (source == nullptr)
    {
        LOG_ERROR("Failed to create crash report zip source: %s", zip_strerror(archive));
        return 1;
    }

    // Add the source as a new file entry
    if (zip_file_add(archive, path.string().c_str(), source, ZIP_FL_OVERWRITE) < 0)
    {
        LOG_ERROR("Failed to add file to crash report zip: %s", zip_strerror(archive));
        zip_source_free(source);
        return 1;
    }

    return 0;
}

static void ZipLogs(zip_t* archive)
{
    auto logFiles = ListFiles("logs/");
    for (const auto& file: logFiles)
    {
        ZipFile(archive, file);
    }
}

static void ZipTimetables(zip_t* archive)
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

static int ZipSystemInfo(zip_t* archive)
{
    // Get system info
    std::string systemInfo;
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
        float monitorDiagonal = sqrtf(powf((float)GetMonitorPhysicalWidth(i), 2) +
                                      powf((float)GetMonitorPhysicalHeight(i), 2));
        constexpr float MM_TO_INCH_RATIO = 25.4;
        monitorDiagonal /= MM_TO_INCH_RATIO;
        const char* monitorDiagonalStr = TextFormat("%.1f", monitorDiagonal);
        systemInfo += "Display " + std::to_string(i) + " (" + GetMonitorName(i) +
                      "): " + std::to_string(GetMonitorWidth(i)) + "x" +
                      std::to_string(GetMonitorHeight(i)) + "@" +
                      std::to_string(GetMonitorRefreshRate(i)) + "Hz " + monitorDiagonalStr + "'\n";
    }
    std::vector<std::string> mounts = GetAllMountPoints();
    for (size_t i = 0; i < mounts.size(); i++)
    {
        constexpr int BYTES_IN_MB = 1024 * 1024;
        std::filesystem::space_info spaceInfo = GetDiskInfo(mounts[i]);
        systemInfo += "Disk " + std::to_string(i) + " at " + mounts[i] +
                      " capacity: " + std::to_string(spaceInfo.capacity / BYTES_IN_MB) + " MB\n";
        systemInfo += "Disk " + std::to_string(i) + " at " + mounts[i] +
                      " free: " + std::to_string(spaceInfo.free / BYTES_IN_MB) + " MB\n";
        systemInfo += "Disk " + std::to_string(i) + " at " + mounts[i] +
                      " available: " + std::to_string(spaceInfo.available / BYTES_IN_MB) + " MB\n";
    }

    // Copy the system info into a temporary buffer
    char* buffer = new char[systemInfo.size() + 1];
    strcpy(buffer, systemInfo.data());

    // Add a new file to the archive
    zip_source_t* source = zip_source_buffer(archive, buffer, systemInfo.size(), 1);
    if (source == nullptr)
    {
        LOG_ERROR("Failed to create crash report zip source: %s", zip_strerror(archive));
        return 1;
    }

    // Add the source as a new file entry
    if (zip_file_add(archive, "system_info.txt", source, ZIP_FL_OVERWRITE) < 0)
    {
        LOG_ERROR("Failed to add file to crash report zip: %s", zip_strerror(archive));
        zip_source_free(source);
        return 1;
    }

    return 0;
}

int CrashesMenu::CreateCrashReport() const
{
    int error = 0;

    // Create crash_report.zip
    zip_t* archive = zip_open("crash_report.zip", ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (archive == nullptr)
    {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, error);
        LOG_ERROR("Failed to create crash report zip archive: %s", zip_error_strerror(&ziperror));
        zip_error_fini(&ziperror);
        return 1;
    }

    ZipFile(archive, "version.txt");
    if (sendLogs_) ZipLogs(archive);
    if (sendTimetables_) ZipTimetables(archive);
    if (sendSettings_) ZipFile(archive, "settings.txt");
    if (sendSystemInfo_) ZipSystemInfo(archive);

    // Close the archive to write changes
    if (zip_close(archive) < 0)
    {
        LOG_ERROR("Failed to close crash report zip archive: %s", zip_strerror(archive));
        return 1;
    }

    LogInfo("Crash report zip file created successfully!");
    return 0;
}

void CrashesMenu::Draw()
{
    if (!ImGui::Begin(gettext("Crash report"), &visible_))
    {
        ImGui::End();
        return;
    }

    ImGui::Text(
        "%s",
        gettext(
            "The program has crashed last time it was opened!\nIf you would like to send an anonymous crash report,\nselect the data you want to share\n(by sending a crash report you can\nhelp to make Timetable Generator even better)"));

    ImGui::Checkbox(gettext("send logs"), &sendLogs_);
    ImGui::Checkbox(gettext("send timetables"), &sendTimetables_);
    ImGui::Checkbox(gettext("send settings"), &sendSettings_);
    ImGui::Checkbox(gettext("send basic system information"), &sendSystemInfo_);

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
