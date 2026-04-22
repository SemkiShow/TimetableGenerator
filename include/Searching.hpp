// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Timetable.hpp"
#include <cstddef>
#include <limits.h>
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
    const static size_t errorValuesPoints = 1000;
    float errorValues[errorValuesPoints];

    // Settings copy (I can't use the real settings data, because if settings are changed while
    // searching for a timetable, the program crashes)
    unsigned int daysPerWeek = 5;
    unsigned int lessonsPerDay = 8;
    int minTimetablesPerGeneration = 100;
    int maxTimetablesPerGeneration = 5000;

    // Timetables used for searching
    std::vector<Timetable> timetables, population, newPopulation;
};

extern IterationData iterationData;
extern size_t threadsNumber;

std::vector<TimetableLessonRule> GetAllRuleVariants(const TimetableLessonRule timetableLessonRule);
void ScoreTimetable(Timetable& timetable);
void BeginSearching(const Timetable& timetable);
void RunASearchIteration();
void StopSearching();
void ToggleVerboseLoggingThreads();
