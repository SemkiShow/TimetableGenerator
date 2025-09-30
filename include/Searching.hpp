// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Timetable.hpp"
#include <limits.h>
#include <unordered_map>

// Preventing g++ from aggressively optimizing this loop, which frees
// timetables before the search iteration finishes, which leads to a segmentation fault
#if defined(__GNUC__) || defined(__clang__)
#define COMPILER_BARRIER() asm volatile("" ::: "memory")
#elif defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(_ReadWriteBarrier)
#define COMPILER_BARRIER() _ReadWriteBarrier()
#else
#error "Unsupported compiler"
#endif

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
    bool threadLock = false;
    int startBonusPoints = INT_MAX;
    int maxBonusPoints = INT_MIN;
    std::unordered_map<int, std::vector<std::vector<TimetableLessonRule>>> classRuleVariants;
    const static size_t errorValuesPoints = 1000;
    float errorValues[errorValuesPoints];

    // Settings copy (I can't use the real settings data, because
    // if settings are changed while searching for a timetable, the program crashes)
    unsigned int daysPerWeek = 5;
    unsigned int lessonsPerDay = 8;
    int minTimetablesPerGeneration = 100;
    int maxTimetablesPerGeneration = 5000;

    // Timetables used for searching
    Timetable* timetables = nullptr;
    Timetable* population = nullptr;
    Timetable* newPopulation = nullptr;
};

extern IterationData iterationData;
extern unsigned int threadsNumber;

std::vector<TimetableLessonRule> GetAllRuleVariants(const TimetableLessonRule timetableLessonRule);
void ScoreTimetable(Timetable* timetable);
void BeginSearching(const Timetable* timetable);
void RunASearchIteration();
void StopSearching();
