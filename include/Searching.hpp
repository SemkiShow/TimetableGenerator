#pragma once

#include "Timetable.hpp"
#include <limits.h>

struct IterationData
{
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
    bool threadLock = true;
    Timetable* timetables = nullptr;
    Timetable* population = nullptr;
    Timetable* newPopulation = nullptr;
};

extern IterationData iterationData;
extern unsigned int threadsNumber;

void ScoreTimetable(Timetable* timetable);
void BeginSearching(const Timetable* timetable);
void RunASearchIteration();
void StopSearching();
