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

std::string GetLatestVesionName()
{
    httplib::SSLClient cli("api.github.com", 443);
    cli.set_default_headers({{"User-Agent", "TimetableGenerator"}});
    cli.set_ca_cert_path("resources/cacert.pem");

    auto res = cli.Get("/repos/SemkiShow/TimetableGenerator/releases");
    if (res && res->status == 200)
    {
        JSONObject jsonObject;
        ParseJSON(res->body, &jsonObject);
        if (!jsonObject.objects.empty()) return jsonObject.objects[0].stringPairs["tag_name"];
        else std::cerr << "No releases found in JSON response" << std::endl;
    }
    else
    {
        std::cerr << "HTTP request failed: ";
        if (res) std::cerr << "Status " << res->status << std::endl;
        else std::cerr << "No response received" << std::endl;
    }
    return "-1";
}

void CheckForUpdates(bool showWindow)
{
    latestVersion = "loading...";
    if (showWindow) isNewVersion = true;
    latestVersion = GetLatestVesionName();
    if (latestVersion != version) isNewVersion = true;
}
