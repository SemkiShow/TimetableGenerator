#include "Timetable.hpp"
#include "JSON.hpp"
#include "UI.hpp"

Timetable currentTimetable;

void SaveTimetable(std::string path, Timetable* timetable)
{
    if (timetable->name == "") return;

    JSONObject jsonObject;
    jsonObject.type = JSON_OBJECT;

    // Classrooms
    jsonObject.objectPairs["classrooms"] = JSONObject();
    jsonObject.objectPairs["classrooms"].type = JSON_LIST;
    jsonObject.objectPairs["classrooms"].format = JSON_INLINE;
    for (int i = 0; i < timetable->classrooms.size(); i++)
    {
        jsonObject.objectPairs["classrooms"].strings.push_back(timetable->classrooms[i].name);
    }

    // Lessons
    jsonObject.objectPairs["lessons"] = JSONObject();
    jsonObject.objectPairs["lessons"].type = JSON_OBJECT;
    for (int i = 0; i < timetable->lessons.size(); i++)
    {
        jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id] = JSONObject();
        jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].type = JSON_OBJECT;

        // Lesson name
        jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].stringPairs["name"] = timetable->lessons[i].name;

        // Class names
        jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].objectPairs["classNames"] = JSONObject();
        jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].objectPairs["classNames"].type = JSON_LIST;
        jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].objectPairs["classNames"].format = JSON_INLINE;
        for (int j = 0; j < timetable->lessons[i].classNames.size(); j++)
        {
            jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].objectPairs["classNames"].strings.push_back(timetable->lessons[i].classNames[j]);
        }

        // Classrooms
        jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].objectPairs["classrooms"].type = JSON_LIST;
        jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].objectPairs["classrooms"].format = JSON_INLINE;
        for (int j = 0; j < timetable->lessons[i].classrooms.size(); j++)
        {
            jsonObject.objectPairs["lessons"].objectPairs[timetable->lessons[i].id].objectPairs["classrooms"].strings.push_back(timetable->lessons[i].classrooms[j]->name);
        }
    }

    // Teachers
    jsonObject.objectPairs["teachers"] = JSONObject();
    jsonObject.objectPairs["teachers"].type = JSON_OBJECT;
    for (int i = 0; i < timetable->teachers.size(); i++)
    {
        // Teacher name
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name] = JSONObject();
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].type = JSON_OBJECT;

        // Lessons
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["lessons"] = JSONObject();
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["lessons"].type = JSON_LIST;
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["lessons"].format = JSON_INLINE;
        for (int j = 0; j < timetable->teachers[i].lessons.size(); j++)
            jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["lessons"].strings.push_back(timetable->teachers[i].lessons[j]->id);

        // Work days
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["workDays"] = JSONObject();
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["workDays"].type = JSON_LIST;
        for (int j = 0; j < 7; j++)
        {
            jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["workDays"].objects.push_back(JSONObject());
            jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["workDays"].objects[j].type = JSON_LIST;
            jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["workDays"].objects[j].format = JSON_INLINE;
            // Lesson number
            for (int k = 0; k < timetable->teachers[i].workTime[j].lessonNumbers.size(); k++)
            {
                jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objectPairs["workDays"].objects[j].ints.push_back(
                    timetable->teachers[i].workTime[j].lessonNumbers[k]);
            }
        }
    }

    // Classes
    jsonObject.objectPairs["classes"] = JSONObject();
    jsonObject.objectPairs["classes"].type = JSON_OBJECT;
    for (int i = 0; i < timetable->classes.size(); i++)
    {
        // Class name
        jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name] = JSONObject();
        jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].type = JSON_OBJECT;

        // Teacher name
        jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].stringPairs["teacher"] = timetable->classes[i].teacher->name;

        // Lessons
        jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"] = JSONObject();
        jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].type = JSON_LIST;

        // Day
        for (int j = 0; j < 7; j++)
        {
            jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects.push_back(JSONObject());
            jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].type = JSON_LIST;

            // Lesson
            for (int k = 0; k < timetable->classes[i].days[j].lessons.size(); k++)
            {
                jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects.push_back(JSONObject());
                jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].type = JSON_LIST;
                jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].format = JSON_INLINE;
                for (int m = 0; m < timetable->classes[i].days[j].lessons[k].lessons.size(); m++)
                {
                    jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].objects.push_back(JSONObject());
                    jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].objects[m].type = JSON_OBJECT;
                    jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].objects[m].format = JSON_INLINE;
                    jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].objects[m].stringPairs["id"] =
                        timetable->classes[i].days[j].lessons[k].lessons[m]->id;
                    jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].objects[m].stringPairs["teacher"] =
                        timetable->classes[i].days[j].lessons[k].teachers[m]->name;
                }
            }
        }
    }

    SaveJSON(path, &jsonObject);
}

void LoadTimetable(std::string path, Timetable* timetable)
{
    JSONObject jsonObject;
    LoadJSON(path, &jsonObject);

    timetable->name = std::filesystem::path(path).stem().string();

    // Classrooms
    for (int i = 0; i < jsonObject.objectPairs["classrooms"].strings.size(); i++)
    {
        timetable->classrooms.push_back(Classroom());
        timetable->classrooms[i].name = jsonObject.objectPairs["classrooms"].strings[i];
    }

    // Lessons
    int i = 0;
    for (auto lesson: jsonObject.objectPairs["lessons"].objectPairs)
    {
        timetable->lessons.push_back(Lesson());
        timetable->lessons[i].id = lesson.first;
        timetable->lessons[i].name = lesson.second.stringPairs["name"];
        for (int j = 0; j < lesson.second.objectPairs["classNames"].strings.size(); j++)
            timetable->lessons[i].classNames.push_back(lesson.second.objectPairs["classNames"].strings[j]);
        for (int j = 0; j < lesson.second.objectPairs["classrooms"].strings.size(); j++)
        {
            for (int k = 0; k < timetable->classrooms.size(); k++)
            {
                if (lesson.second.objectPairs["classrooms"].strings[j] == timetable->classrooms[k].name)
                {
                    timetable->lessons[i].classrooms.push_back(&timetable->classrooms[k]);
                    break;
                }
            }
        }
        i++;
    }

    // Teachers
    i = 0;
    for (auto teacher: jsonObject.objectPairs["teachers"].objectPairs)
    {
        timetable->teachers.push_back(Teacher());
        timetable->teachers[i].name = teacher.first;
        for (int j = 0; j < teacher.second.objectPairs["lessons"].strings.size(); j++)
        {
            for (int k = 0; k < timetable->lessons.size(); k++)
            {
                if (teacher.second.objectPairs["lessons"].strings[j] == timetable->lessons[k].id)
                {
                    timetable->teachers[i].lessons.push_back(&timetable->lessons[k]);
                    break;
                }
            }
        }
        for (int j = 0; j < teacher.second.objectPairs["workDays"].objects.size(); j++)
        {
            for (int k = 0; k < teacher.second.objectPairs["workDays"].objects[j].ints.size(); k++)
            {
                timetable->teachers[i].workTime[j].lessonNumbers.push_back(teacher.second.objectPairs["workDays"].objects[j].ints[k]);
            }
        }
        i++;
    }

    // Classes
    i = 0;
    for (auto classPair: jsonObject.objectPairs["classes"].objectPairs)
    {
        timetable->classes.push_back(Class());
        timetable->classes[i].name = classPair.first;
        for (int j = 0; j < timetable->teachers.size(); j++)
        {
            if (classPair.second.stringPairs["teacher"] == timetable->teachers[j].name)
            {
                timetable->classes[i].teacher = &timetable->teachers[j];
                break;
            }
        }
        for (int j = 0; j < classPair.second.objectPairs["lessons"].objects.size(); j++)
        {
            for (int k = 0; k < classPair.second.objectPairs["lessons"].objects[j].objects.size(); k++)
            {
                timetable->classes[i].days[j].lessons.push_back(TimetableLesson());
                for (int m = 0; m < classPair.second.objectPairs["lessons"].objects[j].objects[k].objects.size(); m++)
                {
                    std::string lessonID = classPair.second.objectPairs["lessons"].objects[j].objects[k].objects[m].stringPairs["id"];
                    for (int n = 0; n < timetable->lessons.size(); n++)
                    {
                        if (lessonID == timetable->lessons[n].id)
                        {
                            timetable->classes[i].days[j].lessons[k].lessons.push_back(&timetable->lessons[n]);
                            break;
                        }
                    }
                    std::string teacherName = classPair.second.objectPairs["lessons"].objects[j].objects[k].objects[m].stringPairs["teacher"];
                    for (int n = 0; n < timetable->teachers.size(); n++)
                    {
                        if (teacherName == timetable->teachers[n].name)
                        {
                            timetable->classes[i].days[j].lessons[k].teachers.push_back(&timetable->teachers[n]);
                            break;
                        }
                    }
                }
            }
        }
        i++;
    }

    // UI-related setup
    classrooms = "";
    for (int i = 0; i < timetable->classrooms.size(); i++)
    {
        classrooms += timetable->classrooms[i].name;
        if (i < timetable->classrooms.size()-1) classrooms += "\n";
    }
}

void GenerateRandomTimetable(Timetable* timetable)
{
    for (int i = 0; i < 10; i++)
    {
        timetable->classrooms.push_back(Classroom());
        for (int j = 0; j < 3; j++)
        {
            timetable->classrooms[i].name += '0' + rand() % 10;
        }
    }
    for (int i = 0; i < 15; i++)
    {
        timetable->lessons.push_back(Lesson());
        timetable->lessons[i].id = std::to_string(i);
        for (int j = 0; j < 7; j++)
        {
            timetable->lessons[i].name += 'a' + rand() % 26;
        }
        for (int j = 0; j < 4; j++)
        {
            timetable->lessons[i].classrooms.push_back(&timetable->classrooms[rand() % timetable->classrooms.size()]);
        }
    }
    for (int i = 0; i < 5; i++)
    {
        timetable->teachers.push_back(Teacher());
        for (int j = 0; j < 7; j++)
        {
            timetable->teachers[i].name += 'a' + rand() % 26;
        }
        for (int j = 0; j < 3; j++)
        {
            timetable->teachers[i].lessons.push_back(&timetable->lessons[rand() % timetable->lessons.size()]);
        }
        for (int j = 0; j < 7; j++)
        {
            for (int k = 0; k < rand() % 15; k++)
            {
                timetable->teachers[i].workTime[j].lessonNumbers.push_back(rand() % 8);
            }
        }
    }
    for (int i = 0; i < 4; i++)
    {
        timetable->classes.push_back(Class());
        for (int j = 0; j < 5; j++)
        {
            timetable->classes[i].name += 'a' + rand() % 26;
        }
        timetable->classes[i].teacher = &timetable->teachers[rand() % timetable->teachers.size()];
        for (int j = 0; j < 7; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                timetable->classes[i].days[j].lessons.push_back(TimetableLesson());
                timetable->classes[i].days[j].lessons[k].lessons.push_back(&timetable->lessons[rand() % timetable->lessons.size()]);
                timetable->classes[i].days[j].lessons[k].teachers.push_back(&timetable->teachers[rand() % timetable->teachers.size()]);
            }
        }

    }
    for (int i = 0; i < timetable->lessons.size(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            timetable->lessons[i].classNames.push_back(timetable->classes[rand() % timetable->classes.size()].name);
        }
    }
}
