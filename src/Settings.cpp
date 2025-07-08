#include "Settings.hpp"
#include "Crashes.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "UI.hpp"
#include "Updates.hpp"
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

int daysPerWeek = 5;
int lessonsPerDay = 8;
int style = STYLE_DARK;
std::string language = "en";
bool vsync = true;
bool mergedFont = false;
int timetableAutosaveInterval = 60;
int fontSize = DEFAULT_FONT_SIZE;
int minFreePeriods = 0;
int maxFreePeriods = 0;
float errorBonusRatio = 10.0f;
int timetablesPerGenerationStep = 10;
int minTimetablesPerGeneration = 100;
int maxTimetablesPerGeneration = 10000;
int maxIterations = -1;
int additionalBonusPoints = 1;
bool verboseLogging = false;
bool usePrereleases = false;
std::string lastCAUpdate = "";

std::string version = "";
std::unordered_map<std::string, std::string> labels;
std::vector<std::string> availableLanguages;
std::string languageValues;
int languageID = -1;
std::string styleValues = "";

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
    std::sort(files->begin(), files->end());
}

std::string TrimJunk(const std::string& input)
{
    auto first = input.find_first_not_of("\t\n\r\f\v");
    auto last = input.find_last_not_of("\t\n\r\f\v");
    return (first == input.npos) ? "" : input.substr(first, last - first + 1);
}

void ReloadLabels()
{
    // Read the language file
    LogInfo("Reloading labels");
    labels.clear();
    std::ifstream languageFile("resources/languages/" + language + ".txt");
    std::ifstream englishFile("resources/languages/en.txt");
    std::string languageBuffer;
    std::string englishBuffer;
    while (true)
    {
        if (!std::getline(languageFile, languageBuffer)) break;
        if (!std::getline(englishFile, englishBuffer)) break;
        labels[englishBuffer] = languageBuffer;
    }
    languageFile.close();

    // Assign translated week days
    weekDays[0] = labels["Monday"];
    weekDays[1] = labels["Tuesday"];
    weekDays[2] = labels["Wednesday"];
    weekDays[3] = labels["Thursday"];
    weekDays[4] = labels["Friday"];
    weekDays[5] = labels["Saturday"];
    weekDays[6] = labels["Sunday"];

    // Assign translated style values
    styleValues = "";
    styleValues += labels["dark"];
    styleValues += '\0';
    styleValues += labels["light"];
    styleValues += '\0';
    styleValues += labels["classic"];
    styleValues += '\0';
    styleValues += '\0';

    // Assign translated wizard texts
    wizardTexts[0] = labels["Welcome to the TimetableGenerator setup wizard!"] + "\n\n" +
                     labels["The first step is to setup classrooms."] + "\n" +
                     labels["After you are done, press Ok and continue to the next step."];
    wizardTexts[1] = labels["The next step is to add classes."] + "\n" +
                     labels["After you are done, press Ok and continue to the next step."];
    wizardTexts[2] = labels["The next step is to add lessons."] + "\n" +
                     labels["After you are done, press Ok and continue to the next step."];
    wizardTexts[3] = labels["The next step is to add teachers."] + "\n" +
                     labels["After you are done, press Ok and continue to the next step."];
    wizardTexts[4] = labels["The next step is to assign lessons to classes."] + "\n" +
                     labels["After you are done, press Ok and continue to the next step."];
    wizardTexts[5] = labels["You are done! Now press the Generate timetable"] + "\n" +
                     labels["button to begin the timetable finding process!"];
}

void Save(std::string fileName)
{
    // Read the file
    LogInfo("Saving settings");
    std::fstream settingsFile;
    settingsFile.open(fileName, std::ios::out);
    settingsFile << "last-timetable=" << currentTimetable.name << '\n';
    settingsFile << "days-per-week=" << daysPerWeek << '\n';
    settingsFile << "lessons-per-day=" << lessonsPerDay << '\n';
    settingsFile << "style=" << style << '\n';
    settingsFile << "language=" << language << '\n';
    settingsFile << "min-free-periods=" << minFreePeriods << '\n';
    settingsFile << "max-free-periods=" << maxFreePeriods << '\n';
    settingsFile << "vsync=" << (vsync ? "true" : "false") << '\n';
    settingsFile << "merged-font=" << (mergedFont ? "true" : "false") << '\n';
    settingsFile << "timetable-autosave-interval=" << timetableAutosaveInterval << '\n';
    settingsFile << "font-size=" << fontSize << '\n';
    settingsFile << "error-bonus-ratio=" << errorBonusRatio << '\n';
    settingsFile << "timetables-per-generation-step=" << timetablesPerGenerationStep << '\n';
    settingsFile << "min-timetables-per-generation=" << minTimetablesPerGeneration << '\n';
    settingsFile << "max-timetables-per-generation=" << maxTimetablesPerGeneration << '\n';
    settingsFile << "max-iterations=" << maxIterations << '\n';
    settingsFile << "additional-bonus-points=" << additionalBonusPoints << '\n';
    settingsFile << "verbose-logging=" << (verboseLogging ? "true" : "false") << '\n';
    settingsFile << "use-prereleases=" << (usePrereleases ? "true" : "false") << '\n';
    settingsFile << "last-ca-update=" << lastCAUpdate << '\n';
    settingsFile << "has-crashed=" << (hasCrashed ? "true" : "false") << '\n';
    settingsFile.close();

    // Save timetable
    SaveTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
}

void Load(std::string fileName)
{
    // Read the file
    LogInfo("Loading settings");
    std::fstream settingsFile;
    settingsFile.open(fileName, std::ios::in);
    std::string buf, label, value;
    while (std::getline(settingsFile, buf))
    {
        if (Split(buf, '=').size() < 2)
        {
            std::cout << "Error: invalid settings.txt!\n";
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
        if (label == "language") language = value;
        if (label == "min-free-periods") minFreePeriods = stoi(value);
        if (label == "max-free-periods") maxFreePeriods = stoi(value);
        if (label == "vsync") vsync = value == "true";
        if (label == "merged-font") mergedFont = value == "true";
        if (label == "timetable-autosave-interval") timetableAutosaveInterval = stoi(value);
        if (label == "font-size") fontSize = stoi(value);
        if (label == "timetables-per-generation-step") timetablesPerGenerationStep = stoi(value);
        if (label == "min-timetables-per-generation") minTimetablesPerGeneration = stoi(value);
        if (label == "max-timetables-per-generation") maxTimetablesPerGeneration = stoi(value);
        if (label == "error-bonus-ratio") errorBonusRatio = stof(value);
        if (label == "max-iterations") maxIterations = stoi(value);
        if (label == "additional-bonus-points") additionalBonusPoints = stoi(value);
        if (label == "verbose-logging") verboseLogging = value == "true";
        if (label == "use-prereleases") usePrereleases = value == "true";
        if (label == "last-ca-update") lastCAUpdate = value;
        if (label == "has-crashed") hasCrashed = value == "true";
    }
    settingsFile.close();

    // Load the current timetable
    if (currentTimetable.name != "")
    {
        LoadTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
    }

    // Load the language
    ListFiles("resources/languages", &availableLanguages);
    for (int i = 0; i < availableLanguages.size(); i++)
    {
        availableLanguages[i] = std::filesystem::path(availableLanguages[i]).stem().string();
    }
    std::sort(availableLanguages.begin(), availableLanguages.end());
    languageValues = "";
    languageID = -1;
    for (int i = 0; i < availableLanguages.size(); i++)
    {
        languageValues += availableLanguages[i];
        languageValues += '\0';
        if (availableLanguages[i] == language) languageID = i;
    }
    languageValues += '\0';
    if (languageID == -1) languageID = 0;
    ReloadLabels();

    // Update the CA certificate, if necessary (done monthly)
    if (lastCAUpdate == "")
    {
        time_t now = time(0);
        lastCAUpdate = asctime(localtime(&now));
        UpdateCACertificate();
    }
    else
    {
        struct tm parsedTm;
        std::istringstream ss(lastCAUpdate);
        ss >> std::get_time(&parsedTm, "%a %b %d %H:%M:%S %Y");
        time_t reconstructedTime = mktime(&parsedTm);
        if (difftime(time(0), reconstructedTime) > 60 * 60 * 24 * 30)
        {
            UpdateCACertificate();
        }
    }

    // Read the version
    std::ifstream versionFile("version.txt");
    std::getline(versionFile, version);
    versionFile.close();
}
