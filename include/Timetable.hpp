#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <random>
#include <ctime>
#include <filesystem>

struct WorkTime
{
    std::vector<int> lessonNumbers;
};

struct Classroom
{
    std::string name;
};

struct Lesson
{
    std::string id = "";
    std::string name = "";
    std::vector<std::string> classNames;
    std::vector<Classroom*> classrooms;
};

struct Teacher
{
    std::string name = "";
    std::vector<Lesson*> lessons;
    WorkTime workTime[7];
};

struct TimetableLesson
{
    std::vector<Lesson*> lessons;
    std::vector<Teacher*> teachers;
};

struct Day
{
    std::vector<TimetableLesson> lessons;
};

struct Class
{
    std::string number = "0";
    std::string letter = "";
    Teacher* teacher;
    Day days[7];
};

struct Timetable
{
    std::string name = "";
    std::vector<Classroom> classrooms;
    std::vector<Lesson> lessons;
    std::vector<Teacher> teachers;
    std::vector<Class> classes;
};

extern Timetable currentTimetable;
extern Timetable tmpTimetable;
extern Timetable tmpTmpTimetable;

void SaveTimetable(std::string path, Timetable* timetable);
void LoadTimetable(std::string path, Timetable* timetable);
void GenerateRandomTimetable(Timetable* timetable);
