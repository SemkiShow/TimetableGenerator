// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Updates.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI/NewVersion.hpp"
#include "Web.hpp"
#include <JsonFormat.hpp>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>
#include <zip.h>
#include <zipconf.h>

std::string latestVersion = "";
std::vector<std::string> releaseNotes;
bool newVersionAvailable = false;
std::string downloadStatus = "";

std::vector<std::string> MultiSplit(const std::string& input, const std::string& delimiter)
{
    std::vector<std::string> output;
    size_t start = 0;
    size_t end;

    while ((end = input.find(delimiter, start)) != std::string::npos)
    {
        output.push_back(input.substr(start, end - start));
        start = end + delimiter.length();
    }
    output.push_back(input.substr(start));

    return output;
}

void GetLatestVersionName()
{
    LogInfo("Fetching the latest version name");

    GetRequest request = {.url =
                              "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases",
                          .headers = {}};
    auto response = PerformGet(request);
    if (!response.success)
    {
        LogError("Failed to get the latest version name!");
        latestVersion = GetText("Failed to get the latest version name!");
        return;
    }

    Json json = Json::Parse(response.response);
    if (!json.empty())
    {
        LogInfo("Successfully fetched releases info");
        size_t releaseID = 0;
        while (json[releaseID]["draft"] ||
               (json[releaseID]["prerelease"] && !settings.usePrereleases))
        {
            releaseID++;
            if (releaseID >= json.size())
            {
                latestVersion = GetText("Error: no valid new version found!");
                return;
            }
        }
        if (releaseID >= json.size())
        {
            releaseID = 0;
            while (json[releaseID]["draft"])
            {
                releaseID++;
                if (releaseID >= json.size())
                {
                    latestVersion = GetText("Error: no valid new version found!");
                    return;
                }
            }
        }
        latestVersion = json[releaseID]["tag_name"];
        releaseNotes = MultiSplit(json[releaseID]["body"], "\\r\\n");
        if (releaseNotes.size() <= 1)
        {
            releaseNotes = MultiSplit(json[releaseID]["body"], "\\n");
        }
        if (latestVersion != version) newVersionMenu->Open();
        LogInfo("Fetched the newest version name: %s", latestVersion.c_str());
    }
    else
    {
        LogError("No releases found in Json response");
        latestVersion = GetText("Error: no valid new version found!");
    }
}

void CheckForUpdates(bool showWindow)
{
    latestVersion = GetText("loading...");
    releaseNotes.clear();
    releaseNotes.push_back(GetText("loading..."));
    if (showWindow) newVersionMenu->Open();
    std::thread networkThread(GetLatestVersionName);
    networkThread.detach();
}

std::string ExtractString(const std::string& input, const std::string& prefix,
                          const std::string& suffix)
{
    size_t start = input.find(prefix);
    if (start == std::string::npos)
    {
        LogError("Prefix not found");
        return "";
    }
    start += prefix.length();

    size_t end = input.rfind(suffix);
    if (end == std::string::npos || end < start)
    {
        LogError("Suffix not found or invalid");
        return "";
    }

    return input.substr(start, end - start);
}

std::string GetLatestVersionArchiveURL()
{
    LogInfo("Fetching the latest version archive URL");

    GetRequest request = {.url =
                              "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases",
                          .headers = {}};
    auto response = PerformGet(request);
    if (!response.success)
    {
        LogError("Failed to fetch the latest version archive URL!");
        return "";
    }

    Json json = Json::Parse(response.response);
    if (!json.empty())
    {
        LogInfo("Successfully fetched releases info");
        size_t releaseID = 0;
        while (json[releaseID]["draft"] ||
               (json[releaseID]["prerelease"] && !settings.usePrereleases))
        {
            releaseID++;
            if (releaseID >= json.size()) return "";
        }
        return json[releaseID]["assets"][0]["browser_download_url"];
    }
    else
    {
        LogError("No releases found in Json response");
    }
    return "";
}

bool UnzipFile(const std::string& zipPath, const std::string& extractDir)
{
    LogInfo("Unzipping the downloaded release");
    int err = 0;
    zip* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &err);
    if (!archive)
    {
        LogError("Failed to open zip archive: %s", zipPath.c_str());
        return false;
    }

    zip_int64_t numEntries = zip_get_num_entries(archive, 0);

    for (zip_uint64_t i = 0; i < (zip_uint64_t)numEntries; i++)
    {
        const char* name = zip_get_name(archive, i, 0);
        if (!name)
        {
            LogError("Failed to get entry name for index %zu", i);
            zip_close(archive);
            return false;
        }

        std::string outPath = extractDir + "/" + name;

        if (name[strlen(name) - 1] == '/')
        {
            std::filesystem::create_directories(outPath);
        }
        else
        {
            std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());

            zip_file* zfile = zip_fopen_index(archive, i, 0);
            if (!zfile)
            {
                LogError("Failed to open file inside zip: %s", name);
                zip_close(archive);
                return false;
            }

            FILE* outfile = fopen(outPath.c_str(), "wb");
            if (!outfile)
            {
                LogError("Failed to create output file: %s", outPath.c_str());
                zip_fclose(zfile);
                zip_close(archive);
                return false;
            }

            char buffer[4096];
            zip_int64_t bytesRead = 0;
            while ((bytesRead = zip_fread(zfile, buffer, sizeof(buffer))) > 0)
            {
                fwrite(buffer, 1, bytesRead, outfile);
            }
            LogInfo("Successfully extracted %s to %s", zipPath.c_str(), extractDir.c_str());

            fclose(outfile);
            zip_fclose(zfile);
        }
    }

    zip_close(archive);
    return true;
}

void CopyFiles(const std::filesystem::path& src, const std::filesystem::path& dest)
{
    for (const auto& entry: std::filesystem::recursive_directory_iterator(src))
    {
        const auto& from = entry.path();
        auto to = dest / std::filesystem::relative(from, src);

        if (entry.is_directory())
        {
            std::filesystem::create_directories(to);
        }
        else
        {
            std::filesystem::copy_file(from, to, std::filesystem::copy_options::overwrite_existing);
        }
    }
    LogInfo("Successfully copied %s to %s", src.string().c_str(), dest.string().c_str());
}

void UpdateToLatestVersion()
{
    downloadStatus = GetText("Fetching the latest version URL...");
    std::string archiveURL = GetLatestVersionArchiveURL();
    if (archiveURL.empty())
    {
        downloadStatus = GetText("Failed to get archive URL");
        LogError("Failed to get archive URL");
        return;
    }

    downloadStatus = GetText("Downloading the latest version...");
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
    if (!DownloadFile(archiveURL, "tmp/release.zip"))
    {
        downloadStatus = GetText("Failed to download the release!");
        LogError("Failed to download the release!");
        return;
    }

    downloadStatus = GetText("Unzipping the release...");
    if (!std::filesystem::exists("tmp/release"))
    {
        std::filesystem::create_directory("tmp/release");
    }
    if (!UnzipFile("tmp/release.zip", "tmp/release"))
    {
        downloadStatus = GetText("Failed to unzip the release!");
        LogError("Failed to uzip the release!");
        return;
    }
    std::filesystem::copy_file("settings.txt", "tmp/settings.txt",
                               std::filesystem::copy_options::overwrite_existing);
    CopyFiles("tmp/release", ".");
    std::filesystem::copy_file("tmp/settings.txt", "settings.txt",
                               std::filesystem::copy_options::overwrite_existing);
    std::filesystem::remove("tmp/release.zip");
    std::filesystem::remove("tmp/settings.txt");
    std::filesystem::remove_all("tmp/release");

    downloadStatus = std::string(GetText("Successfully updated to")) + " " + latestVersion + "!\n" +
                     GetText("Restart the application to see the new features");
    LogInfo("Successfully updated to %s", latestVersion.c_str());
}
