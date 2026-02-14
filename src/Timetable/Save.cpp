// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Json.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"

Json WorkDay::ToJson()
{
    Json json;

    json.format = JsonFormat::Inline;
    for (auto& lessonId: lessonIds) json.push_back(lessonId);

    return json;
}

Json Classroom::ToJson()
{
    Json json;

    json = name;

    return json;
}

Json Lesson::ToJson()
{
    Json json;

    // Lesson name
    json["name"] = name;

    // Classes
    json["classIds"] = Json::array_t();
    json["classIds"].format = JsonFormat::Inline;
    for (auto& classId: classIds) json["classIds"].push_back(classId);

    // Classrooms
    json["classroomIds"] = Json::array_t();
    json["classroomIds"].format = JsonFormat::Inline;
    for (auto& classroomId: classroomIds) json["classroomIds"].push_back(classroomId);

    return json;
}

Json Teacher::ToJson()
{
    Json json;

    // Teacher name
    json["name"] = name;

    // Lessons
    json["lessonIds"] = Json::array_t();
    json["lessonIds"].format = JsonFormat::Inline;
    for (size_t i = 0; i < lessonIds.size(); i++) json["lessonIds"].push_back(lessonIds[i]);

    // Work days
    json["workDays"] = Json::array_t();
    for (auto& workDay: workDays) json["workDays"].push_back(workDay.ToJson());

    return json;
}

Json LessonTeacherPair::ToJson()
{
    Json json(JsonFormat::Inline);

    json["lessonId"] = lessonId;
    json["teacherId"] = teacherId;

    return json;
}

Json TimetableLesson::ToJson()
{
    Json json;

    json = Json(JsonFormat::Inline);
    json["amount"] = amount;
    json["lessonTeacherPairs"] = Json::array_t();
    json["lessonTeacherPairs"].format = JsonFormat::Inline;
    for (auto& lessonTeacherPair: lessonTeacherPairs)
    {
        json["lessonTeacherPairs"].emplace_back(lessonTeacherPair.ToJson());
    }

    return json;
}

Json ClassroomLessonPair::ToJson()
{
    Json json;

    json["timetableLessonId"] = timetableLessonId;
    json["classroomIds"].format = JsonFormat::Inline;
    for (auto& classroomId: classroomIds) json["classroomIds"].push_back(classroomId);

    return json;
}

Json Day::ToJson()
{
    Json json = Json::array_t();

    for (auto& classroomLessonPair: classroomLessonPairs)
    {
        json.emplace_back(classroomLessonPair.ToJson());
    }

    return json;
}

Json TimetableLessonRule::ToJson()
{
    Json json;

    json["preserveOrder"] = preserveOrder;
    json["amount"] = amount;
    json["timetableLessonIds"] = Json::array_t();
    json["timetableLessonIds"].format = JsonFormat::Inline;
    for (size_t i = 0; i < timetableLessonIds.size(); i++)
    {
        json["timetableLessonIds"].push_back(timetableLessonIds[i]);
    }

    return json;
}

Json Class::ToJson()
{
    Json json;

    // Class number and letter
    json["number"] = number;
    json["letter"] = letter;

    // Teacher Id
    json["teacherId"] = teacherId;

    // Lessons
    for (auto& timetableLesson: timetableLessons)
    {
        json["lessons"][std::to_string(timetableLesson.first)] = timetableLesson.second.ToJson();
    }

    // Lesson numbers
    json["lessonNumbers"] = Json::array_t();

    // Days
    json["days"] = Json::array_t();
    for (auto& day: days)
    {
        // Lesson numbers
        json["lessonNumbers"].emplace_back(Json::array_t());
        json["lessonNumbers"].back().format = JsonFormat::Inline;
        for (size_t i = 0; i < day.lessons.size(); i++)
        {
            json["lessonNumbers"].back().push_back((bool)day.lessons[i]);
        }

        json["days"].emplace_back(day.ToJson());
    }

    // Timetable lesson rules
    for (size_t i = 0; i < timetableLessonRules.size(); i++)
    {
        json["timetableLessonRules"].push_back(timetableLessonRules[i].ToJson());
    }

    return json;
}

void Timetable::Save(const std::filesystem::path& path)
{
    LogInfo("Saving timetable at " + path.string());
    if (name == "") return;

    Json json;

    // Version
    json["version"] = version;

    // Timetable year
    json["year"] = year;

    // Classrooms
    json["classrooms"].format = JsonFormat::Inline;
    for (auto& classroom: classrooms)
    {
        json["classrooms"][std::to_string(classroom.first)] = classroom.second.ToJson();
    }

    // Lessons
    for (auto& lesson: lessons)
    {
        json["lessons"][std::to_string(lesson.first)] = lesson.second.ToJson();
    }

    // Teachers
    for (auto& teacher: teachers)
    {
        json["teachers"][std::to_string(teacher.first)] = teacher.second.ToJson();
    }

    // Classes
    for (auto& classPair: classes)
    {
        json["classes"][std::to_string(classPair.first)] = classPair.second.ToJson();
    }

    json.Save(path);
}
