#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

extern bool vsync;
extern bool mergedFont;
extern int timetableAutosaveInterval;
extern int lessonsPerDay;
extern int fontSize;
extern int minFreePeriods;
extern int maxFreePeriods;

extern std::string version;

extern std::vector<std::string> timetables;
extern std::vector<std::string> templates;

std::vector<std::string> Split(std::string input, char delimiter = ' ');
void ListFiles(const std::string& path, std::vector<std::string>* files);
void Save(std::string fileName);
void Load(std::string fileName);
