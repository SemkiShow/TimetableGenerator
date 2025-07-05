#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#define STYLE_DARK 0
#define STYLE_LIGHT 1
#define STYLE_CLASSIC 2

extern bool vsync;
extern bool mergedFont;
extern int timetableAutosaveInterval;
extern int lessonsPerDay;
extern int fontSize;
extern int minFreePeriods;
extern int maxFreePeriods;
extern int timetablesPerGenerationStep;
extern int minTimetablesPerGeneration;
extern int maxTimetablesPerGeneration;
extern float errorBonusRatio;
extern int daysPerWeek;
extern int maxIterations;
extern bool verboseLogging;
extern int style;
extern bool usePrereleases;
extern std::string language;
extern std::string lastCAUpdate;

extern std::string version;
extern std::unordered_map<std::string, std::string> labels;
extern std::vector<std::string> availableLanguages;
extern std::string languageValues;
extern int languageID;
extern std::string styleValues;

std::vector<std::string> Split(std::string input, char delimiter = ' ');
void ListFiles(const std::string& path, std::vector<std::string>* files);
std::string TrimJunk(const std::string& input);
void ReloadLabels();
void Save(std::string fileName);
void Load(std::string fileName);
