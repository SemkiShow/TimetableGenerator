// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "JSON.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include <filesystem>
#include <iostream>

Timetable currentTimetable;
Timetable tmpTimetable;
Timetable tmpTmpTimetable;

void Timetable::Save(const std::string& path)
{
    LogInfo("Saving timetable at " + path);
    if (name == "") return;

    JSON jsonObject;

    // Version
    jsonObject["version"] = version;

    // Timetable year
    jsonObject["year"] = year;

    // Classrooms
    jsonObject["classrooms"] = JSON(JSONFormat::Inline);
    for (auto& classroom: classrooms)
    {
        jsonObject["classrooms"][std::to_string(classroom.first)] = classroom.second.name;
    }

    // Lessons
    jsonObject["lessons"] = JSON();
    for (auto& lesson: lessons)
    {
        jsonObject["lessons"][std::to_string(lesson.first)] = JSON();

        // Lesson name
        jsonObject["lessons"][std::to_string(lesson.first)]["name"] = lesson.second.name;

        // Class names
        jsonObject["lessons"][std::to_string(lesson.first)]["classIDs"] = JSON::array_t();
        jsonObject["lessons"][std::to_string(lesson.first)]["classIDs"].format = JSONFormat::Inline;
        for (size_t i = 0; i < lesson.second.classIDs.size(); i++)
        {
            jsonObject["lessons"][std::to_string(lesson.first)]["classIDs"].push_back(
                lesson.second.classIDs[i]);
        }

        // Classrooms
        jsonObject["lessons"][std::to_string(lesson.first)]["classroomIDs"] = JSON::array_t();
        jsonObject["lessons"][std::to_string(lesson.first)]["classroomIDs"].format =
            JSONFormat::Inline;
        for (size_t i = 0; i < lesson.second.classroomIDs.size(); i++)
        {
            jsonObject["lessons"][std::to_string(lesson.first)]["classroomIDs"].push_back(
                lesson.second.classroomIDs[i]);
        }
    }

    // Teachers
    jsonObject["teachers"] = JSON();
    for (auto& teacher: teachers)
    {
        // Teacher
        jsonObject["teachers"][std::to_string(teacher.first)] = JSON();

        // Teacher name
        jsonObject["teachers"][std::to_string(teacher.first)]["name"] = teacher.second.name;

        // Lessons
        jsonObject["teachers"][std::to_string(teacher.first)]["lessonIDs"] = JSON::array_t();
        jsonObject["teachers"][std::to_string(teacher.first)]["lessonIDs"].format =
            JSONFormat::Inline;
        for (size_t i = 0; i < teacher.second.lessonIDs.size(); i++)
            jsonObject["teachers"][std::to_string(teacher.first)]["lessonIDs"].push_back(
                teacher.second.lessonIDs[i]);

        // Work days
        jsonObject["teachers"][std::to_string(teacher.first)]["workDays"] = JSON::array_t();
        teacher.second.workDays.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            jsonObject["teachers"][std::to_string(teacher.first)]["workDays"].push_back(
                JSON::array_t());
            jsonObject["teachers"][std::to_string(teacher.first)]["workDays"][i].format =
                JSONFormat::Inline;
            // Lesson number
            for (size_t j = 0; j < teacher.second.workDays[i].lessonIDs.size(); j++)
            {
                jsonObject["teachers"][std::to_string(teacher.first)]["workDays"][i].push_back(
                    teacher.second.workDays[i].lessonIDs[j]);
            }
        }
    }

    // Classes
    jsonObject["classes"] = JSON();
    for (auto& classPair: classes)
    {
        // Class
        jsonObject["classes"][std::to_string(classPair.first)] = JSON();

        // Class number and letter
        jsonObject["classes"][std::to_string(classPair.first)]["number"] = classPair.second.number;
        jsonObject["classes"][std::to_string(classPair.first)]["letter"] = classPair.second.letter;

        // Teacher ID
        jsonObject["classes"][std::to_string(classPair.first)]["teacherID"] =
            classPair.second.teacherID;

        // Lessons
        jsonObject["classes"][std::to_string(classPair.first)]["lessons"] = JSON();
        for (auto& timetableLesson: classPair.second.timetableLessons)
        {
            jsonObject["classes"][std::to_string(classPair.first)]["lessons"]
                      [std::to_string(timetableLesson.first)] = JSON::array_t();
            jsonObject["classes"][std::to_string(classPair.first)]["lessons"]
                      [std::to_string(timetableLesson.first)]
                          .format = JSONFormat::Inline;
            jsonObject["classes"][std::to_string(classPair.first)]["lessons"]
                      [std::to_string(timetableLesson.first)] = JSON(JSONFormat::Inline);
            jsonObject["classes"][std::to_string(classPair.first)]["lessons"]
                      [std::to_string(timetableLesson.first)]["amount"] =
                          timetableLesson.second.amount;
            jsonObject["classes"][std::to_string(classPair.first)]["lessons"]
                      [std::to_string(timetableLesson.first)]["lessonTeacherPairs"] =
                          JSON::array_t();
            jsonObject["classes"][std::to_string(classPair.first)]["lessons"]
                      [std::to_string(timetableLesson.first)]["lessonTeacherPairs"]
                          .format = JSONFormat::Inline;
            for (size_t j = 0; j < timetableLesson.second.lessonTeacherPairs.size(); j++)
            {
                jsonObject["classes"][std::to_string(classPair.first)]["lessons"]
                          [std::to_string(timetableLesson.first)]["lessonTeacherPairs"]
                              .emplace_back(JSONFormat::Inline);
                jsonObject["classes"][std::to_string(classPair.first)]["lessons"][std::to_string(
                    timetableLesson.first)]["lessonTeacherPairs"][j]["lessonID"] =
                    timetableLesson.second.lessonTeacherPairs[j].lessonID;
                jsonObject["classes"][std::to_string(classPair.first)]["lessons"][std::to_string(
                    timetableLesson.first)]["lessonTeacherPairs"][j]["teacherID"] =
                    timetableLesson.second.lessonTeacherPairs[j].teacherID;
            }
        }

        // Lesson numbers
        jsonObject["classes"][std::to_string(classPair.first)]["lessonNumbers"] = JSON::array_t();

        // Days
        jsonObject["classes"][std::to_string(classPair.first)]["days"] = JSON::array_t();

        classPair.second.days.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            classPair.second.days[i].lessons.resize(lessonsPerDay);
            // Lesson numbers
            jsonObject["classes"][std::to_string(classPair.first)]["lessonNumbers"].push_back(
                JSON::array_t());
            jsonObject["classes"][std::to_string(classPair.first)]["lessonNumbers"][i].format =
                JSONFormat::Inline;
            for (size_t j = 0; j < classPair.second.days[i].lessons.size(); j++)
            {
                jsonObject["classes"][std::to_string(classPair.first)]["lessonNumbers"][i]
                    .push_back((bool)classPair.second.days[i].lessons[j]);
            }

            // Days
            jsonObject["classes"][std::to_string(classPair.first)]["days"].push_back(
                JSON::array_t());
            for (size_t j = 0; j < classPair.second.days[i].classroomLessonPairs.size(); j++)
            {
                jsonObject["classes"][std::to_string(classPair.first)]["days"][i].push_back(
                    JSON(JSONFormat::Inline));
                jsonObject["classes"][std::to_string(classPair.first)]["days"][i][j]
                          ["timetableLessonID"] =
                              classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                jsonObject["classes"][std::to_string(classPair.first)]["days"][i][j]
                          ["classroomIDs"] = JSON::array_t();
                jsonObject["classes"][std::to_string(classPair.first)]["days"][i][j]["classroomIDs"]
                    .format = JSONFormat::Inline;
                for (size_t k = 0;
                     k < classPair.second.days[i].classroomLessonPairs[j].classroomIDs.size(); k++)
                {
                    jsonObject["classes"][std::to_string(
                        classPair.first)]["days"][i][j]["classroomIDs"]
                        .push_back(
                            classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k]);
                }
            }
        }

        // Timetable lesson rules
        jsonObject["classes"][std::to_string(classPair.first)]["timetableLessonRules"] =
            JSON::array_t();
        for (size_t i = 0; i < classPair.second.timetableLessonRules.size(); i++)
        {
            jsonObject["classes"][std::to_string(classPair.first)]["timetableLessonRules"]
                .push_back(JSON());
            jsonObject["classes"][std::to_string(classPair.first)]["timetableLessonRules"][i]
                      ["preserveOrder"] = classPair.second.timetableLessonRules[i].preserveOrder;
            jsonObject["classes"][std::to_string(classPair.first)]["timetableLessonRules"][i]
                      ["amount"] = classPair.second.timetableLessonRules[i].amount;
            jsonObject["classes"][std::to_string(classPair.first)]["timetableLessonRules"][i]
                      ["timetableLessonIDs"] = JSON::array_t();
            jsonObject["classes"][std::to_string(classPair.first)]["timetableLessonRules"][i]
                      ["timetableLessonIDs"]
                          .format = JSONFormat::Inline;
            for (size_t j = 0;
                 j < classPair.second.timetableLessonRules[i].timetableLessonIDs.size(); j++)
            {
                jsonObject["classes"][std::to_string(
                    classPair.first)]["timetableLessonRules"][i]["timetableLessonIDs"]
                    .push_back(classPair.second.timetableLessonRules[i].timetableLessonIDs[j]);
            }
        }
    }

    jsonObject.Save(path);
}

void MigrateV0(JSON& jsonObject)
{
    JSON newJson = jsonObject;
    for (auto& classPair: jsonObject["classes"].GetObject())
    {
        for (auto& lesson: classPair.second["lessons"].GetObject())
        {
            auto& newLesson = newJson["classes"][classPair.first]["lessons"][lesson.first];
            newLesson["amount"] = lesson.second[(size_t)0];
            newLesson["lessonTeacherPairs"] = JSON::array_t();
            newLesson["lessonTeacherPairs"].format = JSONFormat::Inline;
            for (size_t i = 1; i < lesson.second.size(); i++)
            {
                newLesson["lessonTeacherPairs"].emplace_back(JSONFormat::Inline);
                newLesson["lessonTeacherPairs"][i - 1]["lessonID"] = lesson.second[i]["lessonID"];
                newLesson["lessonTeacherPairs"][i - 1]["teacherID"] = lesson.second[i]["teacherID"];
            }
        }
    }
    jsonObject = newJson;
}

void Timetable::Load(const std::string& path)
{
    LogInfo("Loading timetable at " + path);
    JSON jsonObject = JSON::Load(path);

    *this = Timetable();
    if (jsonObject["version"].IsNull())
        version = 0;
    else
        version = jsonObject["version"].GetInt();
    if (version == 0)
    {
        MigrateV0(jsonObject);
        version = 1;
    }

    name = std::filesystem::path(path).stem().string();
    name = TrimJunk(name);
    year = jsonObject["year"].GetInt();
    if (year < 1900)
    {
        time_t now = time(0);
        tm* localTime = localtime(&now);
        year = 1900 + localTime->tm_year;
    }

    // Classrooms
    for (auto& classroom: jsonObject["classrooms"].GetObject())
    {
        if (classroom.second.GetString() == "") continue;
        int classroomID = stoi(classroom.first);
        if (classroomID > maxClassroomID) maxClassroomID = classroomID;
        classrooms[classroomID] = Classroom();
        classrooms[classroomID].name = classroom.second.GetString();
    }

    // Lessons
    for (auto& lesson: jsonObject["lessons"].GetObject())
    {
        if (lesson.second["name"].GetString() == "") continue;
        int lessonID = stoi(lesson.first);
        if (lessonID > maxLessonID) maxLessonID = lessonID;
        lessons[lessonID] = Lesson();
        lessons[lessonID].name = lesson.second["name"].GetString();
        for (size_t i = 0; i < lesson.second["classIDs"].size(); i++)
            lessons[lessonID].classIDs.push_back(lesson.second["classIDs"][i].GetInt());
        for (size_t i = 0; i < lesson.second["classroomIDs"].size(); i++)
            lessons[lessonID].classroomIDs.push_back(lesson.second["classroomIDs"][i].GetInt());
    }

    // Teachers
    for (auto& teacher: jsonObject["teachers"].GetObject())
    {
        if (teacher.second["name"].GetString() == "") continue;
        int teacherID = stoi(teacher.first);
        if (teacherID > maxTeacherID) maxTeacherID = teacherID;
        teachers[teacherID] = Teacher();
        teachers[teacherID].name = teacher.second["name"].GetString();
        for (size_t i = 0; i < teacher.second["lessonIDs"].size(); i++)
            teachers[teacherID].lessonIDs.push_back(teacher.second["lessonIDs"][i].GetInt());
        teachers[teacherID].workDays.resize(teacher.second["workDays"].size());
        for (size_t i = 0; i < teacher.second["workDays"].size(); i++)
        {
            for (size_t j = 0; j < teacher.second["workDays"][i].size(); j++)
            {
                teachers[teacherID].workDays[i].lessonIDs.push_back(
                    teacher.second["workDays"][i][j].GetInt());
            }
        }
    }

    // Classes
    for (auto& classPair: jsonObject["classes"].GetObject())
    {
        if (classPair.second["number"].GetString() == "") continue;
        int classID = stoi(classPair.first);
        if (classID > maxClassID) maxClassID = classID;
        classes[classID] = Class();
        classes[classID].number = classPair.second["number"].GetString();
        classes[classID].letter = classPair.second["letter"].GetString();
        classes[classID].teacherID = classPair.second["teacherID"].GetInt();

        // Lessons
        for (auto& lesson: classPair.second["lessons"].GetObject())
        {
            int lessonID = stoi(lesson.first);
            if (lessonID > classes[classID].maxTimetableLessonID)
                classes[classID].maxTimetableLessonID = lessonID;
            classes[classID].timetableLessons[lessonID] = TimetableLesson();
            classes[classID].timetableLessons[lessonID].amount = lesson.second["amount"].GetInt();
            for (size_t i = 0; i < lesson.second["lessonTeacherPairs"].size(); i++)
            {
                classes[classID].timetableLessons[lessonID].lessonTeacherPairs.push_back(
                    LessonTeacherPair());
                classes[classID].timetableLessons[lessonID].lessonTeacherPairs[i].lessonID =
                    lesson.second["lessonTeacherPairs"][i]["lessonID"].GetInt();
                classes[classID].timetableLessons[lessonID].lessonTeacherPairs[i].teacherID =
                    lesson.second["lessonTeacherPairs"][i]["teacherID"].GetInt();
            }
        }
        classes[classID].days.resize(classPair.second["lessonNumbers"].size());
        for (size_t i = 0; i < classPair.second["lessonNumbers"].size(); i++)
        {
            for (size_t j = 0; j < classPair.second["lessonNumbers"][i].size(); j++)
                classes[classID].days[i].lessons.push_back(
                    classPair.second["lessonNumbers"][i][j].GetBool());
        }

        // Days
        for (size_t i = 0; i < classPair.second["days"].size(); i++)
        {
            for (size_t j = 0; j < classPair.second["days"][i].size(); j++)
            {
                classes[classID].days[i].classroomLessonPairs.push_back(ClassroomLessonPair());
                classes[classID].days[i].classroomLessonPairs[j].timetableLessonID =
                    classPair.second["days"][i][j]["timetableLessonID"].GetInt();
                for (size_t k = 0; k < classPair.second["days"][i][j]["classroomIDs"].size(); k++)
                {
                    classes[classID].days[i].classroomLessonPairs[j].classroomIDs.push_back(
                        classPair.second["days"][i][j]["classroomIDs"][k].GetInt());
                }
            }
        }

        // Timetable lesson rules
        for (size_t i = 0; i < classPair.second["timetableLessonRules"].size(); i++)
        {
            classes[classID].timetableLessonRules.push_back(TimetableLessonRule());
            classes[classID].timetableLessonRules[i].preserveOrder =
                classPair.second["timetableLessonRules"][i]["preserveOrder"].GetBool();
            classes[classID].timetableLessonRules[i].amount =
                classPair.second["timetableLessonRules"][i]["amount"].GetInt();
            for (size_t j = 0;
                 j < classPair.second["timetableLessonRules"][i]["timetableLessonIDs"].size(); j++)
            {
                classes[classID].timetableLessonRules[i].timetableLessonIDs.push_back(
                    classPair.second["timetableLessonRules"][i]["timetableLessonIDs"][j].GetInt());
            }
        }
    }

    std::map<int, Class> tmpClasses = classes;
    while (tmpClasses.size() > 0)
    {
        std::string number = tmpClasses.begin()->second.number;
        for (auto it = tmpClasses.begin(); it != tmpClasses.end();)
        {
            if (it->second.number == number)
            {
                orderedClasses.push_back(it->first);
                it = tmpClasses.erase(it);
                std::cout << "Adding items to the orderedClasses list... " << tmpClasses.size()
                          << " items left\n";
                continue;
            }
            ++it;
        }
    }

    if (this == &currentTimetable) tmpTmpTimetable = tmpTimetable = currentTimetable;
}

void Timetable::GenerateRandomTimetable()
{
    // Classrooms
    for (size_t i = 0; i < 10; i++)
    {
        classrooms[i] = Classroom();
        for (size_t j = 0; j < 3; j++)
        {
            classrooms[i].name += '0' + rand() % 10;
        }
    }

    // Lessons
    for (size_t i = 0; i < 15; i++)
    {
        lessons[i] = Lesson();
        for (size_t j = 0; j < 7; j++)
            lessons[i].name += 'a' + rand() % 26;
        for (size_t j = 0; j < 4; j++)
        {
            auto it = classrooms.begin();
            std::advance(it, rand() % classrooms.size());
            lessons[i].classroomIDs.push_back(it->first);
        }
    }

    // Teachers
    for (size_t i = 0; i < 5; i++)
    {
        teachers[i] = Teacher();
        for (size_t j = 0; j < 7; j++)
            teachers[i].name += 'a' + rand() % 26;
        for (size_t j = 0; j < 3; j++)
        {
            auto it = lessons.begin();
            std::advance(it, rand() % lessons.size());
            teachers[i].lessonIDs.push_back(it->first);
        }
        teachers[i].workDays.resize(daysPerWeek);
        for (size_t j = 0; j < daysPerWeek; j++)
        {
            teachers[i].workDays[j].lessonIDs.resize(lessonsPerDay, rand() % lessons.size());
        }
    }

    // Classes
    size_t classLettersPerClassNumber = 3;
    for (size_t i = 1; i < 3; i++)
    {
        for (size_t j = 0; j < classLettersPerClassNumber; j++)
        {
            int classID = (i - 1) * classLettersPerClassNumber + j;
            classes[classID] = Class();
            classes[classID].number = std::to_string(i);
            classes[classID].letter = 'a' + j;
            {
                auto it = teachers.begin();
                std::advance(it, rand() % teachers.size());
                classes[classID].teacherID = it->first;
            }

            // Lessons
            for (size_t k = 0; k < 5; k++)
            {
                classes[classID].timetableLessons[k] = TimetableLesson();
                classes[classID].timetableLessons[k].lessonTeacherPairs.push_back(
                    LessonTeacherPair());
                {
                    auto it = lessons.begin();
                    std::advance(it, rand() % lessons.size());
                    classes[classID].timetableLessons[k].lessonTeacherPairs[0].lessonID = it->first;
                }
                {
                    auto it = teachers.begin();
                    std::advance(it, rand() % teachers.size());
                    classes[classID].timetableLessons[k].lessonTeacherPairs[0].teacherID =
                        it->first;
                }
            }

            // Days
            classes[classID].days.resize(daysPerWeek);
            for (size_t k = 0; k < daysPerWeek; k++)
            {
                classes[classID].days[k].lessons.reserve(lessonsPerDay);
                for (size_t m = 0; m < lessonsPerDay; m++)
                {
                    classes[classID].days[k].lessons.push_back(rand() % 2 == 0);

                    auto it = classes[classID].timetableLessons.begin();
                    std::advance(it, rand() % classes[classID].timetableLessons.size());
                    classes[classID].days[k].classroomLessonPairs.push_back(ClassroomLessonPair());
                    classes[classID].days[k].classroomLessonPairs[m].timetableLessonID = it->first;
                    for (size_t n = 0; n < it->second.lessonTeacherPairs.size(); n++)
                    {
                        int lessonID = it->second.lessonTeacherPairs[n].lessonID;
                        int classroomID =
                            lessons[lessonID]
                                .classroomIDs[rand() % lessons[lessonID].classroomIDs.size()];
                        classes[classID].days[k].classroomLessonPairs[m].classroomIDs.push_back(
                            classroomID);
                    }
                }
            }
        }
    }

    // Assigning classes to lessons
    for (size_t i = 0; i < lessons.size(); i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            auto it = classes.begin();
            std::advance(it, rand() % (classes.size() - 1));
            lessons[i].classIDs.push_back(it->first);
        }
    }
}
