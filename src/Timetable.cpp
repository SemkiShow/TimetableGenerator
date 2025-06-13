#include "Timetable.hpp"
#include "JSON.hpp"
#include "UI.hpp"

Timetable currentTimetable;
Timetable tmpTimetable;
Timetable tmpTmpTimetable;

void SaveTimetable(std::string path, Timetable* timetable)
{
    if (timetable->name == "") return;

    JSONObject jsonObject;
    jsonObject.type = JSON_OBJECT;

    // Classrooms
    jsonObject.objectPairs["classrooms"] = JSONObject();
    jsonObject.objectPairs["classrooms"].type = JSON_OBJECT;
    jsonObject.objectPairs["classrooms"].format = JSON_INLINE;
    for (auto& classroom: timetable->classrooms)
        jsonObject.objectPairs["classrooms"].stringPairs[std::to_string(classroom.first)] = classroom.second.name;

    // Lessons
    jsonObject.objectPairs["lessons"] = JSONObject();
    jsonObject.objectPairs["lessons"].type = JSON_OBJECT;
    for (auto& lesson: timetable->lessons)
    {
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)] = JSONObject();
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].type = JSON_OBJECT;

        // Lesson name
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].stringPairs["name"] = lesson.second.name;

        // Class names
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].objectPairs["classIDs"] = JSONObject();
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].objectPairs["classIDs"].type = JSON_LIST;
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].objectPairs["classIDs"].format = JSON_INLINE;
        for (int j = 0; j < lesson.second.classIDs.size(); j++)
        {
            jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].objectPairs["classIDs"].ints.push_back(lesson.second.classIDs[j]);
        }

        // Classrooms
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].objectPairs["classroomIDs"].type = JSON_LIST;
        jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].objectPairs["classroomIDs"].format = JSON_INLINE;
        for (int j = 0; j < lesson.second.classroomIDs.size(); j++)
        {
            jsonObject.objectPairs["lessons"].objectPairs[std::to_string(lesson.first)].objectPairs["classroomIDs"].ints.push_back(lesson.second.classroomIDs[j]);
        }
    }

    // Teachers
    jsonObject.objectPairs["teachers"] = JSONObject();
    jsonObject.objectPairs["teachers"].type = JSON_OBJECT;
    for (auto& teacher: timetable->teachers)
    {
        // Teacher
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)] = JSONObject();
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].type = JSON_OBJECT;

        // Teacher name
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].stringPairs["name"] = teacher.second.name;

        // Lessons
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["lessonIDs"] = JSONObject();
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["lessonIDs"].type = JSON_LIST;
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["lessonIDs"].format = JSON_INLINE;
        for (int j = 0; j < teacher.second.lessonIDs.size(); j++)
            jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["lessonIDs"].ints.push_back(
            teacher.second.lessonIDs[j]);

        // Work days
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["workDays"] = JSONObject();
        jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["workDays"].type = JSON_LIST;
        for (int j = 0; j < 7; j++)
        {
            jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["workDays"].objects.push_back(JSONObject());
            jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["workDays"].objects[j].type = JSON_LIST;
            jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["workDays"].objects[j].format = JSON_INLINE;
            // Lesson number
            for (int k = 0; k < teacher.second.workDays[j].lessonNumbers.size(); k++)
            {
                jsonObject.objectPairs["teachers"].objectPairs[std::to_string(teacher.first)].objectPairs["workDays"].objects[j].ints.push_back(
                    teacher.second.workDays[j].lessonNumbers[k]);
            }
        }
    }

    // Classes
    jsonObject.objectPairs["classes"] = JSONObject();
    jsonObject.objectPairs["classes"].type = JSON_OBJECT;
    for (auto& classPair: timetable->classes)
    {
        // Class
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)] = JSONObject();
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].type = JSON_OBJECT;

        // Class number and letter
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].stringPairs["number"] =
            classPair.second.number;
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].stringPairs["letter"] =
            classPair.second.letter;

        // Teacher name
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].intPairs["teacherID"] =
            classPair.second.teacherID;

        // Lessons
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"] = JSONObject();
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].type = JSON_OBJECT;
        for (auto& timetableLesson: classPair.second.timetableLessons)
        {
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)] = JSONObject();
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)].type = JSON_LIST;
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)].format = JSON_INLINE;
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)].ints.push_back(
                timetableLesson.second.amount);
            for (int k = 0; k < timetableLesson.second.lessonTeacherPairs.size(); k++)
            {
                jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)].objects.push_back(JSONObject());
                jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)].objects[k].type = JSON_OBJECT;
                jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)].objects[k].format = JSON_INLINE;
                jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)].objects[k].intPairs["lessonID"] =
                    timetableLesson.second.lessonTeacherPairs[k].lessonID;
                jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessons"].objectPairs[std::to_string(timetableLesson.first)].objects[k].intPairs["teacherID"] =
                    timetableLesson.second.lessonTeacherPairs[k].teacherID;
            }
        }

        // Lesson numbers
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessonNumbers"] = JSONObject();
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessonNumbers"].type = JSON_LIST;

        // Days
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["days"] = JSONObject();
        jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["days"].type = JSON_LIST;

        for (int j = 0; j < 7; j++)
        {
            // Lesson numbers
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessonNumbers"].objects.push_back(JSONObject());
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessonNumbers"].objects[j].type = JSON_LIST;
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessonNumbers"].objects[j].format = JSON_INLINE;
            for (int k = 0; k < classPair.second.days[j].lessonNumbers.size(); k++)
            {
                jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["lessonNumbers"].objects[j].ints.push_back(
                    classPair.second.days[j].lessonNumbers[k]);
            }

            // Days
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["days"].objects.push_back(JSONObject());
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["days"].objects[j].type = JSON_LIST;
            jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["days"].objects[j].format = JSON_INLINE;
            for (int k = 0; k < classPair.second.days[j].timetableLessonIDs.size(); k++)
            {
                jsonObject.objectPairs["classes"].objectPairs[std::to_string(classPair.first)].objectPairs["days"].objects[j].ints.push_back(
                    classPair.second.days[j].timetableLessonIDs[k]);
            }
        }
    }

    SaveJSON(path, &jsonObject);
}

std::string ExtractNumberFromBeginning(std::string input)
{
    for (int i = 0; i < input.size(); i++)
    {
        try
        {
            int number = stoi(input.substr(0, i));
            return std::to_string(number);
        }
        catch (const std::exception&) {}
    }
    return "-1";
}

void LoadTimetable(std::string path, Timetable* timetable)
{
    JSONObject jsonObject;
    LoadJSON(path, &jsonObject);

    timetable->name = std::filesystem::path(path).stem().string();

    // Classrooms
    for (auto& classroom: jsonObject.objectPairs["classrooms"].stringPairs)
    {
        if (stoi(classroom.first) > timetable->maxClassroomID)
            timetable->maxClassroomID = stoi(classroom.first);
        timetable->classrooms[stoi(classroom.first)] = Classroom();
        timetable->classrooms[stoi(classroom.first)].name = classroom.second;
    }

    // Lessons
    for (auto& lesson: jsonObject.objectPairs["lessons"].objectPairs)
    {
        if (stoi(lesson.first) > timetable->maxLessonID)
            timetable->maxLessonID = stoi(lesson.first);
        timetable->lessons[stoi(lesson.first)] = Lesson();
        timetable->lessons[stoi(lesson.first)].name = lesson.second.stringPairs["name"];
        for (int i = 0; i < lesson.second.objectPairs["classIDs"].ints.size(); i++)
            timetable->lessons[stoi(lesson.first)].classIDs.push_back(lesson.second.objectPairs["classIDs"].ints[i]);
        for (int i = 0; i < lesson.second.objectPairs["classroomIDs"].ints.size(); i++)
            timetable->lessons[stoi(lesson.first)].classroomIDs.push_back(lesson.second.objectPairs["classroomIDs"].ints[i]);
    }

    // Teachers
    for (auto& teacher: jsonObject.objectPairs["teachers"].objectPairs)
    {
        if (stoi(teacher.first) > timetable->maxTeacherID)
            timetable->maxTeacherID = stoi(teacher.first);
        timetable->teachers[stoi(teacher.first)] = Teacher();
        timetable->teachers[stoi(teacher.first)].name = teacher.second.stringPairs["name"];
        for (int i = 0; i < teacher.second.objectPairs["lessonIDs"].ints.size(); i++)
            timetable->teachers[stoi(teacher.first)].lessonIDs.push_back(teacher.second.objectPairs["lessonIDs"].ints[i]);
        for (int i = 0; i < teacher.second.objectPairs["workDays"].objects.size(); i++)
        {
            for (int j = 0; j < teacher.second.objectPairs["workDays"].objects[i].ints.size(); j++)
            {
                timetable->teachers[stoi(teacher.first)].workDays[i].lessonNumbers.push_back(teacher.second.objectPairs["workDays"].objects[i].ints[j]);
            }
        }
    }

    // Classes
    for (auto& classPair: jsonObject.objectPairs["classes"].objectPairs)
    {
        if (stoi(classPair.first) > timetable->maxClassID)
            timetable->maxClassID = stoi(classPair.first);
        timetable->classes[stoi(classPair.first)] = Class();
        timetable->classes[stoi(classPair.first)].number = classPair.second.stringPairs["number"];
        timetable->classes[stoi(classPair.first)].letter = classPair.second.stringPairs["letter"];
        timetable->classes[stoi(classPair.first)].teacherID = classPair.second.intPairs["teacherID"];
        for (auto& lesson : classPair.second.objectPairs["lessons"].objectPairs)
        {
            if (stoi(lesson.first) > timetable->classes[stoi(classPair.first)].maxTimetableLessonID)
                timetable->classes[stoi(classPair.first)].maxTimetableLessonID = stoi(lesson.first);
            timetable->classes[stoi(classPair.first)].timetableLessons[stoi(lesson.first)] = TimetableLesson();
            timetable->classes[stoi(classPair.first)].timetableLessons[stoi(lesson.first)].amount = lesson.second.ints[0];
            for (int i = 0; i < lesson.second.objects.size(); i++)
            {
                timetable->classes[stoi(classPair.first)].timetableLessons[stoi(lesson.first)].lessonTeacherPairs.push_back(LessonTeacherPair());
                timetable->classes[stoi(classPair.first)].timetableLessons[stoi(lesson.first)].lessonTeacherPairs[i].lessonID =
                    lesson.second.objects[i].intPairs["lessonID"];
                timetable->classes[stoi(classPair.first)].timetableLessons[stoi(lesson.first)].lessonTeacherPairs[i].teacherID =
                    lesson.second.objects[i].intPairs["teacherID"];
            }
        }
        for (int i = 0; i < classPair.second.objectPairs["lessonNumbers"].objects.size(); i++)
        {
            for (int j = 0; j < classPair.second.objectPairs["lessonNumbers"].objects[i].ints.size(); j++)
                timetable->classes[stoi(classPair.first)].days[i].lessonNumbers.push_back(
                    classPair.second.objectPairs["lessonNumbers"].objects[i].ints[j]);
        }
        for (int i = 0; i < classPair.second.objectPairs["days"].objects.size(); i++)
        {
            for (int j = 0; j < classPair.second.objectPairs["days"].objects[i].ints.size(); j++)
            {
                timetable->classes[stoi(classPair.first)].days[i].timetableLessonIDs.push_back(
                    classPair.second.objectPairs["days"].objects[i].ints[j]);
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
                std::cout << "Adding items to the orderedClasses list... " << tmpClasses.size() << " items left\n";
                continue;
            }
            it++;
        }
    }

    if (timetable == &currentTimetable) tmpTmpTimetable = tmpTimetable = currentTimetable;
}

void GenerateRandomTimetable(Timetable* timetable)
{
    for (int i = 0; i < 10; i++)
    {
        timetable->classrooms[i] = Classroom();
        for (int j = 0; j < 3; j++)
        {
            timetable->classrooms[i].name += '0' + rand() % 10;
        }
    }
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
        for (int j = 0; j < 7; j++)
        {
            for (int k = 0; k < rand() % 15; k++)
                timetable->teachers[i].workDays[j].lessonNumbers.push_back(rand() % 8);
        }
    }
    int classLettersPerClassNumber = 3;
    for (int i = 1; i < 3; i++)
    {
        for (int m = 0; m < classLettersPerClassNumber; m++)
        {
            timetable->classes[(i-1)*classLettersPerClassNumber + m] = Class();
            timetable->classes[(i-1)*classLettersPerClassNumber + m].number = std::to_string(i);
            timetable->classes[(i-1)*classLettersPerClassNumber + m].letter = 'a' + m;
            {
                auto it = timetable->teachers.begin();
                std::advance(it, rand() % timetable->teachers.size());
                timetable->classes[(i-1)*classLettersPerClassNumber + m].teacherID = it->first;
            }
            for (int j = 0; j < 5; j++)
            {
                timetable->classes[(i-1)*classLettersPerClassNumber + m].timetableLessons[j] = TimetableLesson();
                timetable->classes[(i-1)*classLettersPerClassNumber + m].timetableLessons[j].lessonTeacherPairs.push_back(LessonTeacherPair());
                {
                    auto it = timetable->lessons.begin();
                    std::advance(it, rand() % timetable->lessons.size());
                    timetable->classes[(i-1)*classLettersPerClassNumber + m].timetableLessons[j].lessonTeacherPairs[0].lessonID = it->first;
                }
                {
                    auto it = timetable->teachers.begin();
                    std::advance(it, rand() % timetable->teachers.size());
                    timetable->classes[(i-1)*classLettersPerClassNumber + m].timetableLessons[j].lessonTeacherPairs[0].teacherID = it->first;
                }
            }
            for (int j = 0; j < 7; j++)
            {
                for (int k = 0; k < rand() % 15; k++)
                    timetable->classes[(i-1)*classLettersPerClassNumber + m].days[j].lessonNumbers.push_back(rand() % 8);
                for (int k = 0; k < 3; k++)
                {
                    auto it = timetable->classes[(i-1)*classLettersPerClassNumber + m].timetableLessons.begin();
                    std::advance(it, rand() % timetable->classes[(i-1)*classLettersPerClassNumber + m].timetableLessons.size());
                    timetable->classes[(i-1)*classLettersPerClassNumber + m].days[j].timetableLessonIDs.push_back(it->first);
                }
            }
        }

    }
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
