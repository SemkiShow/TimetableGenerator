// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

class Json;

#define ANY_LESSON -2
#define NO_LESSON -3

struct WorkDay
{
    std::vector<int> lessonIds;

    Json ToJson();
    static WorkDay LoadJson(Json& json);
    static WorkDay GetRandom();
};

struct Classroom
{
    std::string name = "";

    Json ToJson();
    static Classroom LoadJson(Json& json);
    static Classroom GetRandom();
};

struct Lesson
{
    std::string name = "";
    std::vector<int> classIds;
    std::vector<int> classroomIds;

    Json ToJson();
    static Lesson LoadJson(Json& json);
    static Lesson GetRandom();
};

struct Teacher
{
    std::string name = "";
    std::vector<int> lessonIds;
    std::vector<WorkDay> workDays;

    Json ToJson();
    static Teacher LoadJson(Json& json);
    static Teacher GetRandom();
};

struct LessonTeacherPair
{
    int lessonId = -1;
    int teacherId = -1;

    Json ToJson();
    static LessonTeacherPair LoadJson(Json& json);
    static LessonTeacherPair GetRandom();
};

struct TimetableLesson
{
    int amount = 1;
    std::vector<LessonTeacherPair> lessonTeacherPairs;

    Json ToJson();
    static TimetableLesson LoadJson(Json& json);
    static TimetableLesson GetRandom();
};

class Timetable;

struct ClassroomLessonPair
{
    int timetableLessonId = -1;
    std::vector<int> classroomIds;

    Json ToJson();
    static ClassroomLessonPair LoadJson(Json& json);
    static ClassroomLessonPair GetRandom(Timetable& timetable, int classId);
};

struct Day
{
    std::vector<bool> lessons;
    std::vector<ClassroomLessonPair> classroomLessonPairs;

    Json ToJson();
    static Day LoadJson(Json& json);
    static Day GetRandom(Timetable& timetable, int classId);
};

struct TimetableLessonRule
{
    bool preserveOrder = false;
    int amount = 1;
    std::vector<int> timetableLessonIds;

    Json ToJson();
    static TimetableLessonRule LoadJson(Json& json);
    static TimetableLessonRule GetRandom();
};

struct Class
{
    std::string number = "";
    std::string letter = "";
    int teacherId = -1;
    int maxTimetableLessonId = -1;
    std::map<int, TimetableLesson> timetableLessons;
    std::vector<Day> days;
    std::vector<TimetableLessonRule> timetableLessonRules;

    Json ToJson();
    static Class LoadJson(Json& json);
    static Class GetRandom(Timetable& timetable, int classId);
};

class Timetable
{
  public:
    std::string name = "";
    int version = 1;
    int year = -1;
    int errors = -1;
    int bonusPoints = -1;
    int maxClassroomId = -1;
    int maxLessonId = -1;
    int maxTeacherId = -1;
    int maxClassId = -1;
    std::map<int, Classroom> classrooms;
    std::map<int, Lesson> lessons;
    std::map<int, Teacher> teachers;
    std::map<int, Class> classes;
    std::vector<int> orderedClasses;

    void Save(const std::filesystem::path& path);
    void Load(const std::filesystem::path& path);
    static Timetable GetRandom();
    void ExportAsXlsx();

  private:
    void ExportClassesAsXlsx();
    void ExportTeachersAsXlsx();
    void ExportClassroomsAsXlsx();
};

extern Timetable currentTimetable;
extern Timetable tmpTimetable;
extern Timetable tmpTmpTimetable;
