// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "UI.hpp"
#include <string>

enum class Style;

struct Settings
{
    unsigned int daysPerWeek = 5;
    unsigned int lessonsPerDay = 8;
    Style style = Style::Dark;
    std::string language = "en";
    bool vsync = true;
    bool mergedFont = true;
    int timetableAutosaveInterval = 60;
    int fontSize = DEFAULT_FONT_SIZE;
    int minFreePeriods = 0;
    int maxFreePeriods = 0;
    float errorBonusRatio = 10.0f;
    int timetablesPerGenerationStep = 10;
    int minTimetablesPerGeneration = 100;
    int maxTimetablesPerGeneration = 5000;
    int maxIterations = -1;
    int additionalBonusPoints = 1;
    bool verboseLogging = false;
    bool usePrereleases = false;
    std::string lastCAUpdate = "";
    bool hasCrashed = false;

    void Save();
    void Load();
};

extern Settings settings;

extern std::string version;
