#include <string>
#include <vector>
#include <fstream>
#include <iostream>

struct WorkTime
{
    std::vector<int> lessonNumbers;
};

struct Teacher
{
    std::string name = "";
    WorkTime workTime[7];
};

struct Lesson
{
    std::string name = "";
    Teacher* teacher;
};

struct Day
{
    std::vector<Lesson> lessons;
};

struct Class
{
    std::string name = "";
    Teacher* teacher;
    Day days[7];
};

struct Timetable
{
    std::vector<Class> classes;
    std::vector<Teacher> teachers;
};

void SaveTimetable(std::string fileName, Timetable* timetable);
void LoadTimetable(std::string fileName, Timetable* timetable);
