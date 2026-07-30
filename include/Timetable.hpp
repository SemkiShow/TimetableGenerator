// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <filesystem>
#include <map>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp> // IWYU pragma: keep
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <vector>

class Timetable;

#define ANY_LESSON (-2)
#define NO_LESSON  (-3)

struct WorkDay
{
    std::vector<int> lessonIds;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(WorkDay, lessonIds);
};

struct Classroom
{
    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Classroom, name);
};

struct Lesson
{
    std::string name;
    std::vector<int> classIds;
    std::vector<int> classroomIds;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Lesson, name, classIds, classroomIds);
};

struct Teacher
{
    std::string name;
    std::vector<int> lessonIds;
    std::vector<WorkDay> workDays;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Teacher, name, lessonIds, workDays);
};

struct LessonTeacherPair
{
    int lessonId = -1;
    int teacherId = -1;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LessonTeacherPair, lessonId, teacherId);
};

struct TimetableLesson
{
    int count = 1;
    std::vector<LessonTeacherPair> lessonTeacherPairs;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimetableLesson, count, lessonTeacherPairs);
};

struct ClassroomLessonPair
{
    int timetableLessonId = -1;
    std::vector<int> classroomIds;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ClassroomLessonPair, timetableLessonId, classroomIds);
};

struct Day
{
    std::vector<bool> lessons;
    std::vector<ClassroomLessonPair> classroomLessonPairs;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Day, lessons, classroomLessonPairs);
};

struct TimetableLessonRule
{
    bool preserveOrder = false;
    int count = 1;
    std::vector<int> timetableLessonIds;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimetableLessonRule, preserveOrder, count, timetableLessonIds);
};

struct Class
{
    std::string number;
    std::string letter;
    int teacherId = -1;
    int maxTimetableLessonId = -1;
    std::map<int, TimetableLesson> timetableLessons;
    std::vector<Day> days;
    std::vector<TimetableLessonRule> timetableLessonRules;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_ONLY_SERIALIZE(Class, number, letter, teacherId,
                                                  timetableLessons, days, timetableLessonRules);
};
// NOLINTNEXTLINE (readability-identifier-naming)
void from_json(const nlohmann::json& clazzJson, Class& clazz);

class Timetable
{
public:
    std::string name;
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
    void ExportAsXlsx() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_ONLY_SERIALIZE(Timetable, version, year, classrooms, lessons,
                                                  teachers, classes);
};
// NOLINTNEXTLINE (readability-identifier-naming)
void from_json(const nlohmann::json& json, Timetable& timetable);

inline Timetable g_currentTimetable;
