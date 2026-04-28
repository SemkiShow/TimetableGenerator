// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Timetable.hpp"
#include "Utils.hpp"
#include <JsonFormat.hpp>
#include <algorithm>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <string>
#include <utility>

void MigrateV0(Json& json)
{
    Json newJson = json;
    for (auto& classPair: json["classes"].GetObject())
    {
        for (auto& lesson: classPair.second["lessons"].GetObject())
        {
            auto& newLesson = newJson["classes"][classPair.first]["lessons"][lesson.first];
            newLesson["amount"] = lesson.second[0];
            newLesson["lessonTeacherPairs"] = Json::array_t();
            newLesson["lessonTeacherPairs"].format = JsonFormat::Inline;
            for (size_t i = 1; i < lesson.second.size(); i++)
            {
                Json pair;
                pair.format = JsonFormat::Inline;
                pair["lessonID"] = lesson.second[i]["lessonID"];
                pair["teacherID"] = lesson.second[i]["teacherID"];
                newLesson["lessonTeacherPairs"].emplace_back(std::move(pair));
            }
        }
    }
    json = newJson;
}

void MigrateV1(Json& json)
{
    // Lessons
    for (auto& lesson: json["lessons"].GetObject())
    {
        lesson.second["classIds"] = lesson.second["classIDs"];
        lesson.second["classroomIds"] = lesson.second["classroomIDs"];
    }

    // Teachers
    for (auto& teacher: json["teachers"].GetObject())
    {
        teacher.second["lessonIds"] = teacher.second["lessonIDs"];
    }

    // Classes
    for (auto& classPair: json["classes"].GetObject())
    {
        classPair.second["teacherId"] = classPair.second["teacherID"];

        // Lessons
        for (auto& lesson: classPair.second["lessons"].GetObject())
        {
            for (auto& lessonTeacherPair: lesson.second["lessonTeacherPairs"].GetArray())
            {
                lessonTeacherPair["lessonId"] = lessonTeacherPair["lessonID"];
                lessonTeacherPair["teacherId"] = lessonTeacherPair["teacherID"];
            }
        }

        // Days
        for (auto& day: classPair.second["days"].GetArray())
        {
            for (auto& classroomLessonPair: day.GetArray())
            {
                classroomLessonPair["timetableLessonId"] = classroomLessonPair["timetableLessonID"];
                classroomLessonPair["classroomIds"] = classroomLessonPair["classroomIDs"];
            }
        }

        // Timetable lesson rules
        for (auto& rule: classPair.second["timetableLessonRules"].GetArray())
        {
            rule["timetableLessonIds"] = rule["timetableLessonIDs"];
        }
    }
}

WorkDay WorkDay::LoadJson(Json& json)
{
    WorkDay workDay;

    for (auto& lessonId: json.GetArray())
    {
        workDay.lessonIds.push_back(lessonId);
    }

    return workDay;
}

Classroom Classroom::LoadJson(Json& json)
{
    Classroom classroom;

    classroom.name = json;

    return classroom;
}

Lesson Lesson::LoadJson(Json& json)
{
    Lesson lesson;

    lesson.name = json["name"];
    for (auto& classId: json["classIds"].GetArray())
    {
        lesson.classIds.push_back(classId);
    }
    for (auto& classroomId: json["classroomIds"].GetArray())
    {
        lesson.classroomIds.push_back(classroomId);
    }

    return lesson;
}

Teacher Teacher::LoadJson(Json& json)
{
    Teacher teacher;

    teacher.name = json["name"];
    for (auto& lessonId: json["lessonIds"].GetArray())
    {
        teacher.lessonIds.push_back(lessonId);
    }
    for (auto& workDay: json["workDays"].GetArray())
    {
        teacher.workDays.emplace_back(WorkDay::LoadJson(workDay));
    }

    return teacher;
}

LessonTeacherPair LessonTeacherPair::LoadJson(Json& json)
{
    LessonTeacherPair lessonTeacherPair;

    lessonTeacherPair.lessonId = json["lessonId"];
    lessonTeacherPair.teacherId = json["teacherId"];

    return lessonTeacherPair;
}

TimetableLesson TimetableLesson::LoadJson(Json& json)
{
    TimetableLesson timetableLesson;

    timetableLesson.amount = json["amount"];
    for (auto& lessonTeacherPair: json["lessonTeacherPairs"].GetArray())
    {
        timetableLesson.lessonTeacherPairs.push_back(
            LessonTeacherPair::LoadJson(lessonTeacherPair));
    }

    return timetableLesson;
}

ClassroomLessonPair ClassroomLessonPair::LoadJson(Json& json)
{
    ClassroomLessonPair classroomLessonPair;

    classroomLessonPair.timetableLessonId = json["timetableLessonId"];
    for (auto& classroomId: json["classroomIds"].GetArray())
    {
        classroomLessonPair.classroomIds.push_back(classroomId);
    }

    return classroomLessonPair;
}

Day Day::LoadJson(Json& json)
{
    Day day;

    for (auto& classroomLessonPair: json.GetArray())
    {
        day.classroomLessonPairs.push_back(ClassroomLessonPair::LoadJson(classroomLessonPair));
    }

    return day;
}

TimetableLessonRule TimetableLessonRule::LoadJson(Json& json)
{
    TimetableLessonRule timetableLessonRule;

    timetableLessonRule.preserveOrder = json["preserveOrder"];
    timetableLessonRule.amount = json["amount"];
    for (auto& timetableLessonId: json["timetableLessonIds"].GetArray())
    {
        timetableLessonRule.timetableLessonIds.push_back(timetableLessonId);
    }

    return timetableLessonRule;
}

Class Class::LoadJson(Json& json)
{
    Class classPair;

    classPair.number = json["number"];
    classPair.letter = json["letter"];
    classPair.teacherId = json["teacherId"];

    // Lessons
    for (auto& lesson: json["lessons"].GetObject())
    {
        int lessonId = stoi(lesson.first);
        classPair.maxTimetableLessonId = std::max(classPair.maxTimetableLessonId, lessonId);
        classPair.timetableLessons[lessonId] = TimetableLesson::LoadJson(lesson.second);
    }

    // Days
    for (auto& day: json["days"].GetArray())
    {
        classPair.days.emplace_back(Day::LoadJson(day));
    }
    for (size_t i = 0; i < json["lessonNumbers"].size(); i++)
    {
        while (i >= classPair.days.size()) classPair.days.emplace_back();
        for (size_t j = 0; j < json["lessonNumbers"][i].size(); j++)
        {
            classPair.days[i].lessons.push_back(json["lessonNumbers"][i][j]);
        }
    }

    // Timetable lesson rules
    for (auto& timetableLessonRule: json["timetableLessonRules"].GetArray())
    {
        classPair.timetableLessonRules.push_back(
            TimetableLessonRule::LoadJson(timetableLessonRule));
    }

    return classPair;
}

void Timetable::Load(const std::filesystem::path& path)
{
    LogInfo("Loading timetable at " + path.string());
    Json json = Json::Load(path);

    *this = Timetable();

    if (json["version"].IsNull())
        version = 0;
    else
        version = json["version"];

    if (version == 0)
    {
        MigrateV0(json);
        version = 1;
    }
    if (version == 1)
    {
        MigrateV1(json);
        version = 2;
    }

    name = std::filesystem::path(path).stem().string();
    name = TrimJunk(name);

    year = json["year"];
    if (year < 1900)
    {
        time_t now = time(0);
        tm* localTime = localtime(&now);
        year = 1900 + localTime->tm_year;
    }

    // Classrooms
    for (auto& classroom: json["classrooms"].GetObject())
    {
        if (classroom.second == "") continue;
        int classroomId = stoi(classroom.first);
        maxClassroomId = std::max(maxClassroomId, classroomId);
        classrooms[classroomId] = Classroom::LoadJson(classroom.second);
    }

    // Lessons
    for (auto& lesson: json["lessons"].GetObject())
    {
        if (lesson.second["name"] == "") continue;
        int lessonId = stoi(lesson.first);
        maxLessonId = std::max(maxLessonId, lessonId);
        lessons[lessonId] = Lesson::LoadJson(lesson.second);
    }

    // Teachers
    for (auto& teacher: json["teachers"].GetObject())
    {
        if (teacher.second["name"] == "") continue;
        int teacherId = stoi(teacher.first);
        maxTeacherId = std::max(maxTeacherId, teacherId);
        teachers[teacherId] = Teacher::LoadJson(teacher.second);
    }

    // Classes
    for (auto& classPair: json["classes"].GetObject())
    {
        if (classPair.second["number"] == "") continue;
        int classId = stoi(classPair.first);
        maxClassId = std::max(maxClassId, classId);
        classes[classId] = Class::LoadJson(classPair.second);
    }

    // Ordered classes
    auto tmpClasses = classes;
    while (tmpClasses.size() > 0)
    {
        std::string number = tmpClasses.begin()->second.number;
        for (auto it = tmpClasses.begin(); it != tmpClasses.end();)
        {
            if (it->second.number == number)
            {
                orderedClasses.push_back(it->first);
                it = tmpClasses.erase(it);
                LogInfo("Adding items to the orderedClasses list... " +
                        std::to_string(tmpClasses.size()) + " items left");
                continue;
            }
            ++it;
        }
    }
}
