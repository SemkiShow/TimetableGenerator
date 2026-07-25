// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "UI.hpp"
#include <string>

enum class Style;

struct Settings
{
    static constexpr const char* const SETTINGS_PATH = "settings.txt";
    static constexpr int DEFAULT_DAYS_PER_WEEK = 5;
    static constexpr int DEFAULT_LESSONS_PER_DAY = 8;
    static constexpr Style DEFAULT_STYLE = Style::Dark;
    static constexpr const char* const DEFAULT_LANGUAGE = "en";
    static constexpr bool DEFAULT_VSYNC = true;
    static constexpr bool DEFAULT_MERGED_FONT = true;
    static constexpr int DEFAULT_AUTOSAVE_INTERVAL = 60;
    static constexpr int DEFAULT_FONT_SIZE = 16;
    static constexpr int DEFAULT_MIN_FREE_PERIODS = 0;
    static constexpr int DEFAULT_MAX_FREE_PERIODS = 0;
    static constexpr float DEFAULT_ERROR_BONUS_RATIO = 10;
    static constexpr int DEFAULT_TIMETABLES_PER_GENERATION_STEP = 10;
    static constexpr int DEFAULT_MIN_TIMETABLES_PER_GENERATION = 100;
    static constexpr int DEFAULT_MAX_TIMETABLES_PER_GENERATION = 5000;
    static constexpr int DEFAULT_MAX_ITERATIONS = -1;
    static constexpr int DEFAULT_ADDITIONAL_BONUS_POINTS = 1;
    static constexpr bool DEFAULT_VERBOSE_LOGGING = false;
    static constexpr bool DEFAULT_USE_PRERELEASES = false;
    static constexpr bool DEFAULT_HAS_CRASHED = false;

    int daysPerWeek = DEFAULT_DAYS_PER_WEEK;
    int lessonsPerDay = DEFAULT_LESSONS_PER_DAY;
    Style style = DEFAULT_STYLE;
    std::string language = DEFAULT_LANGUAGE;
    bool vsync = DEFAULT_VSYNC;
    bool mergedFont = DEFAULT_MERGED_FONT;
    int autosaveInterval = DEFAULT_AUTOSAVE_INTERVAL;
    int fontSize = DEFAULT_FONT_SIZE;
    int minFreePeriods = DEFAULT_MIN_FREE_PERIODS;
    int maxFreePeriods = DEFAULT_MAX_FREE_PERIODS;
    float errorBonusRatio = DEFAULT_ERROR_BONUS_RATIO;
    int timetablesPerGenerationStep = DEFAULT_TIMETABLES_PER_GENERATION_STEP;
    int minTimetablesPerGeneration = DEFAULT_MIN_TIMETABLES_PER_GENERATION;
    int maxTimetablesPerGeneration = DEFAULT_MAX_TIMETABLES_PER_GENERATION;
    int maxIterations = DEFAULT_MAX_ITERATIONS;
    int additionalBonusPoints = DEFAULT_ADDITIONAL_BONUS_POINTS;
    bool verboseLogging = DEFAULT_VERBOSE_LOGGING;
    bool usePrereleases = DEFAULT_USE_PRERELEASES;
    bool hasCrashed = DEFAULT_HAS_CRASHED;

    void Save() const;
    void Load();
};

extern Settings g_settings;

extern std::string g_version;
