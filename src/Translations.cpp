// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Translations.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include "UI/Settings.hpp"
#include <cstddef>
#include <filesystem>
#include <libintl.h>
#include <stdlib.h>
#include <string>
#include <vector>

std::vector<std::string> g_availableLanguages;
std::string g_languageValues;
int g_languageId = -1;

std::string GetText(const std::string& id) { return std::string(gettext(id.c_str())); }

void SetLanguage(const std::string& domain, const std::filesystem::path& localePath,
                 const std::string& language)
{
#ifdef _WIN32
    _putenv(("LANGUAGE=" + language).c_str());
#else
    setenv("LANGUAGE", language.c_str(), 1);
#endif
    closeAllLoadedMessageCatalogs();
    bindtextdomain(domain.c_str(), localePath.string().c_str());
    textdomain(domain.c_str());
}

void GetAllLanguages()
{
    g_availableLanguages.clear();
    g_availableLanguages.push_back("en");
    for (const auto& lang: std::filesystem::directory_iterator("resources/locales"))
    {
        if (!lang.is_directory()) continue;
        g_availableLanguages.push_back(lang.path().filename().string());
    }

    g_languageValues = "";
    g_languageId = -1;
    for (size_t i = 0; i < g_availableLanguages.size(); i++)
    {
        g_languageValues += g_availableLanguages[i];
        g_languageValues += '\0';
        if (std::string(g_availableLanguages[i]) == g_settings.language) g_languageId = (int)i;
    }
    g_languageValues += '\0';
    if (g_languageId == -1) g_languageId = 0;
}

void ReloadLabels()
{
    // Read the language file
    LogInfo("Reloading labels");
    LogInfo("Current language: %s", g_settings.language.c_str());
    SetLanguage("TimetableGenerator", "resources/locales", g_settings.language);

    // Assign translated week days
    g_weekDays = {
        gettext("Monday"), gettext("Tuesday"),  gettext("Wednesday"), gettext("Thursday"),
        gettext("Friday"), gettext("Saturday"), gettext("Sunday"),
    };

    // Assign translated style values
    g_settingsMenu->ReloadLabels();
}
