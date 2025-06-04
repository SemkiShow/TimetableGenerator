#include "Timetable.hpp"
#include "JSON.hpp"

void SaveTimetable(std::string fileName, Timetable* timetable)
{
    JSONObject jsonObject;
    jsonObject.type = JSON_OBJECT;

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
                jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].type = JSON_OBJECT;
                jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].format = JSON_INLINE;
                jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].stringPairs["name"] =
                    timetable->classes[i].days[j].lessons[k].name;
                jsonObject.objectPairs["classes"].objectPairs[timetable->classes[i].name].objectPairs["lessons"].objects[j].objects[k].stringPairs["teacher"] =
                    timetable->classes[i].days[j].lessons[k].teacher->name;
            }
        }
    }

    // Teachers
    jsonObject.objectPairs["teachers"] = JSONObject();
    jsonObject.objectPairs["teachers"].type = JSON_OBJECT;
    for (int i = 0; i < timetable->teachers.size(); i++)
    {
        // Teacher name
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name] = JSONObject();
        jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].type = JSON_LIST;

        // WorkDay
        for (int j = 0; j < 7; j++)
        {
            jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objects.push_back(JSONObject());
            jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objects[j].type = JSON_LIST;
            jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objects[j].format = JSON_INLINE;
            // Lesson number
            for (int k = 0; k < timetable->teachers[i].workTime[j].lessonNumbers.size(); k++)
            {
                jsonObject.objectPairs["teachers"].objectPairs[timetable->teachers[i].name].objects[j].ints.push_back(
                    timetable->teachers[i].workTime[j].lessonNumbers[k]);
            }
        }
    }

    SaveJSON(fileName, &jsonObject);
}

void LoadTimetable(std::string fileName, Timetable* timetable)
{
    JSONObject jsonObject;
    LoadJSON(fileName, &jsonObject);

    int i = 0;
    for (auto teacher: jsonObject.objectPairs["teachers"].objectPairs)
    {
        timetable->teachers.push_back(Teacher());
        timetable->teachers[i].name = teacher.first;
        for (int j = 0; j < teacher.second.objects.size(); j++)
        {
            for (int k = 0; k < teacher.second.objects[j].ints.size(); k++)
            {
                timetable->teachers[i].workTime[j].lessonNumbers.push_back(teacher.second.objects[j].ints[k]);
            }
        }
        i++;
    }

    i = 0;
    for (auto classPair: jsonObject.objectPairs["classes"].objectPairs)
    {
        timetable->classes.push_back(Class());
        timetable->classes[i].name = classPair.first;
        std::string teacherName = classPair.second.stringPairs["teacher"];
        for (int j = 0; j < timetable->teachers.size(); j++)
        {
            if (teacherName == timetable->teachers[j].name)
            {
                timetable->classes[i].teacher = &timetable->teachers[j];
                break;
            }
        }
        for (int j = 0; j < classPair.second.objectPairs["lessons"].objects.size(); j++)
        {
            for (int k = 0; k < classPair.second.objectPairs["lessons"].objects[j].objects.size(); k++)
            {
                timetable->classes[i].days[j].lessons.push_back(Lesson());
                timetable->classes[i].days[j].lessons[k].name = classPair.second.objectPairs["lessons"].objects[j].objects[k].stringPairs["name"];
                teacherName = classPair.second.objectPairs["lessons"].objects[j].objects[k].stringPairs["teacher"];
                for (int l = 0; l < timetable->teachers.size(); l++)
                {
                    if (teacherName == timetable->teachers[l].name)
                    {
                        timetable->classes[i].days[j].lessons[k].teacher = &timetable->teachers[l];
                        break;
                    }
                }
            }
        }
        i++;
    }
}
