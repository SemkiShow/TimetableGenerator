// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Settings.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "Updates.hpp"
#include "Utils.hpp"
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

Settings settings;

std::string version = "";

void Settings::Save()
{
    // Read the file
    LogInfo("Saving settings");
    std::fstream settingsFile;
    settingsFile.open("settings.txt", std::ios::out);
    settingsFile << "last-timetable=" << currentTimetable.name << '\n';
    settingsFile << "days-per-week=" << daysPerWeek << '\n';
    settingsFile << "lessons-per-day=" << lessonsPerDay << '\n';
    settingsFile << "style=" << static_cast<int>(style) << '\n';
    settingsFile << "language=" << language << '\n';
    settingsFile << "min-free-periods=" << minFreePeriods << '\n';
    settingsFile << "max-free-periods=" << maxFreePeriods << '\n';
    settingsFile << "vsync=" << (vsync ? "true" : "false") << '\n';
    settingsFile << "merged-font=" << (mergedFont ? "true" : "false") << '\n';
    settingsFile << "timetable-autosave-interval=" << timetableAutosaveInterval << '\n';
    settingsFile << "font-size=" << fontSize << '\n';
    settingsFile << "error-bonus-ratio=" << errorBonusRatio << '\n';
    settingsFile << "timetables-per-generation-step=" << timetablesPerGenerationStep << '\n';
    settingsFile << "min-timetables-per-generation=" << minTimetablesPerGeneration << '\n';
    settingsFile << "max-timetables-per-generation=" << maxTimetablesPerGeneration << '\n';
    settingsFile << "max-iterations=" << maxIterations << '\n';
    settingsFile << "additional-bonus-points=" << additionalBonusPoints << '\n';
    settingsFile << "verbose-logging=" << (verboseLogging ? "true" : "false") << '\n';
    settingsFile << "use-prereleases=" << (usePrereleases ? "true" : "false") << '\n';
    settingsFile << "last-ca-update=" << lastCAUpdate << '\n';
    settingsFile << "has-crashed=" << (hasCrashed ? "true" : "false") << '\n';
    settingsFile.close();

    // Save timetable
    currentTimetable.Save("templates/" + currentTimetable.name + ".json");
}

void Settings::Load()
{
    // Read the file
    LogInfo("Loading settings");
    std::fstream settingsFile;
    settingsFile.open("settings.txt", std::ios::in);
    std::string buf, label, value;
    while (std::getline(settingsFile, buf))
    {
        if (Split(buf, '=').size() < 2)
        {
            LogError("Invalid settings.txt!");
            continue;
        }
        label = TrimJunk(Split(buf, '=')[0]);
        value = TrimJunk(Split(buf, '=')[1]);

        if (label == "last-timetable" && value != "")
        {
            currentTimetable = Timetable();
            currentTimetable.name = value;
        }
        if (label == "days-per-week") daysPerWeek = stoi(value);
        if (label == "lessons-per-day") lessonsPerDay = stoi(value);
        if (label == "style") style = static_cast<Style>(stoi(value));
        if (label == "language") language = value;
        if (label == "min-free-periods") minFreePeriods = stoi(value);
        if (label == "max-free-periods") maxFreePeriods = stoi(value);
        if (label == "vsync") vsync = value == "true";
        if (label == "merged-font") mergedFont = value == "true";
        if (label == "timetable-autosave-interval") timetableAutosaveInterval = stoi(value);
        if (label == "font-size") fontSize = stoi(value);
        if (label == "timetables-per-generation-step") timetablesPerGenerationStep = stoi(value);
        if (label == "min-timetables-per-generation") minTimetablesPerGeneration = stoi(value);
        if (label == "max-timetables-per-generation") maxTimetablesPerGeneration = stoi(value);
        if (label == "error-bonus-ratio") errorBonusRatio = stof(value);
        if (label == "max-iterations") maxIterations = stoi(value);
        if (label == "additional-bonus-points") additionalBonusPoints = stoi(value);
        if (label == "verbose-logging") verboseLogging = value == "true";
        if (label == "use-prereleases") usePrereleases = value == "true";
        if (label == "last-ca-update") lastCAUpdate = value;
        if (label == "has-crashed") hasCrashed = value == "true";
    }
    settingsFile.close();

    // Load the current timetable
    if (currentTimetable.name != "")
    {
        currentTimetable.Load("templates/" + currentTimetable.name + ".json");
    }

    // Load the language
    GetAllLanguages();
    ReloadLabels();

    // Update the CA certificate, if necessary (done monthly)
    struct tm parsedTm;
    std::istringstream ss(lastCAUpdate);
    ss >> std::get_time(&parsedTm, "%a %b %d %H:%M:%S %Y");
    time_t reconstructedTime = mktime(&parsedTm);
    if (reconstructedTime < 0 || difftime(time(0), reconstructedTime) > 60 * 60 * 24 * 30)
    {
        UpdateCACertificate();
    }

    // Read the version
    std::ifstream versionFile("version.txt");
    std::getline(versionFile, version);
    versionFile.close();
}
