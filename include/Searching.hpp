#pragma once

#include <algorithm> // IWYU pragma: keep
#include <random> // IWYU pragma: keep
#include <ctime>
#include <unordered_map> // IWYU pragma: keep
#include <limits.h>
#include <iostream> // IWYU pragma: keep
#include <thread> // IWYU pragma: keep
#include <mutex> // IWYU pragma: keep
#include <assert.h>
#include "Timetable.hpp"

struct IterationData
{
    int iteration = 0;
    int iterationsPerChange = 0;
    int lastAllTimeBestScore = 0;
    int lastBestScore = 0;
    int minErrors = INT_MAX;
    int bestTimetableIndex = 0;
    int bestScore = INT_MIN;
    int allTimeBestScore = bestScore;
    double temperature = 0;
    bool isDone = true;
    Timetable* timetables;
    Timetable* population;
    Timetable* newPopulation;
};

extern IterationData iterationData;

void ScoreTimetable(Timetable* timetable);
void BeginSearching(const Timetable* timetable);
void RunASearchIteration();
