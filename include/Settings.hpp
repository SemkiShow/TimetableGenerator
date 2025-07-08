#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#define STYLE_DARK 0
#define STYLE_LIGHT 1
#define STYLE_CLASSIC 2

#define DEFAULT_FONT_SIZE 16

extern int daysPerWeek;
extern int lessonsPerDay;
extern int style;
extern std::string language;
extern bool vsync;
extern bool mergedFont;
extern int timetableAutosaveInterval;
extern int fontSize;
extern int minFreePeriods;
extern int maxFreePeriods;
extern int timetablesPerGenerationStep;
extern int minTimetablesPerGeneration;
extern int maxTimetablesPerGeneration;
extern float errorBonusRatio;
extern int maxIterations;
extern int additionalBonusPoints;
extern bool verboseLogging;
extern bool usePrereleases;
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
