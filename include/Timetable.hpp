#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <random>
#include <ctime>
#include <filesystem>
#include <map>

struct WorkDay
{
    std::vector<int> lessonNumbers;
};

struct Classroom
{
    std::string name;
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
    WorkDay workDays[7];
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

struct Day
{
    std::vector<int> lessonNumbers;
    std::vector<int> timetableLessonIDs;
};

struct Class
{
    std::string number = "-1";
    std::string letter = "";
    int teacherID = -1;
    std::map<int, TimetableLesson> timetableLessons;
    Day days[7];
};

struct Timetable
{
    std::string name = "";
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
std::string ExtractNumberFromBeginning(std::string input);
void LoadTimetable(std::string path, Timetable* timetable);
void GenerateRandomTimetable(Timetable* timetable);
