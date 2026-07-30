// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Timetable.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Time.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <utility>
#include <vector>
using json = nlohmann::json;

template <typename T>
// NOLINTNEXTLINE (readability-identifier-naming)
static void to_json(json& outJson, const std::map<int, T>& map)
{
    for (const auto& [key, value]: map)
    {
        outJson[std::to_string(key)] = value;
    }
}

void Timetable::Save(const std::filesystem::path& path)
{
    LogInfo("Saving timetable at %s", path.string().c_str());
    if (name == "") return;

    std::ofstream file(path);
    file << json(*this).dump(4);
}

void from_json(const json& clazzJson, Class& clazz)
{
    clazz.number = clazzJson["number"];
    clazz.letter = clazzJson["letter"];
    clazz.teacherId = clazzJson["teacherId"];

    // Lessons
    for (const auto& [lessonId, lesson]: clazzJson["lessons"].items())
    {
        int lessonIdInt = stoi(lessonId);
        clazz.maxTimetableLessonId = std::max(clazz.maxTimetableLessonId, lessonIdInt);
        clazz.timetableLessons[lessonIdInt] = lesson;
    }

    // Days
    for (const auto& day: clazzJson["days"])
    {
        clazz.days.emplace_back(day);
    }
    for (size_t i = 0; i < clazzJson["lessonNumbers"].size(); i++)
    {
        while (i >= clazz.days.size()) clazz.days.emplace_back();
        for (size_t j = 0; j < clazzJson["lessonNumbers"][i].size(); j++)
        {
            clazz.days[i].lessons.emplace_back(clazzJson["lessonNumbers"][i][j]);
        }
    }

    // Timetable lesson rules
    for (const auto& timetableLessonRule: clazzJson["timetableLessonRules"])
    {
        clazz.timetableLessonRules.emplace_back(timetableLessonRule);
    }
}

static void MigrateV0(json& oldJson)
{
    json newJson = oldJson;
    for (const auto& [clazzId, clazz]: oldJson["classes"].items())
    {
        for (const auto& [lessonId, lesson]: clazz["lessons"].items())
        {
            auto& newLesson = newJson["classes"][clazzId]["lessons"][lessonId];
            newLesson["amount"] = lesson[0];
            newLesson["lessonTeacherPairs"] = json::array_t();
            for (size_t i = 1; i < lesson.size(); i++)
            {
                json pair;
                pair["lessonID"] = lesson[i]["lessonID"];
                pair["teacherID"] = lesson[i]["teacherID"];
                newLesson["lessonTeacherPairs"].emplace_back(std::move(pair));
            }
        }
    }
    oldJson = newJson;
}

static void MigrateV1(json& oldJson)
{
    // Lessons
    for (auto& lesson: oldJson["lessons"])
    {
        lesson["classIds"] = lesson["classIDs"];
        lesson["classroomIds"] = lesson["classroomIDs"];
    }

    // Teachers
    for (auto& teacher: oldJson["teachers"])
    {
        teacher["lessonIds"] = teacher["lessonIDs"];
    }

    // Classes
    for (auto& classPair: oldJson["classes"])
    {
        classPair["teacherId"] = classPair["teacherID"];

        // Lessons
        for (auto& lesson: classPair["lessons"])
        {
            for (auto& lessonTeacherPair: lesson["lessonTeacherPairs"])
            {
                lessonTeacherPair["lessonId"] = lessonTeacherPair["lessonID"];
                lessonTeacherPair["teacherId"] = lessonTeacherPair["teacherID"];
            }
        }

        // Days
        for (auto& day: classPair["days"])
        {
            for (auto& classroomLessonPair: day)
            {
                classroomLessonPair["timetableLessonId"] = classroomLessonPair["timetableLessonID"];
                classroomLessonPair["classroomIds"] = classroomLessonPair["classroomIDs"];
            }
        }

        // Timetable lesson rules
        for (auto& rule: classPair["timetableLessonRules"])
        {
            rule["timetableLessonIds"] = rule["timetableLessonIDs"];
        }
    }
}

static void MigrateV2(json& oldJson)
{
    for (auto& classPair: oldJson["classes"])
    {
        for (auto& lesson: classPair["lessons"])
        {
            lesson["count"] = lesson["amount"];
        }
        for (auto& rule: classPair["timetableLessonRules"])
        {
            rule["count"] = rule["amount"];
        }
    }
}

static void MigrateV3(json& oldJson)
{
    for (auto& classroom: oldJson["classrooms"])
    {
        classroom = {{"name", classroom}};
    }
    for (auto& teacher: oldJson["teachers"])
    {
        for (auto& workDay: teacher["workDays"])
        {
            workDay = {{"lessonIds", workDay}};
        }
    }
    for (auto& clazz: oldJson["classes"])
    {
        clazz["timetableLessons"] = clazz["lessons"];

        size_t dayIdx = 0;
        for (auto& day: clazz["days"])
        {
            day = {{"classroomLessonPairs", day}};
            auto lessons = clazz["lessonNumbers"];
            if (dayIdx < lessons.size())
                day["lessons"] = lessons[dayIdx];
            else
                day["lesssons"] = std::vector<bool>(g_settings.lessonsPerDay, true);
            dayIdx++;
        }
    }
}

void from_json(const json& timetableJson, Timetable& timetable)
{
    timetable.year = timetableJson["year"];
    if (timetable.year < Time::FIRST_YEAR)
    {
        time_t now = time(0);
        tm* localTime = localtime(&now);
        timetable.year = Time::FIRST_YEAR + localTime->tm_year;
    }

    // Classrooms
    for (const auto& [classroomId, classroom]: timetableJson["classrooms"].items())
    {
        if (classroom == "") continue;
        int classroomIdInt = stoi(classroomId);
        timetable.maxClassroomId = std::max(timetable.maxClassroomId, classroomIdInt);
        timetable.classrooms[classroomIdInt] = classroom;
    }

    // Lessons
    for (const auto& [lessonId, lesson]: timetableJson["lessons"].items())
    {
        if (lesson["name"] == "") continue;
        int lessonIdInt = stoi(lessonId);
        timetable.maxLessonId = std::max(timetable.maxLessonId, lessonIdInt);
        timetable.lessons[lessonIdInt] = lesson;
    }

    // Teachers
    for (const auto& [teacherId, teacher]: timetableJson["teachers"].items())
    {
        if (teacher["name"] == "") continue;
        int teacherIdInt = stoi(teacherId);
        timetable.maxTeacherId = std::max(timetable.maxTeacherId, teacherIdInt);
        timetable.teachers[teacherIdInt] = teacher;
    }

    // Classes
    for (const auto& [classId, clazz]: timetableJson["classes"].items())
    {
        if (clazz["number"] == "") continue;
        int classIdInt = stoi(classId);
        timetable.maxClassId = std::max(timetable.maxClassId, classIdInt);
        timetable.classes[classIdInt] = clazz;
    }

    // Ordered classes
    auto tmpClasses = timetable.classes;
    while (tmpClasses.size() > 0)
    {
        std::string number = tmpClasses.begin()->second.number;
        for (auto it = tmpClasses.begin(); it != tmpClasses.end();)
        {
            if (it->second.number == number)
            {
                timetable.orderedClasses.push_back(it->first);
                it = tmpClasses.erase(it);
                LogInfo("Adding items to the orderedClasses list... %zu items left",
                        tmpClasses.size());
                continue;
            }
            ++it;
        }
    }
}

void Timetable::Load(const std::filesystem::path& path)
{
    LogInfo("Loading timetable at %s", path.string().c_str());

    // Back up the current file just in case
    if (std::filesystem::exists(path))
    {
        auto parentDir = path.parent_path().string();
        std::filesystem::create_directory(parentDir + "/backups");
        auto filename = path.filename();
        auto backupPath = parentDir + "/backups/" + filename.stem().string() + "-" +
                          Time::Now().ToString(Time::Format::Path) + filename.extension().string();
        std::filesystem::copy_file(path, backupPath);
    }

    std::string data;
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) data += line;
    json timetableJson = json::parse(data);
    if (!timetableJson.contains("version")) timetableJson["version"] = 0;

    *this = {};

    version = timetableJson["version"];

    if (version == 0)
    {
        MigrateV0(timetableJson);
        version = 1;
    }
    if (version == 1)
    {
        MigrateV1(timetableJson);
        version = 2;
    }
    if (version == 2)
    {
        MigrateV2(timetableJson);
        version = 3;
    }
    if (version == 3)
    {
        MigrateV3(timetableJson);
        version = 4;
    }

    name = std::filesystem::path(path).stem().string();
    name = TrimJunk(name);

    from_json(timetableJson, *this);
}
