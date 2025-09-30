// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <unordered_map>

static int currentLessonID = 0;
bool newLesson = false;
bool allLessonClasses = true;
std::unordered_map<std::string, bool> lessonClassGroups;
std::unordered_map<int, bool> lessonClasses;
bool allLessonClassrooms = true;
std::unordered_map<int, bool> lessonClassrooms;

static void ResetVariables()
{
    LogInfo("Resetting lesson variables");
    allLessonClasses = allLessonClassrooms = true;
    lessonClassGroups.clear();
    lessonClasses.clear();
    for (auto& classPair: currentTimetable.classes)
    {
        lessonClassGroups[classPair.second.number] = true;
        lessonClasses[classPair.first] = newLesson;
    }
    for (size_t i = 0; i < tmpTmpTimetable.lessons[currentLessonID].classIDs.size(); i++)
        lessonClasses[tmpTmpTimetable.lessons[currentLessonID].classIDs[i]] = true;
    lessonClassrooms.clear();
    for (auto& classroom: currentTimetable.classrooms)
        lessonClassrooms[classroom.first] = newLesson;
    for (size_t i = 0; i < tmpTmpTimetable.lessons[currentLessonID].classroomIDs.size(); i++)
        lessonClassrooms[tmpTmpTimetable.lessons[currentLessonID].classroomIDs[i]] = true;
}

bool isEditLesson = false;
void ShowEditLesson(bool* isOpen)
{
    if (!ImGui::Begin((newLesson ? labels["New lesson"] : labels["Edit lesson"]).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }

    // Basic lesson data
    ImGui::InputText(labels["name"].c_str(), &tmpTmpTimetable.lessons[currentLessonID].name);
    ImGui::Columns(2);
    ImGui::Text("%s", labels["classes"].c_str());

    // Deselect/select all classes
    if (ImGui::Checkbox(
            (allLessonClasses ? labels["Deselect all"] + "##1" : labels["Select all"] + "##1")
                .c_str(),
            &allLessonClasses))
    {
        LogInfo("Clicked allLessonClasses in a lesson with ID " + std::to_string(currentLessonID));
        for (auto& classPair: currentTimetable.classes)
        {
            lessonClassGroups[classPair.second.number] = allLessonClasses;
            lessonClasses[classPair.first] = allLessonClasses;
        }
    }

    // No classes warning
    if (currentTimetable.classes.size() == 0)
    {
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s", labels["You need to add classes"].c_str());
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s", labels["in the Classes menu"].c_str());
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s",
                           labels["to select classes for this lesson!"].c_str());
    }

    // Classes
    std::string lastClassNumber = "";
    int pushID = 0;
    for (size_t classID: currentTimetable.orderedClasses)
    {
        if (lastClassNumber != currentTimetable.classes[classID].number)
        {
            ImGui::PushID(pushID);
            lastClassNumber = currentTimetable.classes[classID].number;

            // Class group select
            if (ImGui::Checkbox(currentTimetable.classes[classID].number.c_str(),
                                &lessonClassGroups[currentTimetable.classes[classID].number]))
            {
                LogInfo("Clicked lessonClassGroups in class ID " + std::to_string(classID) +
                        " in lesson with ID " + std::to_string(currentLessonID));
                for (auto& classPair: currentTimetable.classes)
                {
                    if (classPair.second.number == currentTimetable.classes[classID].number)
                        lessonClasses[classPair.first] =
                            lessonClassGroups[currentTimetable.classes[classID].number];
                }
            }
            ImGui::PopID();
            pushID++;
        }
        ImGui::PushID(pushID);
        ImGui::Indent();

        // Individual class select
        ImGui::Checkbox(
            (currentTimetable.classes[classID].number + currentTimetable.classes[classID].letter)
                .c_str(),
            &lessonClasses[classID]);
        ImGui::Unindent();
        ImGui::PopID();
        pushID++;
    }
    ImGui::NextColumn();

    // Deselect/select all classrooms
    ImGui::Text("classrooms");
    if (ImGui::Checkbox(
            (allLessonClassrooms ? labels["Deselect all"] + "##2" : labels["Select all"] + "##2")
                .c_str(),
            &allLessonClassrooms))
    {
        LogInfo("Clicked allLessonClassrooms in lesson with ID " + std::to_string(currentLessonID));
        for (auto& classroom: currentTimetable.classrooms)
            lessonClassrooms[classroom.first] = allLessonClassrooms;
    }

    // No classrooms warning
    if (currentTimetable.classrooms.size() == 0)
    {
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s",
                           labels["You need to add classrooms"].c_str());
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s", labels["in the Classrooms menu"].c_str());
        ImGui::TextColored(ImVec4(255, 0, 0, 255), "%s",
                           labels["to select classrooms for this lesson!"].c_str());
    }

    // Classrooms
    for (auto& classroom: currentTimetable.classrooms)
    {
        ImGui::PushID(pushID);
        ImGui::Checkbox(classroom.second.name.c_str(), &lessonClassrooms[classroom.first]);
        ImGui::PopID();
        pushID++;
    }
    ImGui::NextColumn();
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Clicked Ok while editing a lesson with ID " + std::to_string(currentLessonID));
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
        tmpTimetable.maxLessonID = tmpTmpTimetable.maxLessonID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str())) *isOpen = false;
    ImGui::End();
}

bool isLessons = false;
void ShowLessons(bool* isOpen)
{
    if (!ImGui::Begin(labels["Lessons"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(labels["+"].c_str()))
    {
        newLesson = true;
        tmpTmpTimetable.lessons = tmpTimetable.lessons;
        tmpTmpTimetable.maxLessonID = tmpTimetable.maxLessonID;
        tmpTmpTimetable.maxLessonID++;
        tmpTmpTimetable.lessons[tmpTmpTimetable.maxLessonID] = Lesson();
        currentLessonID = tmpTmpTimetable.maxLessonID;
        LogInfo("Adding a new lesson with ID " + std::to_string(currentLessonID));
        ResetVariables();
        isEditLesson = true;
    }
    ImGui::Separator();

    ImGui::Columns(3);
    for (auto it = tmpTimetable.lessons.begin(); it != tmpTimetable.lessons.end();)
    {
        ImGui::PushID(it->first);

        if (ImGui::Button(labels["-"].c_str()))
        {
            LogInfo("Removed a lesson with ID " + std::to_string(it->first));
            ImGui::PopID();
            it = tmpTimetable.lessons.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(labels["Edit"].c_str()))
        {
            newLesson = false;
            tmpTmpTimetable.lessons = tmpTimetable.lessons;
            tmpTmpTimetable.maxLessonID = tmpTimetable.maxLessonID;
            currentLessonID = it->first;
            LogInfo("Editing a lesson with ID " + std::to_string(currentLessonID));
            ResetVariables();
            isEditLesson = true;
        }
        ImGui::SameLine();

        ImGui::Text("%s", it->second.name.c_str());
        ImGui::NextColumn();

        std::string classNames = "";
        for (size_t i = 0; i < it->second.classIDs.size(); i++)
        {
            classNames += currentTimetable.classes[it->second.classIDs[i]].number;
            classNames += currentTimetable.classes[it->second.classIDs[i]].letter;
            if (i < it->second.classIDs.size() - 1) classNames += ' ';
        }
        ImGui::Text("%s", classNames.c_str());
        ImGui::NextColumn();

        std::string lessonClassrooms = "";
        for (size_t i = 0; i < it->second.classroomIDs.size(); i++)
        {
            lessonClassrooms += currentTimetable.classrooms[it->second.classroomIDs[i]].name;
            if (i < it->second.classroomIDs.size() - 1) lessonClassrooms += ' ';
        }
        ImGui::Text("%s", lessonClassrooms.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
        ++it;
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Clicked Ok in the lessons menu");
        currentTimetable.lessons = tmpTimetable.lessons;
        currentTimetable.maxLessonID = tmpTimetable.maxLessonID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str())) *isOpen = false;
    ImGui::End();
}
