// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Updates.hpp"
#include "Json.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI/NewVersion.hpp"
#include <cstring>
#include <curl/curl.h>
#include <filesystem>
#include <string>
#include <thread>
#include <zip.h>

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

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void GetLatestVersionName()
{
    LogInfo("Fetching the latest version archive URL");
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        LogError("Failed to initialize CURL");
        latestVersion = GetText("Error: CURL init failed!");
        curl_easy_cleanup(curl);
        return;
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("TimetableGenerator/" + version).c_str());
    curl_easy_setopt(curl, CURLOPT_CAINFO, "resources/cacert.pem");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        LogError(std::string("CURL request failed: ") + curl_easy_strerror(res));
        latestVersion = GetText("Error: network request failed!");
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 200)
        {
            Json jsonObject = Json::Parse(readBuffer);
            if (!jsonObject.GetArray().empty())
            {
                LogInfo("Successfully fetched releases info");
                size_t releaseID = 0;
                while (jsonObject[releaseID]["draft"].GetBool() ||
                       (jsonObject[releaseID]["prerelease"].GetBool() && !usePrereleases))
                {
                    releaseID++;
                    if (releaseID >= jsonObject.size())
                    {
                        latestVersion = GetText("Error: no valid new version found!");
                        curl_easy_cleanup(curl);
                        return;
                    }
                }
                if (releaseID >= jsonObject.size())
                {
                    releaseID = 0;
                    while (jsonObject[releaseID]["draft"].GetBool())
                    {
                        releaseID++;
                        if (releaseID >= jsonObject.size())
                        {
                            latestVersion = GetText("Error: no valid new version found!");
                            curl_easy_cleanup(curl);
                            return;
                        }
                    }
                }
                latestVersion = jsonObject[releaseID]["tag_name"].GetString();
                releaseNotes = MultiSplit(jsonObject[releaseID]["body"].GetString(), "\\r\\n");
                if (releaseNotes.size() <= 1)
                {
                    releaseNotes = MultiSplit(jsonObject[releaseID]["body"].GetString(), "\\n");
                }
                if (latestVersion != version) newVersionMenu->Open();
                LogInfo("Fetched the newest version name: " + latestVersion);
            }
            else
            {
                LogError("No releases found in Json response");
                latestVersion = GetText("Error: no valid new version found!");
            }
        }
        else
        {
            LogError("HTTP request failed: Status " + std::to_string(responseCode));
            latestVersion = GetText("Error: bad HTTP status!");
        }
    }

    curl_easy_cleanup(curl);
    return;
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
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        LogError("Failed to initialize CURL");
        return "";
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("TimetableGenerator/" + version).c_str());
    curl_easy_setopt(curl, CURLOPT_CAINFO, "resources/cacert.pem");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        LogError(std::string("CURL request failed: ") + curl_easy_strerror(res));
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 200)
        {
            Json jsonObject = Json::Parse(readBuffer);
            if (!jsonObject.GetArray().empty())
            {
                LogInfo("Successfully fetched releases info");
                size_t releaseID = 0;
                while (jsonObject[releaseID]["draft"].GetBool() ||
                       (jsonObject[releaseID]["prerelease"].GetBool() && !usePrereleases))
                {
                    releaseID++;
                    if (releaseID >= jsonObject.size()) return "";
                }
                return jsonObject[releaseID]["assets"][0]["browser_download_url"].GetString();
            }
            else
            {
                LogError("No releases found in Json response");
            }
        }
        else
        {
            LogError("HTTP request failed: Status " + std::to_string(responseCode));
        }
    }

    curl_easy_cleanup(curl);
    return "";
}

size_t WriteFileCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

int DownloadFile(const std::string& url, const std::string& outputPath)
{
    LogInfo("Downloading the file " + url);
    FILE* file = fopen(outputPath.c_str(), "wb");
    if (!file) return 1;

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        LogError("Failed to initialize CURL");
        fclose(file);
        return 1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("TimetableGenerator/" + version).c_str());
    curl_easy_setopt(curl, CURLOPT_CAINFO, "resources/cacert.pem");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(file);
    return (res == CURLE_OK) ? 0 : 1;
}

int UnzipFile(const std::string& zipPath, const std::string& extractDir)
{
    LogInfo("Unzipping the downloaded release");
    int err = 0;
    zip* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &err);
    if (!archive)
    {
        LogError("Failed to open zip archive: " + zipPath);
        return 1;
    }

    zip_int64_t numEntries = zip_get_num_entries(archive, 0);

    for (zip_uint64_t i = 0; i < (zip_uint64_t)numEntries; i++)
    {
        const char* name = zip_get_name(archive, i, 0);
        if (!name)
        {
            LogError("Failed to get entry name for index " + std::to_string(i));
            zip_close(archive);
            return 1;
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
                LogError(std::string("Failed to open file inside zip: ") + name);
                zip_close(archive);
                return 1;
            }

            FILE* outfile = fopen(outPath.c_str(), "wb");
            if (!outfile)
            {
                LogError("Failed to create output file: " + outPath);
                zip_fclose(zfile);
                zip_close(archive);
                return 1;
            }

            char buffer[4096];
            zip_int64_t bytesRead = 0;
            while ((bytesRead = zip_fread(zfile, buffer, sizeof(buffer))) > 0)
            {
                fwrite(buffer, 1, bytesRead, outfile);
            }
            LogInfo("Successfully extracted " + zipPath + " to " + extractDir);

            fclose(outfile);
            zip_fclose(zfile);
        }
    }

    zip_close(archive);
    return 0;
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
    LogInfo("Successfully copied " + src.string() + " to " + dest.string());
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
    if (DownloadFile(archiveURL, "tmp/release.zip") != 0)
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
    if (UnzipFile("tmp/release.zip", "tmp/release") != 0)
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
    LogInfo("Successfully updated to " + latestVersion);
}

void UpdateCACertificate()
{
    if (!std::filesystem::exists("tmp")) std::filesystem::create_directory("tmp");
    if (DownloadFile("https://curl.se/ca/cacert.pem", "tmp/cacert.pem") != 0)
    {
        LogError("Failed to update the CA certificate!");
        return;
    }
    std::filesystem::copy_file("tmp/cacert.pem", "resources/cacert.pem",
                               std::filesystem::copy_options::overwrite_existing);
    std::filesystem::remove("tmp/cacert.pem");

    // Update lastCAUpdate
    time_t now = time(0);
    lastCAUpdate = asctime(localtime(&now));
    LogInfo("Updated the CA certificate");
}
