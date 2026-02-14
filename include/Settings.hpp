// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

enum class Style;

extern unsigned int daysPerWeek;
extern unsigned int lessonsPerDay;
extern Style style;
extern std::string language;
extern bool vsync;
extern bool mergedFont;
extern int timetableAutosaveInterval;
extern int fontSize;
extern int minFreePeriods;
extern int maxFreePeriods;
extern float errorBonusRatio;
extern int timetablesPerGenerationStep;
extern int minTimetablesPerGeneration;
extern int maxTimetablesPerGeneration;
extern int maxIterations;
extern int additionalBonusPoints;
extern bool verboseLogging;
extern bool usePrereleases;
extern std::string lastCAUpdate;

extern std::string version;

void Save(std::string fileName);
void Load(std::string fileName);
