#pragma once

#include <string>
#include <vector>
#include <fstream> // IWYU pragma: keep
#include <filesystem> // IWYU pragma: keep

extern bool vsync;
extern bool mergedFont;
extern int timetableAutosaveInterval;
extern int lessonsPerDay;
extern int fontSize;
extern int minFreePeriods;
extern int maxFreePeriods;
extern int maxTemperature;
extern int timetablesPerGeneration;
extern int maxMutations;
extern float coolingRate;
extern float errorBonusRatio;

extern std::string version;

extern std::vector<std::string> timetables;
extern std::vector<std::string> templates;

std::vector<std::string> Split(std::string input, char delimiter = ' ');
void ListFiles(const std::string& path, std::vector<std::string>* files);
void Save(std::string fileName);
void Load(std::string fileName);
