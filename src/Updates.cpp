#include "Updates.hpp"
#include "JSON.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <thread>
#define CURL_STATICLIB
#include <curl/curl.h>
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
        std::cerr << "Failed to initialize CURL" << std::endl;
        LogError("Failed to initialize CURL");
        latestVersion = labels["Error: CURL init failed!"];
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
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        LogError(std::string("CURL request failed: ") + curl_easy_strerror(res));
        latestVersion = labels["Error: network request failed!"];
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 200)
        {
            JSONObject jsonObject;
            ParseJSON(readBuffer, &jsonObject);
            if (!jsonObject.objects.empty())
            {
                LogInfo("Successfully fetched releases info");
                size_t releaseID = 0;
                while (jsonObject.objects[releaseID].boolPairs["draft"] ||
                       (jsonObject.objects[releaseID].boolPairs["prerelease"] && !usePrereleases))
                {
                    releaseID++;
                    if (releaseID >= jsonObject.objects.size())
                    {
                        latestVersion = labels["Error: no valid new version found!"];
                        curl_easy_cleanup(curl);
                        return;
                    }
                }
                latestVersion = jsonObject.objects[releaseID].stringPairs["tag_name"];
                releaseNotes = MultiSplit(jsonObject.objects[releaseID].stringPairs["body"], "\\n");
                if (latestVersion != version) isNewVersion = true;
                LogInfo("Fetched the newest version name: " + latestVersion);
            }
            else
            {
                std::cerr << "No releases found in JSON response" << std::endl;
                LogError("No releases found in JSON response");
                latestVersion = labels["Error: no valid new version found!"];
            }
        }
        else
        {
            std::cerr << "HTTP request failed: Status " << responseCode << std::endl;
            LogError("HTTP request failed: Status " + std::to_string(responseCode));
            latestVersion = labels["Error: bad HTTP status!"];
        }
    }

    curl_easy_cleanup(curl);
    return;
}

void CheckForUpdates(bool showWindow)
{
    latestVersion = labels["loading..."];
    releaseNotes.clear();
    releaseNotes.push_back(labels["loading..."]);
    if (showWindow) isNewVersion = true;
    std::thread networkThread(GetLatestVersionName);
    networkThread.detach();
}

std::string ExtractString(const std::string& input, const std::string& prefix,
                          const std::string& suffix)
{
    size_t start = input.find(prefix);
    if (start == std::string::npos)
    {
        std::cerr << "Prefix not found\n";
        return "";
    }
    start += prefix.length();

    size_t end = input.rfind(suffix);
    if (end == std::string::npos || end < start)
    {
        std::cerr << "Suffix not found or invalid\n";
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
        std::cerr << "Failed to initialize CURL" << std::endl;
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
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        LogError(std::string("CURL request failed: ") + curl_easy_strerror(res));
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        if (responseCode == 200)
        {
            JSONObject jsonObject;
            ParseJSON(readBuffer, &jsonObject);
            if (!jsonObject.objects.empty())
            {
                LogInfo("Successfully fetched releases info");
                size_t releaseID = 0;
                while (jsonObject.objects[releaseID].boolPairs["draft"] ||
                       (jsonObject.objects[releaseID].boolPairs["prerelease"] && !usePrereleases))
                {
                    releaseID++;
                    if (releaseID >= jsonObject.objects.size()) return "";
                }
                int assetID = -1;
#ifdef _WIN32
                std::string releasePrefix = "release-windows-";
#else
                std::string releasePrefix = "release-linux-";
#endif
                for (size_t i = 0;
                     i < jsonObject.objects[releaseID].objectPairs["assets"].objects.size(); i++)
                {
                    std::string assetLanguage = ExtractString(jsonObject.objects[releaseID]
                                                                  .objectPairs["assets"]
                                                                  .objects[i]
                                                                  .stringPairs["name"],
                                                              releasePrefix, ".zip");
                    if (assetLanguage == language)
                    {
                        assetID = i;
                        break;
                    }
                }
                if (assetID == -1)
                {
                    LogError("Current language not found in the response");
                    for (size_t i = 0;
                         i < jsonObject.objects[releaseID].objectPairs["assets"].objects.size();
                         i++)
                    {
                        std::string assetLanguage = ExtractString(jsonObject.objects[releaseID]
                                                                      .objectPairs["assets"]
                                                                      .objects[i]
                                                                      .stringPairs["name"],
                                                                  releasePrefix, ".zip");
                        if (assetLanguage == "en")
                        {
                            assetID = i;
                            break;
                        }
                    }
                }
                if (assetID == -1)
                {
                    assetID = 0;
                    LogError("English not found in the response");
                }
                return jsonObject.objects[releaseID]
                    .objectPairs["assets"]
                    .objects[assetID]
                    .stringPairs["browser_download_url"];
            }
            else
            {
                std::cerr << "No releases found in JSON response" << std::endl;
                LogError("No releases found in JSON response");
            }
        }
        else
        {
            std::cerr << "HTTP request failed: Status " << responseCode << std::endl;
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
    LogInfo("Downloading the latest release");
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
        std::cerr << "Failed to open zip archive: " << zipPath << std::endl;
        LogError("Failed to open zip archive: " + zipPath);
        return 1;
    }

    zip_int64_t numEntries = zip_get_num_entries(archive, 0);

    for (zip_uint64_t i = 0; i < (zip_uint64_t)numEntries; i++)
    {
        const char* name = zip_get_name(archive, i, 0);
        if (!name)
        {
            std::cerr << "Failed to get entry name for index " << i << std::endl;
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
                std::cerr << "Failed to open file inside zip: " << name << std::endl;
                LogError(std::string("Failed to open file inside zip: ") + name);
                zip_close(archive);
                return 1;
            }

            FILE* outfile = fopen(outPath.c_str(), "wb");
            if (!outfile)
            {
                std::cerr << "Failed to create output file: " << outPath << std::endl;
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
    downloadStatus = labels["Fetching the latest version URL..."];
    std::string archiveURL = GetLatestVersionArchiveURL();
    if (archiveURL.empty())
    {
        downloadStatus = labels["Failed to get archive URL"];
        std::cerr << "Failed to get archive URL\n";
        LogError("Failed to get archive URL");
        return;
    }

    downloadStatus = labels["Downloading the latest version..."];
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
    if (DownloadFile(archiveURL, "tmp/release.zip") != 0)
    {
        downloadStatus = labels["Failed to download the release!"];
        std::cerr << "Failed to download the release!\n";
        return;
    }

    downloadStatus = labels["Unzipping the release..."];
    if (!std::filesystem::exists("tmp/release"))
    {
        std::filesystem::create_directory("tmp/release");
    }
    if (UnzipFile("tmp/release.zip", "tmp/release") != 0)
    {
        downloadStatus = labels["Failed to unzip the release!"];
        std::cerr << "Failed to uzip the release!\n";
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

    downloadStatus = labels["Successfully updated to"] + " " + latestVersion + "!\n" +
                     labels["Restart the application to see the new features"];
    LogInfo("Successfully updated to " + latestVersion);
}

void UpdateCACertificate()
{
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
    DownloadFile("https://curl.se/ca/cacert.pem", "tmp/cacert.pem");
    std::filesystem::copy_file("tmp/cacert.pem", "resources/cacert.pem",
                               std::filesystem::copy_options::overwrite_existing);
    std::filesystem::remove("tmp/cacert.pem");
    std::cout << "Updated the CA certificate\n";
    LogInfo("Updated the CA certificate");
}
