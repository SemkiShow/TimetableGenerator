// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Settings.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "Utils.hpp"
#include <fstream>
#include <iostream>
#include <string>

Settings g_settings;

std::string g_version;

void Settings::Save() const
{
    LogInfo("Saving settings");
    std::ofstream file(SETTINGS_PATH);
    file << "last-timetable=" << g_currentTimetable.name << '\n';
    file << "days-per-week=" << daysPerWeek << '\n';
    file << "lessons-per-day=" << lessonsPerDay << '\n';
    file << "style=" << static_cast<int>(style) << '\n';
    file << "language=" << language << '\n';
    file << "min-free-periods=" << minFreePeriods << '\n';
    file << "max-free-periods=" << maxFreePeriods << '\n';
    file << "vsync=" << (vsync ? "true" : "false") << '\n';
    file << "merged-font=" << (mergedFont ? "true" : "false") << '\n';
    file << "timetable-autosave-interval=" << autosaveInterval << '\n';
    file << "font-size=" << fontSize << '\n';
    file << "error-bonus-ratio=" << errorBonusRatio << '\n';
    file << "timetables-per-generation-step=" << timetablesPerGenerationStep << '\n';
    file << "min-timetables-per-generation=" << minTimetablesPerGeneration << '\n';
    file << "max-timetables-per-generation=" << maxTimetablesPerGeneration << '\n';
    file << "max-iterations=" << maxIterations << '\n';
    file << "additional-bonus-points=" << additionalBonusPoints << '\n';
    file << "verbose-logging=" << (verboseLogging ? "true" : "false") << '\n';
    file << "use-prereleases=" << (usePrereleases ? "true" : "false") << '\n';
    file << "has-crashed=" << (hasCrashed ? "true" : "false") << '\n';
    file.close();

    // Save timetable
    g_currentTimetable.Save("templates/" + g_currentTimetable.name + ".json");
}

void Settings::Load()
{
    LogInfo("Loading settings");
    std::ifstream file(SETTINGS_PATH);
    std::string buf;
    std::string label;
    std::string value;
    while (std::getline(file, buf))
    {
        if (Split(buf, '=').size() < 2)
        {
            LOG_ERROR("Invalid %s!", SETTINGS_PATH);
            continue;
        }
        label = TrimJunk(Split(buf, '=')[0]);
        value = TrimJunk(Split(buf, '=')[1]);

        if (label == "last-timetable" && value != "")
        {
            g_currentTimetable = Timetable();
            g_currentTimetable.name = value;
        }
        if (label == "days-per-week") daysPerWeek = stoi(value);
        if (label == "lessons-per-day") lessonsPerDay = stoi(value);
        if (label == "style") style = static_cast<Style>(stoi(value));
        if (label == "language") language = value;
        if (label == "min-free-periods") minFreePeriods = stoi(value);
        if (label == "max-free-periods") maxFreePeriods = stoi(value);
        if (label == "vsync") vsync = value == "true";
        if (label == "merged-font") mergedFont = value == "true";
        if (label == "timetable-autosave-interval") autosaveInterval = stoi(value);
        if (label == "font-size") fontSize = stoi(value);
        if (label == "timetables-per-generation-step") timetablesPerGenerationStep = stoi(value);
        if (label == "min-timetables-per-generation") minTimetablesPerGeneration = stoi(value);
        if (label == "max-timetables-per-generation") maxTimetablesPerGeneration = stoi(value);
        if (label == "error-bonus-ratio") errorBonusRatio = stof(value);
        if (label == "max-iterations") maxIterations = stoi(value);
        if (label == "additional-bonus-points") additionalBonusPoints = stoi(value);
        if (label == "verbose-logging") verboseLogging = value == "true";
        if (label == "use-prereleases") usePrereleases = value == "true";
        if (label == "has-crashed") hasCrashed = value == "true";
    }
    file.close();

    // Load the current timetable
    if (g_currentTimetable.name != "")
    {
        g_currentTimetable.Load("templates/" + g_currentTimetable.name + ".json");
    }

    // Load the language
    GetAllLanguages();
    ReloadLabels();

    // Read the version
    std::ifstream versionFile("version.txt");
    std::getline(versionFile, g_version);
    versionFile.close();
}
