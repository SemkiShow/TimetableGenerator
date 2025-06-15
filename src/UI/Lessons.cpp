#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

static int currentLessonID = 0;
bool newLesson = false;
bool allLessonClasses = true;
std::unordered_map<std::string, bool> lessonClassGroups;
std::unordered_map<int, bool> lessonClasses;
bool allLessonClassrooms = true;
std::unordered_map<int, bool> lessonClassrooms;

static void ResetVariables()
{
    allLessonClasses = allLessonClassrooms = true;
    lessonClassGroups.clear();
    lessonClasses.clear();
    for (auto& classPair: currentTimetable.classes)
    {
        lessonClassGroups[classPair.second.number] = true;
        lessonClasses[classPair.first] = newLesson;
    }
    for (int i = 0; i < tmpTimetable.lessons[currentLessonID].classIDs.size(); i++)
        lessonClasses[tmpTimetable.lessons[currentLessonID].classIDs[i]] = true;
    lessonClassrooms.clear();
    for (auto& classroom: currentTimetable.classrooms)
        lessonClassrooms[classroom.first] = newLesson;
    for (int i = 0; i < tmpTimetable.lessons[currentLessonID].classroomIDs.size(); i++)
        lessonClassrooms[tmpTimetable.lessons[currentLessonID].classroomIDs[i]] = true;
}

bool isEditLesson = false;
void ShowEditLesson(bool* isOpen)
{
    if (!ImGui::Begin(((newLesson ? "New" : "Edit") + std::string(" Lesson")).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputText("name", &tmpTmpTimetable.lessons[currentLessonID].name);
    ImGui::Columns(2);
    ImGui::Text("classes");
    if (ImGui::Checkbox((allLessonClasses ? "Deselect all##1" : "Select all##1"), &allLessonClasses))
    {
        for (auto& classPair: currentTimetable.classes)
        {
            lessonClassGroups[classPair.second.number] = allLessonClasses;
            lessonClasses[classPair.first] = allLessonClasses;
        }
    }
    if (currentTimetable.classes.size() == 0) ImGui::TextColored(ImVec4(255, 0, 0, 255), "You need to add classes\nin the Classes menu\nto select classes for this lesson!");
    std::string lastClassNumber = "";
    for (int classID: currentTimetable.orderedClasses)
    {
        if (lastClassNumber != currentTimetable.classes[classID].number)
        {
            lastClassNumber = currentTimetable.classes[classID].number;
            if (ImGui::Checkbox(currentTimetable.classes[classID].number.c_str(), &lessonClassGroups[currentTimetable.classes[classID].number]))
            {
                for (auto& classPair: currentTimetable.classes)
                {
                    if (classPair.second.number == currentTimetable.classes[classID].number)
                        lessonClasses[classPair.first] = lessonClassGroups[currentTimetable.classes[classID].number];
                }
            }
        }
        ImGui::Indent();
        ImGui::Checkbox((currentTimetable.classes[classID].number + currentTimetable.classes[classID].letter).c_str(), &lessonClasses[classID]);
        ImGui::Unindent();
    }
    ImGui::NextColumn();
    ImGui::Text("classrooms");
    if (ImGui::Checkbox((allLessonClassrooms ? "Deselect all##2" : "Select all##2"), &allLessonClassrooms))
    {
        for (auto& classroom: currentTimetable.classrooms)
            lessonClassrooms[classroom.first] = allLessonClassrooms;
    }
    if (currentTimetable.classrooms.size() == 0) ImGui::TextColored(ImVec4(255, 0, 0, 255), "You need to add classrooms\nin the Classrooms menu\nto select classrooms for this lesson!");
    for (auto& classroom: tmpTmpTimetable.classrooms)
        ImGui::Checkbox(classroom.second.name.c_str(), &lessonClassrooms[classroom.first]);
    ImGui::NextColumn();
    ImGui::Columns(1);
    if (ImGui::Button("Ok"))
    {
        tmpTmpTimetable.lessons[currentLessonID].classIDs.clear();
        for (auto& classPair: currentTimetable.classes)
        {
            if (lessonClasses[classPair.first])
                tmpTmpTimetable.lessons[currentLessonID].classIDs.push_back(classPair.first);
        }
        tmpTmpTimetable.lessons[currentLessonID].classroomIDs.clear();
        for (auto& classroom: currentTimetable.classrooms)
        {
            if (lessonClassrooms[classroom.first])
                tmpTmpTimetable.lessons[currentLessonID].classroomIDs.push_back(classroom.first);
        }
        tmpTimetable.lessons = tmpTmpTimetable.lessons;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        if (newLesson) tmpTimetable.lessons.erase(currentLessonID);
        *isOpen = false;
    }
    ImGui::End();
}

bool isLessons = false;
void ShowLessons(bool* isOpen)
{
    if (!ImGui::Begin("Lessons", isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::Button("+"))
    {
        tmpTimetable.maxLessonID++;
        tmpTimetable.lessons[tmpTimetable.maxLessonID] = Lesson();
        currentLessonID = tmpTimetable.maxLessonID;
        newLesson = true;
        ResetVariables();
        tmpTmpTimetable.lessons = tmpTimetable.lessons;
        isEditLesson = true;
    }
    ImGui::Separator();
    ImGui::Columns(3);
    for (auto it = tmpTimetable.lessons.begin(); it != tmpTimetable.lessons.end();)
    {
        if (it->first == 0 || it->first == 1)
        {
            it++;
            continue;
        }
        ImGui::PushID(it->first);
        if (ImGui::Button("-"))
        {
            ImGui::PopID();
            it = tmpTimetable.lessons.erase(it);
            continue;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            currentLessonID = it->first;
            newLesson = false;
            ResetVariables();
            tmpTmpTimetable.lessons = tmpTimetable.lessons;
            isEditLesson = true;
        }
        ImGui::SameLine();
        ImGui::Text(it->second.name.c_str());
        ImGui::NextColumn();
        std::string classNames = "";
        for (int i = 0; i < it->second.classIDs.size(); i++)
        {
            classNames += currentTimetable.classes[it->second.classIDs[i]].number;
            classNames += currentTimetable.classes[it->second.classIDs[i]].letter;
            if (i < it->second.classIDs.size()-1) classNames += ' ';
        }
        ImGui::Text(classNames.c_str());
        ImGui::NextColumn();
        std::string lessonClassrooms = "";
        for (int i = 0; i < it->second.classroomIDs.size(); i++)
        {
            lessonClassrooms += currentTimetable.classrooms[it->second.classroomIDs[i]].name;
            if (i < it->second.classroomIDs.size()-1) lessonClassrooms += ' ';
        }
        ImGui::Text(lessonClassrooms.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
        it++;
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (ImGui::Button("Ok"))
    {
        currentTimetable.lessons = tmpTimetable.lessons;
        currentTimetable.maxLessonID = tmpTimetable.maxLessonID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}
