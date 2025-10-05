// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Timetable.hpp"
#include "JSON.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include <filesystem>
#include <iostream>

Timetable currentTimetable;
Timetable tmpTimetable;
Timetable tmpTmpTimetable;

void Timetable::Save(const std::string& path)
{
    LogInfo("Saving timetable at " + path);
    if (name == "") return;

    JSONObject jsonObject;
    jsonObject.type = JSON_OBJECT;

    // Version
    jsonObject.intPairs["version"] = version;

    // Timetable year
    jsonObject.intPairs["year"] = year;

    // Classrooms
    jsonObject.objectPairs["classrooms"] = JSONObject();
    jsonObject.objectPairs["classrooms"].type = JSON_OBJECT;
    jsonObject.objectPairs["classrooms"].format = JSON_INLINE;
    for (auto& classroom: classrooms)
        jsonObject.objectPairs["classrooms"].stringPairs[std::to_string(classroom.first)] =
            classroom.second.name;

    // Lessons
    jsonObject.objectPairs["lessons"] = JSONObject();
    jsonObject.objectPairs["lessons"].type = JSON_OBJECT;
    for (auto& lesson: lessons)
    {
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)] = JSONObject();
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].type =
            JSON_OBJECT;

        // Lesson name
        jsonObject.objectPairs["lessons"]
            .objectPairs[std::to_string(lesson.first)]
            .stringPairs["name"] = lesson.second.name;

        // Class names
        jsonObject.objectPairs["lessons"]
            .objectPairs[std::to_string(lesson.first)]
            .objectPairs["classIDs"] = JSONObject();
        jsonObject.objectPairs["lessons"]
            .objectPairs[std::to_string(lesson.first)]
            .objectPairs["classIDs"]
            .type = JSON_LIST;
        jsonObject.objectPairs["lessons"]
            .objectPairs[std::to_string(lesson.first)]
            .objectPairs["classIDs"]
            .format = JSON_INLINE;
        for (size_t i = 0; i < lesson.second.classIDs.size(); i++)
        {
            jsonObject.objectPairs["lessons"]
                .objectPairs[std::to_string(lesson.first)]
                .objectPairs["classIDs"]
                .ints.push_back(lesson.second.classIDs[i]);
        }

        // Classrooms
        jsonObject.objectPairs["lessons"]
            .objectPairs[std::to_string(lesson.first)]
            .objectPairs["classroomIDs"]
            .type = JSON_LIST;
        jsonObject.objectPairs["lessons"]
            .objectPairs[std::to_string(lesson.first)]
            .objectPairs["classroomIDs"]
            .format = JSON_INLINE;
        for (size_t i = 0; i < lesson.second.classroomIDs.size(); i++)
        {
            jsonObject.objectPairs["lessons"]
                .objectPairs[std::to_string(lesson.first)]
                .objectPairs["classroomIDs"]
                .ints.push_back(lesson.second.classroomIDs[i]);
        }
    }

    // Teachers
    jsonObject.objectPairs["teachers"] = JSONObject();
    jsonObject.objectPairs["teachers"].type = JSON_OBJECT;
    for (auto& teacher: teachers)
    {
        // Teacher
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)] =
            JSONObject();
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].type =
            JSON_OBJECT;

        // Teacher name
        jsonObject.objectPairs["teachers"]
            .objectPairs[std::to_string(teacher.first)]
            .stringPairs["name"] = teacher.second.name;

        // Lessons
        jsonObject.objectPairs["teachers"]
            .objectPairs[std::to_string(teacher.first)]
            .objectPairs["lessonIDs"] = JSONObject();
        jsonObject.objectPairs["teachers"]
            .objectPairs[std::to_string(teacher.first)]
            .objectPairs["lessonIDs"]
            .type = JSON_LIST;
        jsonObject.objectPairs["teachers"]
            .objectPairs[std::to_string(teacher.first)]
            .objectPairs["lessonIDs"]
            .format = JSON_INLINE;
        for (size_t i = 0; i < teacher.second.lessonIDs.size(); i++)
            jsonObject.objectPairs["teachers"]
                .objectPairs[std::to_string(teacher.first)]
                .objectPairs["lessonIDs"]
                .ints.push_back(teacher.second.lessonIDs[i]);

        // Work days
        jsonObject.objectPairs["teachers"]
            .objectPairs[std::to_string(teacher.first)]
            .objectPairs["workDays"] = JSONObject();
        jsonObject.objectPairs["teachers"]
            .objectPairs[std::to_string(teacher.first)]
            .objectPairs["workDays"]
            .type = JSON_LIST;
        teacher.second.workDays.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            jsonObject.objectPairs["teachers"]
                .objectPairs[std::to_string(teacher.first)]
                .objectPairs["workDays"]
                .objects.push_back(JSONObject());
            jsonObject.objectPairs["teachers"]
                .objectPairs[std::to_string(teacher.first)]
                .objectPairs["workDays"]
                .objects[i]
                .type = JSON_LIST;
            jsonObject.objectPairs["teachers"]
                .objectPairs[std::to_string(teacher.first)]
                .objectPairs["workDays"]
                .objects[i]
                .format = JSON_INLINE;
            // Lesson number
            for (size_t j = 0; j < teacher.second.workDays[i].lessonIDs.size(); j++)
            {
                jsonObject.objectPairs["teachers"]
                    .objectPairs[std::to_string(teacher.first)]
                    .objectPairs["workDays"]
                    .objects[i]
                    .ints.push_back(teacher.second.workDays[i].lessonIDs[j]);
            }
        }
    }

    // Classes
    jsonObject.objectPairs["classes"] = JSONObject();
    jsonObject.objectPairs["classes"].type = JSON_OBJECT;
    for (auto& classPair: classes)
    {
        // Class
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)] =
            JSONObject();
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].type =
            JSON_OBJECT;

        // Class number and letter
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .stringPairs["number"] = classPair.second.number;
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .stringPairs["letter"] = classPair.second.letter;

        // Teacher ID
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .intPairs["teacherID"] = classPair.second.teacherID;

        // Lessons
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .objectPairs["lessons"] = JSONObject();
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .objectPairs["lessons"]
            .type = JSON_OBJECT;
        for (auto& timetableLesson: classPair.second.timetableLessons)
        {
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["lessons"]
                .objectPairs[std::to_string(timetableLesson.first)] = JSONObject();
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["lessons"]
                .objectPairs[std::to_string(timetableLesson.first)]
                .type = JSON_LIST;
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["lessons"]
                .objectPairs[std::to_string(timetableLesson.first)]
                .format = JSON_INLINE;
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["lessons"]
                .objectPairs[std::to_string(timetableLesson.first)]
                .ints.push_back(timetableLesson.second.amount);
            for (size_t j = 0; j < timetableLesson.second.lessonTeacherPairs.size(); j++)
            {
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["lessons"]
                    .objectPairs[std::to_string(timetableLesson.first)]
                    .objects.push_back(JSONObject());
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["lessons"]
                    .objectPairs[std::to_string(timetableLesson.first)]
                    .objects[j]
                    .type = JSON_OBJECT;
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["lessons"]
                    .objectPairs[std::to_string(timetableLesson.first)]
                    .objects[j]
                    .format = JSON_INLINE;
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["lessons"]
                    .objectPairs[std::to_string(timetableLesson.first)]
                    .objects[j]
                    .intPairs["lessonID"] = timetableLesson.second.lessonTeacherPairs[j].lessonID;
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["lessons"]
                    .objectPairs[std::to_string(timetableLesson.first)]
                    .objects[j]
                    .intPairs["teacherID"] = timetableLesson.second.lessonTeacherPairs[j].teacherID;
            }
        }

        // Lesson numbers
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .objectPairs["lessonNumbers"] = JSONObject();
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .objectPairs["lessonNumbers"]
            .type = JSON_LIST;

        // Days
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .objectPairs["days"] = JSONObject();
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .objectPairs["days"]
            .type = JSON_LIST;

        classPair.second.days.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            classPair.second.days[i].lessons.resize(lessonsPerDay);
            // Lesson numbers
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["lessonNumbers"]
                .objects.push_back(JSONObject());
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["lessonNumbers"]
                .objects[i]
                .type = JSON_LIST;
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["lessonNumbers"]
                .objects[i]
                .format = JSON_INLINE;
            for (size_t j = 0; j < classPair.second.days[i].lessons.size(); j++)
            {
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["lessonNumbers"]
                    .objects[i]
                    .bools.push_back(classPair.second.days[i].lessons[j]);
            }

            // Days
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["days"]
                .objects.push_back(JSONObject());
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["days"]
                .objects[i]
                .type = JSON_LIST;
            for (size_t j = 0; j < classPair.second.days[i].classroomLessonPairs.size(); j++)
            {
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["days"]
                    .objects[i]
                    .objects.push_back(JSONObject());
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["days"]
                    .objects[i]
                    .objects[j]
                    .type = JSON_OBJECT;
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["days"]
                    .objects[i]
                    .objects[j]
                    .format = JSON_INLINE;
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["days"]
                    .objects[i]
                    .objects[j]
                    .intPairs["timetableLessonID"] =
                    classPair.second.days[i].classroomLessonPairs[j].timetableLessonID;
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["days"]
                    .objects[i]
                    .objects[j]
                    .objectPairs["classroomIDs"] = JSONObject();
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["days"]
                    .objects[i]
                    .objects[j]
                    .objectPairs["classroomIDs"]
                    .type = JSON_LIST;
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["days"]
                    .objects[i]
                    .objects[j]
                    .objectPairs["classroomIDs"]
                    .format = JSON_INLINE;
                for (size_t k = 0;
                     k < classPair.second.days[i].classroomLessonPairs[j].classroomIDs.size(); k++)
                {
                    jsonObject.objectPairs["classes"]
                        .objectPairs[std::to_string(classPair.first)]
                        .objectPairs["days"]
                        .objects[i]
                        .objects[j]
                        .objectPairs["classroomIDs"]
                        .ints.push_back(
                            classPair.second.days[i].classroomLessonPairs[j].classroomIDs[k]);
                }
            }
        }

        // Timetable lesson rules
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .objectPairs["timetableLessonRules"] = JSONObject();
        jsonObject.objectPairs["classes"]
            .objectPairs[std::to_string(classPair.first)]
            .objectPairs["timetableLessonRules"]
            .type = JSON_LIST;
        for (size_t i = 0; i < classPair.second.timetableLessonRules.size(); i++)
        {
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["timetableLessonRules"]
                .objects.push_back(JSONObject());
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["timetableLessonRules"]
                .objects[i]
                .type = JSON_OBJECT;
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["timetableLessonRules"]
                .objects[i]
                .boolPairs["preserveOrder"] =
                classPair.second.timetableLessonRules[i].preserveOrder;
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["timetableLessonRules"]
                .objects[i]
                .intPairs["amount"] = classPair.second.timetableLessonRules[i].amount;
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["timetableLessonRules"]
                .objects[i]
                .objectPairs["timetableLessonIDs"] = JSONObject();
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["timetableLessonRules"]
                .objects[i]
                .objectPairs["timetableLessonIDs"]
                .type = JSON_LIST;
            jsonObject.objectPairs["classes"]
                .objectPairs[std::to_string(classPair.first)]
                .objectPairs["timetableLessonRules"]
                .objects[i]
                .objectPairs["timetableLessonIDs"]
                .format = JSON_INLINE;
            for (size_t j = 0;
                 j < classPair.second.timetableLessonRules[i].timetableLessonIDs.size(); j++)
            {
                jsonObject.objectPairs["classes"]
                    .objectPairs[std::to_string(classPair.first)]
                    .objectPairs["timetableLessonRules"]
                    .objects[i]
                    .objectPairs["timetableLessonIDs"]
                    .ints.push_back(classPair.second.timetableLessonRules[i].timetableLessonIDs[j]);
            }
        }
    }

    SaveJSON(path, &jsonObject);
}

void Timetable::Load(const std::string& path)
{
    LogInfo("Loading timetable at " + path);
    JSONObject jsonObject;
    LoadJSON(path, &jsonObject);

    *this = Timetable();
    version = jsonObject.intPairs["version"];
    name = std::filesystem::path(path).stem().string();
    name = TrimJunk(name);
    year = jsonObject.intPairs["year"];
    if (year < 1900)
    {
        time_t now = time(0);
        tm* localTime = localtime(&now);
        year = 1900 + localTime->tm_year;
    }

    // Classrooms
    for (auto& classroom: jsonObject.objectPairs["classrooms"].stringPairs)
    {
        if (classroom.second == "") continue;
        int classroomID = stoi(classroom.first);
        if (classroomID > maxClassroomID) maxClassroomID = classroomID;
        classrooms[classroomID] = Classroom();
        classrooms[classroomID].name = classroom.second;
    }

    // Lessons
    for (auto& lesson: jsonObject.objectPairs["lessons"].objectPairs)
    {
        if (lesson.second.stringPairs["name"] == "") continue;
        int lessonID = stoi(lesson.first);
        if (lessonID > maxLessonID) maxLessonID = lessonID;
        lessons[lessonID] = Lesson();
        lessons[lessonID].name = lesson.second.stringPairs["name"];
        for (size_t i = 0; i < lesson.second.objectPairs["classIDs"].ints.size(); i++)
            lessons[lessonID].classIDs.push_back(
                lesson.second.objectPairs["classIDs"].ints[i]);
        for (size_t i = 0; i < lesson.second.objectPairs["classroomIDs"].ints.size(); i++)
            lessons[lessonID].classroomIDs.push_back(
                lesson.second.objectPairs["classroomIDs"].ints[i]);
    }

    // Teachers
    for (auto& teacher: jsonObject.objectPairs["teachers"].objectPairs)
    {
        if (teacher.second.stringPairs["name"] == "") continue;
        int teacherID = stoi(teacher.first);
        if (teacherID > maxTeacherID) maxTeacherID = teacherID;
        teachers[teacherID] = Teacher();
        teachers[teacherID].name = teacher.second.stringPairs["name"];
        for (size_t i = 0; i < teacher.second.objectPairs["lessonIDs"].ints.size(); i++)
            teachers[teacherID].lessonIDs.push_back(
                teacher.second.objectPairs["lessonIDs"].ints[i]);
        teachers[teacherID].workDays.resize(
            teacher.second.objectPairs["workDays"].objects.size());
        for (size_t i = 0; i < teacher.second.objectPairs["workDays"].objects.size(); i++)
        {
            for (size_t j = 0; j < teacher.second.objectPairs["workDays"].objects[i].ints.size();
                 j++)
            {
                teachers[teacherID].workDays[i].lessonIDs.push_back(
                    teacher.second.objectPairs["workDays"].objects[i].ints[j]);
            }
        }
    }

    // Classes
    for (auto& classPair: jsonObject.objectPairs["classes"].objectPairs)
    {
        if (classPair.second.stringPairs["number"] == "") continue;
        int classID = stoi(classPair.first);
        if (classID > maxClassID) maxClassID = classID;
        classes[classID] = Class();
        classes[classID].number = classPair.second.stringPairs["number"];
        classes[classID].letter = classPair.second.stringPairs["letter"];
        classes[classID].teacherID = classPair.second.intPairs["teacherID"];

        // Lessons
        for (auto& lesson: classPair.second.objectPairs["lessons"].objectPairs)
        {
            int lessonID = stoi(lesson.first);
            if (lessonID > classes[classID].maxTimetableLessonID)
                classes[classID].maxTimetableLessonID = lessonID;
            classes[classID].timetableLessons[lessonID] = TimetableLesson();
            classes[classID].timetableLessons[lessonID].amount = lesson.second.ints[0];
            for (size_t i = 0; i < lesson.second.objects.size(); i++)
            {
                classes[classID].timetableLessons[lessonID].lessonTeacherPairs.push_back(
                    LessonTeacherPair());
                classes[classID]
                    .timetableLessons[lessonID]
                    .lessonTeacherPairs[i]
                    .lessonID = lesson.second.objects[i].intPairs["lessonID"];
                classes[classID]
                    .timetableLessons[lessonID]
                    .lessonTeacherPairs[i]
                    .teacherID = lesson.second.objects[i].intPairs["teacherID"];
            }
        }
        classes[classID].days.resize(
            classPair.second.objectPairs["lessonNumbers"].objects.size());
        for (size_t i = 0; i < classPair.second.objectPairs["lessonNumbers"].objects.size(); i++)
        {
            for (size_t j = 0;
                 j < classPair.second.objectPairs["lessonNumbers"].objects[i].bools.size(); j++)
                classes[classID].days[i].lessons.push_back(
                    classPair.second.objectPairs["lessonNumbers"].objects[i].bools[j]);
        }

        // Days
        for (size_t i = 0; i < classPair.second.objectPairs["days"].objects.size(); i++)
        {
            for (size_t j = 0; j < classPair.second.objectPairs["days"].objects[i].objects.size();
                 j++)
            {
                classes[classID].days[i].classroomLessonPairs.push_back(
                    ClassroomLessonPair());
                classes[classID].days[i].classroomLessonPairs[j].timetableLessonID =
                    classPair.second.objectPairs["days"]
                        .objects[i]
                        .objects[j]
                        .intPairs["timetableLessonID"];
                for (size_t k = 0; k < classPair.second.objectPairs["days"]
                                           .objects[i]
                                           .objects[j]
                                           .objectPairs["classroomIDs"]
                                           .ints.size();
                     k++)
                {
                    classes[classID]
                        .days[i]
                        .classroomLessonPairs[j]
                        .classroomIDs.push_back(classPair.second.objectPairs["days"]
                                                    .objects[i]
                                                    .objects[j]
                                                    .objectPairs["classroomIDs"]
                                                    .ints[k]);
                }
            }
        }

        // Timetable lesson rules
        for (size_t i = 0; i < classPair.second.objectPairs["timetableLessonRules"].objects.size();
             i++)
        {
            classes[classID].timetableLessonRules.push_back(TimetableLessonRule());
            classes[classID].timetableLessonRules[i].preserveOrder =
                classPair.second.objectPairs["timetableLessonRules"]
                    .objects[i]
                    .boolPairs["preserveOrder"];
            classes[classID].timetableLessonRules[i].amount =
                classPair.second.objectPairs["timetableLessonRules"].objects[i].intPairs["amount"];
            for (size_t j = 0; j < classPair.second.objectPairs["timetableLessonRules"]
                                       .objects[i]
                                       .objectPairs["timetableLessonIDs"]
                                       .ints.size();
                 j++)
            {
                classes[classID].timetableLessonRules[i].timetableLessonIDs.push_back(
                    classPair.second.objectPairs["timetableLessonRules"]
                        .objects[i]
                        .objectPairs["timetableLessonIDs"]
                        .ints[j]);
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
            teachers[i].workDays[j].lessonIDs.resize(lessonsPerDay,
                                                                rand() % lessons.size());
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
                    classes[classID].timetableLessons[k].lessonTeacherPairs[0].lessonID =
                        it->first;
                }
                {
                    auto it = teachers.begin();
                    std::advance(it, rand() % teachers.size());
                    classes[classID]
                        .timetableLessons[k]
                        .lessonTeacherPairs[0]
                        .teacherID = it->first;
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
                    classes[classID].days[k].classroomLessonPairs.push_back(
                        ClassroomLessonPair());
                    classes[classID].days[k].classroomLessonPairs[m].timetableLessonID =
                        it->first;
                    for (size_t n = 0; n < it->second.lessonTeacherPairs.size(); n++)
                    {
                        int lessonID = it->second.lessonTeacherPairs[n].lessonID;
                        int classroomID =
                            lessons[lessonID]
                                .classroomIDs[rand() %
                                              lessons[lessonID].classroomIDs.size()];
                        classes[classID]
                            .days[k]
                            .classroomLessonPairs[m]
                            .classroomIDs.push_back(classroomID);
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
