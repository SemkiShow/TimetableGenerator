// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Timetable.hpp"
#include <Settings.hpp>
#include <climits>
#include <cstddef>
#include <unordered_map>
#include <vector>

struct IterationData
{
    // Iteration-specific data
    int iteration = -1;
    int iterationsPerChange = -1;
    int lastAllTimeBestScore = -1;
    int lastBestScore = -1;
    int minErrors = INT_MAX;
    int maxErrors = INT_MIN;
    int bestTimetableIndex = -1;
    int bestScore = INT_MIN;
    int allTimeBestScore = bestScore;
    int timetablesPerGeneration = -1;
    bool isDone = true;
    int startBonusPoints = INT_MAX;
    int maxBonusPoints = INT_MIN;
    std::unordered_map<int, std::vector<std::vector<TimetableLessonRule>>> classRuleVariants;
    static constexpr size_t ERROR_VALUES_SIZE = 1000;
    float errorValues[ERROR_VALUES_SIZE];

    // Settings copy (I can't use the real settings data, because if settings are changed while
    // searching for a timetable, the program crashes)
    int daysPerWeek = Settings::DEFAULT_DAYS_PER_WEEK;
    int lessonsPerDay = Settings::DEFAULT_LESSONS_PER_DAY;
    int minTimetablesPerGeneration = Settings::DEFAULT_MIN_TIMETABLES_PER_GENERATION;
    int maxTimetablesPerGeneration = Settings::DEFAULT_MAX_TIMETABLES_PER_GENERATION;

    // Timetables used for searching
    std::vector<Timetable> timetables, population, newPopulation;
};

extern IterationData g_iterationData;
extern size_t g_threadsNumber;

std::vector<TimetableLessonRule> GetAllRuleVariants(const TimetableLessonRule& timetableLessonRule);
void ScoreTimetable(Timetable& timetable);
void BeginSearching(const Timetable& timetable);
void RunASearchIteration();
void StopSearching();
void ToggleVerboseLoggingThreads();
