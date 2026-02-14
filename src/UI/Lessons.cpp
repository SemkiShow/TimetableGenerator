// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <unordered_map>

static int currentLessonId = 0;
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
    for (size_t i = 0; i < tmpTmpTimetable.lessons[currentLessonId].classIds.size(); i++)
        lessonClasses[tmpTmpTimetable.lessons[currentLessonId].classIds[i]] = true;
    lessonClassrooms.clear();
    for (auto& classroom: currentTimetable.classrooms)
        lessonClassrooms[classroom.first] = newLesson;
    for (size_t i = 0; i < tmpTmpTimetable.lessons[currentLessonId].classroomIds.size(); i++)
        lessonClassrooms[tmpTmpTimetable.lessons[currentLessonId].classroomIds[i]] = true;
}

bool isEditLesson = false;
void ShowEditLesson(bool* isOpen)
{
    if (!ImGui::Begin((newLesson ? gettext("New lesson") : gettext("Edit lesson")), isOpen))
    {
        ImGui::End();
        return;
    }

    // Basic lesson data
    ImGui::InputText(gettext("name"), &tmpTmpTimetable.lessons[currentLessonId].name);
    ImGui::Columns(2);
    ImGui::Text("%s", gettext("classes"));

    // Deselect/select all classes
    if (ImGui::Checkbox(
            (std::string(allLessonClasses ? gettext("Deselect all") : gettext("Select all")) +
             "##1")
                .c_str(),
            &allLessonClasses))
    {
        LogInfo("Clicked allLessonClasses in a lesson with Id " + std::to_string(currentLessonId));
        for (auto& classPair: currentTimetable.classes)
        {
            lessonClassGroups[classPair.second.number] = allLessonClasses;
            lessonClasses[classPair.first] = allLessonClasses;
        }
    }

    // No classes warning
    if (currentTimetable.classes.size() == 0)
    {
        ImGui::TextColored(
            ImVec4(255, 0, 0, 255), "%s",
            gettext(
                "You need to add classes\nin the Classes menu\nto select classes for this lesson!"));
    }

    // Classes
    std::string lastClassNumber = "";
    int pushId = 0;
    for (size_t classId: currentTimetable.orderedClasses)
    {
        if (lastClassNumber != currentTimetable.classes[classId].number)
        {
            ImGui::PushID(pushId);
            lastClassNumber = currentTimetable.classes[classId].number;

            // Class group select
            if (ImGui::Checkbox(currentTimetable.classes[classId].number.c_str(),
                                &lessonClassGroups[currentTimetable.classes[classId].number]))
            {
                LogInfo("Clicked lessonClassGroups in class Id " + std::to_string(classId) +
                        " in lesson with Id " + std::to_string(currentLessonId));
                for (auto& classPair: currentTimetable.classes)
                {
                    if (classPair.second.number == currentTimetable.classes[classId].number)
                        lessonClasses[classPair.first] =
                            lessonClassGroups[currentTimetable.classes[classId].number];
                }
            }
            ImGui::PopID();
            pushId++;
        }
        ImGui::PushID(pushId);
        ImGui::Indent();

        // Individual class select
        ImGui::Checkbox(
            (currentTimetable.classes[classId].number + currentTimetable.classes[classId].letter)
                .c_str(),
            &lessonClasses[classId]);
        ImGui::Unindent();
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();

    // Deselect/select all classrooms
    ImGui::Text("classrooms");
    if (ImGui::Checkbox(
            (std::string(allLessonClassrooms ? gettext("Deselect all") : gettext("Select all")) +
             "##2")
                .c_str(),
            &allLessonClassrooms))
    {
        LogInfo("Clicked allLessonClassrooms in lesson with Id " + std::to_string(currentLessonId));
        for (auto& classroom: currentTimetable.classrooms)
            lessonClassrooms[classroom.first] = allLessonClassrooms;
    }

    // No classrooms warning
    if (currentTimetable.classrooms.size() == 0)
    {
        ImGui::TextColored(
            ImVec4(255, 0, 0, 255), "%s",
            gettext(
                "You need to add classrooms\nin the Classrooms menu\nto select classrooms for this lesson!"));
    }

    // Classrooms
    for (auto& classroom: currentTimetable.classrooms)
    {
        ImGui::PushID(pushId);
        ImGui::Checkbox(classroom.second.name.c_str(), &lessonClassrooms[classroom.first]);
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok while editing a lesson with Id " + std::to_string(currentLessonId));
        tmpTmpTimetable.lessons[currentLessonId].classIds.clear();
        for (auto& classPair: currentTimetable.classes)
        {
            if (lessonClasses[classPair.first])
                tmpTmpTimetable.lessons[currentLessonId].classIds.push_back(classPair.first);
        }
        tmpTmpTimetable.lessons[currentLessonId].classroomIds.clear();
        for (auto& classroom: currentTimetable.classrooms)
        {
            if (lessonClassrooms[classroom.first])
                tmpTmpTimetable.lessons[currentLessonId].classroomIds.push_back(classroom.first);
        }
        tmpTimetable.lessons = tmpTmpTimetable.lessons;
        tmpTimetable.maxLessonId = tmpTmpTimetable.maxLessonId;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}

bool isLessons = false;
void ShowLessons(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Lessons"), isOpen))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(gettext("+")))
    {
        newLesson = true;
        tmpTmpTimetable.lessons = tmpTimetable.lessons;
        tmpTmpTimetable.maxLessonId = tmpTimetable.maxLessonId;
        tmpTmpTimetable.maxLessonId++;
        tmpTmpTimetable.lessons[tmpTmpTimetable.maxLessonId] = Lesson();
        currentLessonId = tmpTmpTimetable.maxLessonId;
        LogInfo("Adding a new lesson with Id " + std::to_string(currentLessonId));
        ResetVariables();
        isEditLesson = true;
    }
    ImGui::Separator();

    ImGui::Columns(3);
    for (auto it = tmpTimetable.lessons.begin(); it != tmpTimetable.lessons.end();)
    {
        ImGui::PushID(it->first);

        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a lesson with Id " + std::to_string(it->first));
            ImGui::PopID();
            it = tmpTimetable.lessons.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            newLesson = false;
            tmpTmpTimetable.lessons = tmpTimetable.lessons;
            tmpTmpTimetable.maxLessonId = tmpTimetable.maxLessonId;
            currentLessonId = it->first;
            LogInfo("Editing a lesson with Id " + std::to_string(currentLessonId));
            ResetVariables();
            isEditLesson = true;
        }
        ImGui::SameLine();

        ImGui::Text("%s", it->second.name.c_str());
        ImGui::NextColumn();

        std::string classNames = "";
        for (size_t i = 0; i < it->second.classIds.size(); i++)
        {
            classNames += currentTimetable.classes[it->second.classIds[i]].number;
            classNames += currentTimetable.classes[it->second.classIds[i]].letter;
            if (i < it->second.classIds.size() - 1) classNames += ' ';
        }
        ImGui::Text("%s", classNames.c_str());
        ImGui::NextColumn();

        std::string lessonClassrooms = "";
        for (size_t i = 0; i < it->second.classroomIds.size(); i++)
        {
            lessonClassrooms += currentTimetable.classrooms[it->second.classroomIds[i]].name;
            if (i < it->second.classroomIds.size() - 1) lessonClassrooms += ' ';
        }
        ImGui::Text("%s", lessonClassrooms.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
        ++it;
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok in the lessons menu");
        currentTimetable.lessons = tmpTimetable.lessons;
        currentTimetable.maxLessonId = tmpTimetable.maxLessonId;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}
