#include "Settings.hpp"
#include "Timetable.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

bool vsync = true;
bool mergedFont = false;
int timetableAutosaveInterval = 60;
int lessonsPerDay = 8;
int fontSize = 16;
int minFreePeriods = 0;
int maxFreePeriods = 0;
int timetablesPerGeneration = 2000;
int maxMutations = 100;
float errorBonusRatio = 10.0f;
int daysPerWeek = 5;
int maxIterations = -1;
bool verboseLogging = false;
int style = STYLE_DARK;
bool usePrereleases = false;

std::string version = "";

std::vector<std::string> Split(std::string input, char delimiter)
{
    std::vector<std::string> output;
    output.push_back("");
    int index = 0;
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == delimiter)
        {
            index++;
            output.push_back("");
            continue;
        }
        output[index] += input[i];
    }
    return output;
}

void ListFiles(const std::string& path, std::vector<std::string>* files)
{
    files->clear();
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (std::filesystem::is_regular_file(entry.path()))
        {
            files->push_back(entry.path().string());
        }
    }
}

std::string TrimJunk(const std::string& input)
{
    auto first = input.find_first_not_of("\t\n\r\f\v");
    auto last  = input.find_last_not_of ("\t\n\r\f\v");
    return (first == input.npos) ? "" : input.substr(first, last-first+1);
}

void Save(std::string fileName)
{
    // Read the file
    std::fstream settingsFile;
    settingsFile.open(fileName, std::ios::out);
    settingsFile << "last-timetable=" << currentTimetable.name << '\n';
    settingsFile << "days-per-week=" << daysPerWeek << '\n';
    settingsFile << "lessons-per-day=" << lessonsPerDay << '\n';
    settingsFile << "style=" << style << '\n';
    settingsFile << "min-free-periods=" << minFreePeriods << '\n';
    settingsFile << "max-free-periods=" << maxFreePeriods << '\n';
    settingsFile << "vsync=" << (vsync ? "true" : "false") << '\n';
    settingsFile << "merged-font=" << (mergedFont ? "true" : "false") << '\n';
    settingsFile << "timetable-autosave-interval=" << timetableAutosaveInterval << '\n';
    settingsFile << "font-size=" << fontSize << '\n';
    settingsFile << "max-mutations=" << maxMutations << '\n';
    settingsFile << "error-bonus-ratio=" << errorBonusRatio << '\n';
    settingsFile << "timetables-per-generation=" << timetablesPerGeneration << '\n';
    settingsFile << "max-iterations=" << maxIterations << '\n';
    settingsFile << "verbose-logging=" << (verboseLogging ? "true" : "false") << '\n';
    settingsFile << "use-prereleases=" << (usePrereleases ? "true" : "false") << '\n';
    settingsFile.close();

    // Save timetable
    SaveTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
}

void Load(std::string fileName)
{
    // Read the file
    std::fstream settingsFile;
    settingsFile.open(fileName, std::ios::in);
    std::string buf, label, value;
    while (std::getline(settingsFile, buf))
    {
        if (Split(buf, '=').size() < 2)
        {
            std::cout << "Error: invalid setings.txt!\n";
            continue;
        }
        label = TrimJunk(Split(buf, '=')[0]);
        value = TrimJunk(Split(buf, '=')[1]);

        if (label == "last-timetable" && value != "")
        {
            currentTimetable = Timetable();
            currentTimetable.name = value;
        }
        if (label == "days-per-week") daysPerWeek = stoi(value);
        if (label == "lessons-per-day") lessonsPerDay = stoi(value);
        if (label == "style") style = stoi(value);
        if (label == "min-free-periods") minFreePeriods = stoi(value);
        if (label == "max-free-periods") maxFreePeriods = stoi(value);
        if (label == "vsync") vsync = value == "true";
        if (label == "merged-font") mergedFont = value == "true";
        if (label == "timetable-autosave-interval") timetableAutosaveInterval = stoi(value);
        if (label == "font-size") fontSize = stoi(value);
        if (label == "timetables-per-generation") timetablesPerGeneration = stoi(value);
        if (label == "max-mutations") maxMutations = stoi(value);
        if (label == "error-bonus-ratio") errorBonusRatio = stof(value);
        if (label == "max-iterations") maxIterations = stoi(value);
        if (label == "verbose-logging") verboseLogging = value == "true";
        if (label == "use-prereleases") usePrereleases = value == "true";
    }
    settingsFile.close();

    if (currentTimetable.name != "")
    {
        LoadTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
    }

    std::ifstream versionFile("version.txt");
    std::getline(versionFile, version);
    versionFile.close();
}
