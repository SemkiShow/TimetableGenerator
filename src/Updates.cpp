#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMINMAX
#define NODRAWTEXT
#define NOUSER

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "UI.hpp"
#include "Updates.hpp"
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

void GetLatestVesionName()
{
    httplib::SSLClient cli("api.github.com", 443);
    cli.set_default_headers({{"User-Agent", "TimetableGenerator"}});
    cli.set_ca_cert_path("resources/cacert.pem");

    auto res = cli.Get("/repos/SemkiShow/TimetableGenerator/releases");
    if (res && res->status == 200)
    {
        JSONObject jsonObject;
        ParseJSON(res->body, &jsonObject);
        if (!jsonObject.objects.empty())
        {
            latestVersion = jsonObject.objects[0].stringPairs["tag_name"];
            releaseNotes = MultiSplit(jsonObject.objects[0].stringPairs["body"], "\\n");
            if (latestVersion != version) isNewVersion = true;
            return;
        }
        else std::cerr << "No releases found in JSON response" << std::endl;
    }
    else
    {
        std::cerr << "HTTP request failed: ";
        if (res) std::cerr << "Status " << res->status << std::endl;
        else std::cerr << "No response received" << std::endl;
    }
    latestVersion = "\nError: no network connection!";
}

void CheckForUpdates(bool showWindow)
{
    latestVersion = "loading...";
    releaseNotes.clear();
    releaseNotes.push_back("loading...");
    if (showWindow) isNewVersion = true;
    std::thread networkThread(GetLatestVesionName);
    networkThread.detach();
}
