#include "Settings.hpp"
#include "UI.hpp"
#include "Timetable.hpp"

bool vsync = true;

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

void Save(std::string fileName)
{
    // Read the file
    std::fstream settingsFile;
    settingsFile.open(fileName, std::ios::out);
    settingsFile << "vsync=" << (vsync ? "true" : "false") << '\n';
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
        label = Split(buf, '=')[0];
        value = Split(buf, '=')[1];

        if (label == "vsync") vsync = value == "true";
        if (label == "last-timetable" && value != "")
        {
            currentTimetable = Timetable();
            LoadTimetable("templates/" + value + ".json", &currentTimetable);
        }
    }
    settingsFile.close();
}
