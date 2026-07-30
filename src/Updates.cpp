// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Updates.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI/NewVersion.hpp"
#include "Web.hpp"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <thread>
#include <vector>
#include <zip.h>
#include <zipconf.h>
using json = nlohmann::json;

std::string g_latestVersion;
std::vector<std::string> g_releaseNotes;
std::string g_downloadStatus;

static std::vector<std::string> MultiSplit(const std::string& input, const std::string& delimiter)
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

static void GetLatestVersionName()
{
    LogInfo("Fetching the latest version name");

    GetRequest request = {
        .url = "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases",
        .headers = {},
    };
    auto response = PerformGet(request);
    if (!response.success)
    {
        LOG_ERROR("Failed to get the latest version name!");
        g_latestVersion = GetText("Failed to get the latest version name!");
        return;
    }

    json responseJson = json::parse(response.body);
    if (responseJson.empty())
    {
        LOG_ERROR("No releases found in json response");
        g_latestVersion = GetText("Error: no valid new version found!");
        return;
    }

    LogInfo("Successfully fetched releases info");
    size_t releaseId = 0;
    while (responseJson[releaseId]["draft"] ||
           (responseJson[releaseId]["prerelease"] && !g_settings.usePrereleases))
    {
        releaseId++;
        if (releaseId >= responseJson.size())
        {
            g_latestVersion = GetText("Error: no valid new version found!");
            return;
        }
    }
    g_latestVersion = responseJson[releaseId]["tag_name"];
    g_releaseNotes = MultiSplit(responseJson[releaseId]["body"], "\\r\\n");
    if (g_releaseNotes.size() <= 1)
    {
        g_releaseNotes = MultiSplit(responseJson[releaseId]["body"], "\\n");
    }
    if (g_latestVersion != g_version) g_newVersionMenu->Open();
    LogInfo("Fetched the newest version name: %s", g_latestVersion.c_str());
}

void CheckForUpdates(bool showWindow)
{
    g_latestVersion = GetText("Loading...");
    g_releaseNotes.clear();
    g_releaseNotes.push_back(GetText("Loading..."));
    if (showWindow) g_newVersionMenu->Open();
    std::thread networkThread(GetLatestVersionName);
    networkThread.detach();
}

static std::string GetLatestVersionArchiveURL()
{
    LogInfo("Fetching the latest version archive URL");

    GetRequest request = {
        .url = "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases",
        .headers = {},
    };
    auto response = PerformGet(request);
    if (!response.success)
    {
        LOG_ERROR("Failed to fetch the latest version archive URL!");
        return "";
    }

    json responseJson = json::parse(response.body);
    if (responseJson.empty())
    {
        LOG_ERROR("No releases found in response");
        return "";
    }

    size_t releaseId = 0;
    while (responseJson[releaseId]["draft"] ||
           (responseJson[releaseId]["prerelease"] && !g_settings.usePrereleases))
    {
        releaseId++;
        if (releaseId >= responseJson.size()) return "";
    }
    return responseJson[releaseId]["assets"][0]["browser_download_url"];
}

static bool UnzipFile(const std::string& zipPath, const std::string& extractDir)
{
    LogInfo("Unzipping the downloaded release");
    int err = 0;
    zip* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &err);
    if (archive == nullptr)
    {
        LOG_ERROR("Failed to open zip archive: %s", zipPath.c_str());
        return false;
    }

    zip_int64_t numEntries = zip_get_num_entries(archive, 0);

    for (zip_uint64_t i = 0; i < (zip_uint64_t)numEntries; i++)
    {
        const char* name = zip_get_name(archive, i, 0);
        if (name == nullptr)
        {
            LOG_ERROR("Failed to get entry name for index %zu", i);
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
            if (zfile == nullptr)
            {
                LOG_ERROR("Failed to open file inside zip: %s", name);
                zip_close(archive);
                return false;
            }

            FILE* outfile = fopen(outPath.c_str(), "wb");
            if (outfile == nullptr)
            {
                LOG_ERROR("Failed to create output file: %s", outPath.c_str());
                zip_fclose(zfile);
                zip_close(archive);
                return false;
            }

            constexpr size_t BUFFER_SIZE = 4096;
            char buffer[BUFFER_SIZE];
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

static void CopyFiles(const std::filesystem::path& src, const std::filesystem::path& dest)
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
    g_downloadStatus = GetText("Fetching the latest version URL...");
    std::string archiveURL = GetLatestVersionArchiveURL();
    if (archiveURL.empty())
    {
        g_downloadStatus = GetText("Failed to get archive URL");
        LOG_ERROR("Failed to get archive URL");
        return;
    }

    g_downloadStatus = GetText("Downloading the latest version...");
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
    if (!DownloadFile(archiveURL, "tmp/release.zip"))
    {
        g_downloadStatus = GetText("Failed to download the release!");
        LOG_ERROR("Failed to download the release!");
        return;
    }

    g_downloadStatus = GetText("Unzipping the release...");
    if (!std::filesystem::exists("tmp/release"))
    {
        std::filesystem::create_directory("tmp/release");
    }
    if (!UnzipFile("tmp/release.zip", "tmp/release"))
    {
        g_downloadStatus = GetText("Failed to unzip the release!");
        LOG_ERROR("Failed to uzip the release!");
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

    g_downloadStatus = GetText("Successfully updated to") + " " + g_latestVersion + "!\n" +
                       GetText("Restart the application to see the new features");
    LogInfo("Successfully updated to %s", g_latestVersion.c_str());
}
