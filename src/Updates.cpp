#include "Updates.hpp"
#include "UI.hpp"
#include "JSON.hpp"
#include "Settings.hpp"
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

    while ((end = input.find(delimiter, start)) != std::string::npos) {
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
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Failed to initialize CURL" << std::endl;
        latestVersion = "Error: CURL init failed!";
        return;
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("TimetableGenerator/" + version).c_str());
    curl_easy_setopt(curl, CURLOPT_CAINFO, "resources/cacert.pem");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        latestVersion = "Error: network request failed!";
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
                latestVersion = jsonObject.objects[0].stringPairs["tag_name"];
                releaseNotes = MultiSplit(jsonObject.objects[0].stringPairs["body"], "\\n");
                if (latestVersion != version) isNewVersion = true;
            }
            else std::cerr << "No releases found in JSON response" << std::endl;
        }
        else
        {
            std::cerr << "HTTP request failed: Status " << responseCode << std::endl;
            latestVersion = "Error: bad HTTP status!";
        }
    }

    curl_easy_cleanup(curl);
}

void CheckForUpdates(bool showWindow)
{
    latestVersion = "loading...";
    releaseNotes.clear();
    releaseNotes.push_back("loading...");
    if (showWindow) isNewVersion = true;
    std::thread networkThread(GetLatestVersionName);
    networkThread.detach();
}

std::string GetLatestVersionArchiveURL()
{
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Failed to initialize CURL" << std::endl;
        return "";
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("TimetableGenerator/" + version).c_str());
    curl_easy_setopt(curl, CURLOPT_CAINFO, "resources/cacert.pem");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
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
                int releaseID = 0;
                while (jsonObject.objects[releaseID].boolPairs["draft"] ||
                    (jsonObject.objects[releaseID].boolPairs["prerelease"] && !usePrereleases))
                {
                    releaseID++;
                    if (releaseID >= jsonObject.objects.size()) return "";
                }
                return jsonObject.objects[releaseID].objectPairs["assets"].objects[0].stringPairs["browser_download_url"];
            }
            else std::cerr << "No releases found in JSON response" << std::endl;
        }
        else
        {
            std::cerr << "HTTP request failed: Status " << responseCode << std::endl;
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
    FILE* file = fopen(outputPath.c_str(), "wb");
    if (!file) return 1;

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        fclose(file);
        return 1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "TimetableGenerator");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "resources/cacert.pem");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(file);
    return (res == CURLE_OK) ? 0 : 1;
}

int UnzipFile(const std::string& zipPath, const std::string& extractDir)
{
    int err = 0;
    zip* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &err);
    if (!archive)
    {
        std::cerr << "Failed to open zip archive: " << zipPath << std::endl;
        return 1;
    }

    zip_int64_t numEntries = zip_get_num_entries(archive, 0);

    for (zip_uint64_t i = 0; i < (zip_uint64_t)numEntries; i++)
    {
        const char* name = zip_get_name(archive, i, 0);
        if (!name)
        {
            std::cerr << "Failed to get entry name for index " << i << std::endl;
            zip_close(archive);
            return 1;
        }

        std::string outPath = extractDir + "/" + name;

        // Check if entry is a directory (ends with '/')
        if (name[strlen(name) - 1] == '/')
        {
            // Create directory
            std::filesystem::create_directories(outPath);
        }
        else
        {
            // Ensure parent directory exists
            std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());

            zip_file* zfile = zip_fopen_index(archive, i, 0);
            if (!zfile)
            {
                std::cerr << "Failed to open file inside zip: " << name << std::endl;
                zip_close(archive);
                return 1;
            }

            FILE* outfile = fopen(outPath.c_str(), "wb");
            if (!outfile)
            {
                std::cerr << "Failed to create output file: " << outPath << std::endl;
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

            fclose(outfile);
            zip_fclose(zfile);
        }
    }

    zip_close(archive);
    return 0;
}

void CopyFiles(const std::filesystem::path& src, const std::filesystem::path& dest)
{
    for (const auto& entry : std::filesystem::recursive_directory_iterator(src))
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
}

void UpdateToLatestVersion()
{
    downloadStatus = "Fetching the latest version URL...";
    std::string archiveURL = GetLatestVersionArchiveURL();
    if (archiveURL.empty())
    {
        downloadStatus = "Failed to get archive URL";
        std::cerr << "Failed to get archive URL\n";
        return;
    }

    downloadStatus = "Downloading the latest version...";
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
    if (DownloadFile(archiveURL, "tmp/release.zip"))
    {
        downloadStatus = "Failed to download the release!";
        std::cerr << "Failed to download the release!\n";
        return;
    }

    downloadStatus = "Unzipping the file...";
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
    if (UnzipFile("tmp/release.zip", "tmp/release"))
    {
        downloadStatus = "Failed to uzip the release!";
        std::cerr << "Failed to uzip the release!\n";
        return;
    }
    std::filesystem::copy_file("settings.txt", "tmp/settings.txt", std::filesystem::copy_options::overwrite_existing);
    CopyFiles("tmp/release", ".");
    std::filesystem::copy_file("tmp/settings.txt", "settings.txt", std::filesystem::copy_options::overwrite_existing);
    
    downloadStatus = "Successfully updated to " + latestVersion + "!\nRestart the application to see the new features";
}
