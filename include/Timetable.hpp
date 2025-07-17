#pragma once

#include <map>
#include <string>
#include <vector>

#define ANY_LESSON -2
#define NO_LESSON -3

struct WorkDay
{
    std::vector<int> lessonIDs;
};

struct Classroom
{
    std::string name = "";
};

struct Lesson
{
    std::string name = "";
    std::vector<int> classIDs;
    std::vector<int> classroomIDs;
};

struct Teacher
{
    std::string name = "";
    std::vector<int> lessonIDs;
    std::vector<WorkDay> workDays;
};

struct LessonTeacherPair
{
    int lessonID = -1;
    int teacherID = -1;
};

struct TimetableLesson
{
    int amount = 1;
    std::vector<LessonTeacherPair> lessonTeacherPairs;
};

struct ClassroomLessonPair
{
    int timetableLessonID = -1;
    std::vector<int> classroomIDs;
};

struct Day
{
    std::vector<bool> lessons;
    std::vector<ClassroomLessonPair> classroomLessonPairs;
};

struct TimetableLessonRule
{
    bool preserveOrder = false;
    int amount = 1;
    std::vector<int> timetableLessonIDs;
};

struct Class
{
    std::string number = "";
    std::string letter = "";
    int teacherID = -1;
    int maxTimetableLessonID = -1;
    std::map<int, TimetableLesson> timetableLessons;
    std::vector<Day> days;
    std::vector<TimetableLessonRule> timetableLessonRules;
};

struct Timetable
{
    std::string name = "";
    int year = -1;
    int errors = -1;
    int bonusPoints = -1;
    int maxClassroomID = -1;
    int maxLessonID = -1;
    int maxTeacherID = -1;
    int maxClassID = -1;
    std::map<int, Classroom> classrooms;
    std::map<int, Lesson> lessons;
    std::map<int, Teacher> teachers;
    std::map<int, Class> classes;
    std::vector<int> orderedClasses;
};

extern Timetable currentTimetable;
extern Timetable tmpTimetable;
extern Timetable tmpTmpTimetable;

void SaveTimetable(std::string path, Timetable* timetable);
void LoadTimetable(std::string path, Timetable* timetable);
void GenerateRandomTimetable(Timetable* timetable);
void ExportTimetableAsXlsx(Timetable* timetable);
