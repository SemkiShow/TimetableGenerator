#include "Timetable.hpp"
#include "JSON.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include <filesystem>
#include <iostream>

Timetable currentTimetable;
Timetable tmpTimetable;
Timetable tmpTmpTimetable;

void SaveTimetable(std::string path, Timetable* timetable)
{
    LogInfo("Saving timetable at " + path);
    if (timetable->name == "") return;

    JSONObject jsonObject;
    jsonObject.type = JSON_OBJECT;

    // Classrooms
    jsonObject.objectPairs["classrooms"] = JSONObject();
    jsonObject.objectPairs["classrooms"].type = JSON_OBJECT;
    jsonObject.objectPairs["classrooms"].format = JSON_INLINE;
    for (auto& classroom : timetable->classrooms)
        jsonObject.objectPairs["classrooms"].stringPairs[std::to_string(classroom.first)] =
            classroom.second.name;

    // Lessons
    jsonObject.objectPairs["lessons"] = JSONObject();
    jsonObject.objectPairs["lessons"].type = JSON_OBJECT;
    for (auto& lesson : timetable->lessons)
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
        for (int i = 0; i < lesson.second.classIDs.size(); i++)
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
        for (int i = 0; i < lesson.second.classroomIDs.size(); i++)
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
    for (auto& teacher : timetable->teachers)
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
        for (int i = 0; i < teacher.second.lessonIDs.size(); i++)
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
        for (int i = 0; i < daysPerWeek; i++)
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
            for (int j = 0; j < teacher.second.workDays[i].lessonIDs.size(); j++)
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
    for (auto& classPair : timetable->classes)
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

        // Teacher name
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
        for (auto& timetableLesson : classPair.second.timetableLessons)
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
            for (int j = 0; j < timetableLesson.second.lessonTeacherPairs.size(); j++)
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
        for (int i = 0; i < daysPerWeek; i++)
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
            for (int j = 0; j < classPair.second.days[i].lessons.size(); j++)
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
            for (int j = 0; j < classPair.second.days[i].classroomLessonPairs.size(); j++)
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
                for (int k = 0;
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
    }

    SaveJSON(path, &jsonObject);
}

void LoadTimetable(std::string path, Timetable* timetable)
{
    LogInfo("Loading timetable at " + path);
    JSONObject jsonObject;
    LoadJSON(path, &jsonObject);

    *timetable = Timetable();
    timetable->name = std::filesystem::path(path).stem().string();
    timetable->name = TrimJunk(timetable->name);

    // Classrooms
    for (auto& classroom : jsonObject.objectPairs["classrooms"].stringPairs)
    {
        if (classroom.second == "") continue;
        int classroomID = stoi(classroom.first);
        if (classroomID > timetable->maxClassroomID) timetable->maxClassroomID = classroomID;
        timetable->classrooms[classroomID] = Classroom();
        timetable->classrooms[classroomID].name = classroom.second;
    }

    // Lessons
    for (auto& lesson : jsonObject.objectPairs["lessons"].objectPairs)
    {
        if (lesson.second.stringPairs["name"] == "") continue;
        int lessonID = stoi(lesson.first);
        if (lessonID > timetable->maxLessonID) timetable->maxLessonID = lessonID;
        timetable->lessons[lessonID] = Lesson();
        timetable->lessons[lessonID].name = lesson.second.stringPairs["name"];
        for (int i = 0; i < lesson.second.objectPairs["classIDs"].ints.size(); i++)
            timetable->lessons[lessonID].classIDs.push_back(
                lesson.second.objectPairs["classIDs"].ints[i]);
        for (int i = 0; i < lesson.second.objectPairs["classroomIDs"].ints.size(); i++)
            timetable->lessons[lessonID].classroomIDs.push_back(
                lesson.second.objectPairs["classroomIDs"].ints[i]);
    }

    // Teachers
    for (auto& teacher : jsonObject.objectPairs["teachers"].objectPairs)
    {
        if (teacher.second.stringPairs["name"] == "") continue;
        int teacherID = stoi(teacher.first);
        if (teacherID > timetable->maxTeacherID) timetable->maxTeacherID = teacherID;
        timetable->teachers[teacherID] = Teacher();
        timetable->teachers[teacherID].name = teacher.second.stringPairs["name"];
        for (int i = 0; i < teacher.second.objectPairs["lessonIDs"].ints.size(); i++)
            timetable->teachers[teacherID].lessonIDs.push_back(
                teacher.second.objectPairs["lessonIDs"].ints[i]);
        timetable->teachers[teacherID].workDays.resize(
            teacher.second.objectPairs["workDays"].objects.size());
        for (int i = 0; i < teacher.second.objectPairs["workDays"].objects.size(); i++)
        {
            for (int j = 0; j < teacher.second.objectPairs["workDays"].objects[i].ints.size(); j++)
            {
                timetable->teachers[teacherID].workDays[i].lessonIDs.push_back(
                    teacher.second.objectPairs["workDays"].objects[i].ints[j]);
            }
        }
    }

    // Classes
    for (auto& classPair : jsonObject.objectPairs["classes"].objectPairs)
    {
        if (classPair.second.stringPairs["number"] == "") continue;
        int classID = stoi(classPair.first);
        if (classID > timetable->maxClassID) timetable->maxClassID = classID;
        timetable->classes[classID] = Class();
        timetable->classes[classID].number = classPair.second.stringPairs["number"];
        timetable->classes[classID].letter = classPair.second.stringPairs["letter"];
        timetable->classes[classID].teacherID = classPair.second.intPairs["teacherID"];
        for (auto& lesson : classPair.second.objectPairs["lessons"].objectPairs)
        {
            int lessonID = stoi(lesson.first);
            if (lessonID > timetable->classes[classID].maxTimetableLessonID)
                timetable->classes[classID].maxTimetableLessonID = lessonID;
            timetable->classes[classID].timetableLessons[lessonID] = TimetableLesson();
            timetable->classes[classID].timetableLessons[lessonID].amount = lesson.second.ints[0];
            for (int i = 0; i < lesson.second.objects.size(); i++)
            {
                timetable->classes[classID].timetableLessons[lessonID].lessonTeacherPairs.push_back(
                    LessonTeacherPair());
                timetable->classes[classID]
                    .timetableLessons[lessonID]
                    .lessonTeacherPairs[i]
                    .lessonID = lesson.second.objects[i].intPairs["lessonID"];
                timetable->classes[classID]
                    .timetableLessons[lessonID]
                    .lessonTeacherPairs[i]
                    .teacherID = lesson.second.objects[i].intPairs["teacherID"];
            }
        }
        timetable->classes[classID].days.resize(
            classPair.second.objectPairs["lessonNumbers"].objects.size());
        for (int i = 0; i < classPair.second.objectPairs["lessonNumbers"].objects.size(); i++)
        {
            for (int j = 0;
                 j < classPair.second.objectPairs["lessonNumbers"].objects[i].bools.size(); j++)
                timetable->classes[classID].days[i].lessons.push_back(
                    classPair.second.objectPairs["lessonNumbers"].objects[i].bools[j]);
        }
        for (int i = 0; i < classPair.second.objectPairs["days"].objects.size(); i++)
        {
            for (int j = 0; j < classPair.second.objectPairs["days"].objects[i].objects.size(); j++)
            {
                timetable->classes[classID].days[i].classroomLessonPairs.push_back(
                    ClassroomLessonPair());
                timetable->classes[classID].days[i].classroomLessonPairs[j].timetableLessonID =
                    classPair.second.objectPairs["days"]
                        .objects[i]
                        .objects[j]
                        .intPairs["timetableLessonID"];
                for (int k = 0; k < classPair.second.objectPairs["days"]
                                        .objects[i]
                                        .objects[j]
                                        .objectPairs["classroomIDs"]
                                        .ints.size();
                     k++)
                {
                    timetable->classes[classID]
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
    }

    std::map<int, Class> tmpClasses = timetable->classes;
    int classesLeft = tmpClasses.size();
    while (tmpClasses.size() > 0)
    {
        std::string number = tmpClasses.begin()->second.number;
        for (auto it = tmpClasses.begin(); it != tmpClasses.end();)
        {
            if (it->second.number == number)
            {
                timetable->orderedClasses.push_back(it->first);
                it = tmpClasses.erase(it);
                std::cout << "Adding items to the orderedClasses list... " << tmpClasses.size()
                          << " items left\n";
                continue;
            }
            ++it;
        }
    }

    if (timetable == &currentTimetable) tmpTmpTimetable = tmpTimetable = currentTimetable;
}

void GenerateRandomTimetable(Timetable* timetable)
{
    // Classrooms
    for (int i = 0; i < 10; i++)
    {
        timetable->classrooms[i] = Classroom();
        for (int j = 0; j < 3; j++)
        {
            timetable->classrooms[i].name += '0' + rand() % 10;
        }
    }

    // Lessons
    for (int i = 0; i < 15; i++)
    {
        timetable->lessons[i] = Lesson();
        for (int j = 0; j < 7; j++)
            timetable->lessons[i].name += 'a' + rand() % 26;
        for (int j = 0; j < 4; j++)
        {
            auto it = timetable->classrooms.begin();
            std::advance(it, rand() % timetable->classrooms.size());
            timetable->lessons[i].classroomIDs.push_back(it->first);
        }
    }

    // Teachers
    for (int i = 0; i < 5; i++)
    {
        timetable->teachers[i] = Teacher();
        for (int j = 0; j < 7; j++)
            timetable->teachers[i].name += 'a' + rand() % 26;
        for (int j = 0; j < 3; j++)
        {
            auto it = timetable->lessons.begin();
            std::advance(it, rand() % timetable->lessons.size());
            timetable->teachers[i].lessonIDs.push_back(it->first);
        }
        timetable->teachers[i].workDays.resize(daysPerWeek);
        for (int j = 0; j < daysPerWeek; j++)
        {
            timetable->teachers[i].workDays[j].lessonIDs.resize(lessonsPerDay,
                                                                rand() % timetable->lessons.size());
        }
    }

    // Classes
    int classLettersPerClassNumber = 3;
    for (int i = 1; i < 3; i++)
    {
        for (int j = 0; j < classLettersPerClassNumber; j++)
        {
            int classID = (i - 1) * classLettersPerClassNumber + j;
            timetable->classes[classID] = Class();
            timetable->classes[classID].number = std::to_string(i);
            timetable->classes[classID].letter = 'a' + j;
            {
                auto it = timetable->teachers.begin();
                std::advance(it, rand() % timetable->teachers.size());
                timetable->classes[classID].teacherID = it->first;
            }
            for (int k = 0; k < 5; k++)
            {
                timetable->classes[classID].timetableLessons[k] = TimetableLesson();
                timetable->classes[classID].timetableLessons[k].lessonTeacherPairs.push_back(
                    LessonTeacherPair());
                {
                    auto it = timetable->lessons.begin();
                    std::advance(it, rand() % timetable->lessons.size());
                    timetable->classes[classID].timetableLessons[k].lessonTeacherPairs[0].lessonID =
                        it->first;
                }
                {
                    auto it = timetable->teachers.begin();
                    std::advance(it, rand() % timetable->teachers.size());
                    timetable->classes[classID]
                        .timetableLessons[k]
                        .lessonTeacherPairs[0]
                        .teacherID = it->first;
                }
            }
            timetable->classes[classID].days.resize(daysPerWeek);
            for (int k = 0; k < daysPerWeek; k++)
            {
                timetable->classes[classID].days[k].lessons.reserve(lessonsPerDay);
                for (int m = 0; m < lessonsPerDay; m++)
                {
                    timetable->classes[classID].days[k].lessons.push_back(rand() % 2 == 0);

                    auto it = timetable->classes[classID].timetableLessons.begin();
                    std::advance(it, rand() % timetable->classes[classID].timetableLessons.size());
                    timetable->classes[classID].days[k].classroomLessonPairs.push_back(
                        ClassroomLessonPair());
                    timetable->classes[classID].days[k].classroomLessonPairs[m].timetableLessonID =
                        it->first;
                    for (int n = 0; n < it->second.lessonTeacherPairs.size(); n++)
                    {
                        int lessonID = it->second.lessonTeacherPairs[n].lessonID;
                        int classroomID =
                            timetable->lessons[lessonID]
                                .classroomIDs[rand() %
                                              timetable->lessons[lessonID].classroomIDs.size()];
                        timetable->classes[classID]
                            .days[k]
                            .classroomLessonPairs[m]
                            .classroomIDs.push_back(classroomID);
                    }
                }
            }
        }
    }

    // Assigning classes to lessons
    for (int i = 0; i < timetable->lessons.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            auto it = timetable->classes.begin();
            std::advance(it, rand() % (timetable->classes.size() - 1));
            timetable->lessons[i].classIDs.push_back(it->first);
        }
    }
}
