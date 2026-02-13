// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Translations.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <libintl.h>

std::vector<std::string> availableLanguages;
std::string languageValues;
int languageId = -1;

std::string GetText(const std::string& id) { return std::string(gettext(id.c_str())); }

void SetLanguage(const std::string& domain, const std::filesystem::path& localePath,
                 const std::string& language)
{
#ifdef _WIN32
    _putenv(("LANGUAGE=" + language).c_str());
#else
    setenv("LANGUAGE", language.c_str(), 1);
#endif
    bindtextdomain(domain.c_str(), localePath.string().c_str());
    textdomain(domain.c_str());
}

void GetAllLanguages()
{
    availableLanguages.clear();
    availableLanguages.push_back("en");
    for (auto& lang: std::filesystem::directory_iterator("resources/locales"))
    {
        if (!lang.is_directory()) continue;
        availableLanguages.push_back(lang.path().filename().string());
    }

    languageValues = "";
    languageId = -1;
    for (size_t i = 0; i < availableLanguages.size(); i++)
    {
        languageValues += availableLanguages[i];
        languageValues += '\0';
        if (std::string(availableLanguages[i]) == language) languageId = i;
    }
    languageValues += '\0';
    if (languageId == -1) languageId = 0;
}

void ReloadLabels()
{
    // Read the language file
    LogInfo("Reloading labels");
    SetLanguage("TimetableGenerator", "resources/locales", language);

    // Assign translated week days
    weekDays[0] = GetText("Monday");
    weekDays[1] = GetText("Tuesday");
    weekDays[2] = GetText("Wednesday");
    weekDays[3] = GetText("Thursday");
    weekDays[4] = GetText("Friday");
    weekDays[5] = GetText("Saturday");
    weekDays[6] = GetText("Sunday");

    // Assign translated style values
    styleValues = "";
    styleValues += GetText("dark");
    styleValues += '\0';
    styleValues += GetText("light");
    styleValues += '\0';
    styleValues += GetText("classic");
    styleValues += '\0';
    styleValues += '\0';
}
