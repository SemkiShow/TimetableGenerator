#pragma once

#include "Timetable.hpp"
#include <limits.h>

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
extern unsigned int threadsNumber;

void ScoreTimetable(Timetable* timetable);
void BeginSearching(const Timetable* timetable);
void RunASearchIteration();
