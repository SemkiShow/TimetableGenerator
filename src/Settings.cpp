#include "Settings.hpp"
#include "Timetable.hpp"

bool vsync = true;
bool mergedFont = false;
int timetableAutosaveInterval = 60;
int lessonsPerDay = 8;
int fontSize = 16;
int minFreePeriods = 0;
int maxFreePeriods = 0;
int timetablesPerGeneration = 500;
int maxMutations = 100;
float errorBonusRatio = 10.0f;
int daysPerWeek = 5;
int maxIterations = -1;
bool verboseLogging = false;

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
    settingsFile << "days-per-week=" << daysPerWeek << '\n';
    settingsFile << "lessons-per-day=" << lessonsPerDay << '\n';
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
    // !IMPORTANT: last-timetable must ALWAYS be last because of the quirks of the timetable loading
    settingsFile << "last-timetable=" << currentTimetable.name << '\n';
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

        if (label == "vsync") vsync = value == "true";
        if (label == "last-timetable" && value != "")
        {
            currentTimetable = Timetable();
            LoadTimetable("templates/" + value + ".json", &currentTimetable);
        }
        if (label == "merged-font") mergedFont = value == "true";
        if (label == "timetable-autosave-interval") timetableAutosaveInterval = stoi(value);
        if (label == "lessons-per-day") lessonsPerDay = stoi(value);
        if (label == "font-size") fontSize = stoi(value);
        if (label == "min-free-periods") minFreePeriods = stoi(value);
        if (label == "max-free-periods") maxFreePeriods = stoi(value);
        if (label == "timetables-per-generation") timetablesPerGeneration = stoi(value);
        if (label == "max-mutations") maxMutations = stoi(value);
        if (label == "error-bonus-ratio") errorBonusRatio = stof(value);
        if (label == "days-per-week") daysPerWeek = stoi(value);
        if (label == "max-iterations") maxIterations = stoi(value);
        if (label == "verbose-logging") verboseLogging = value == "true";
    }
    settingsFile.close();

    std::ifstream versionFile("version.txt");
    std::getline(versionFile, version);
    versionFile.close();
}
