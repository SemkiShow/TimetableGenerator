#include "Updates.hpp"
#include "UI.hpp"
#include "JSON.hpp"
#include "Settings.hpp"

std::string latestVersion = "";
std::vector<std::string> releaseNotes;
bool newVersionAvailable = false;

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
        latestVersion = "\nError: CURL init failed!";
        return;
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/repos/SemkiShow/TimetableGenerator/releases");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "TimetableGenerator");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "resources/cacert.pem");

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        latestVersion = "\nError: network request failed!";
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
            latestVersion = "\nError: bad HTTP status!";
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
