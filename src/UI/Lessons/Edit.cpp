// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Lessons/Edit.hpp"
#include "Logging.hpp"
#include "Translations.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

std::shared_ptr<EditLessonMenu> editLessonMenu;

void EditLessonMenu::Open(Timetable* prevTimetable, bool newLesson, int lessonId)
{
    this->prevTimetable = prevTimetable;
    this->newLesson = newLesson;
    this->lessonId = lessonId;

    timetable = *prevTimetable;
    if (newLesson) timetable.lessons[lessonId] = Lesson();

    LogInfo("Resetting EditLessonMenu variables");

    allClasses = allClassrooms = true;

    classGroups.clear();
    classes.clear();
    for (auto& classPair: currentTimetable.classes)
    {
        classGroups[classPair.second.number] = true;
        classes[classPair.first] = newLesson;
    }

    for (auto& classId: timetable.lessons[lessonId].classIds) classes[classId] = true;

    classrooms.clear();
    for (auto& classroom: currentTimetable.classrooms) classrooms[classroom.first] = newLesson;
    for (auto& classroomId: timetable.lessons[lessonId].classroomIds)
        classrooms[classroomId] = true;

    Window::Open();
}

void EditLessonMenu::Draw()
{
    if (!ImGui::Begin((newLesson ? gettext("New lesson") : gettext("Edit lesson")), &visible))
    {
        ImGui::End();
        return;
    }

    // Basic lesson data
    ImGui::InputText(gettext("name"), &timetable.lessons[lessonId].name);
    ImGui::Columns(2);
    ImGui::Text("%s", gettext("classes"));

    // Deselect/select all classes
    if (ImGui::Checkbox(
            (std::string(allClasses ? gettext("Deselect all") : gettext("Select all")) + "##1")
                .c_str(),
            &allClasses))
    {
        LogInfo("Clicked allClasses in a lesson with id " + std::to_string(lessonId));
        for (auto& classPair: currentTimetable.classes)
        {
            classGroups[classPair.second.number] = allClasses;
            classes[classPair.first] = allClasses;
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
                                &classGroups[currentTimetable.classes[classId].number]))
            {
                LogInfo("Clicked classGroups in class Id " + std::to_string(classId) +
                        " in lesson with id " + std::to_string(lessonId));
                for (auto& classPair: currentTimetable.classes)
                {
                    if (classPair.second.number == currentTimetable.classes[classId].number)
                        classes[classPair.first] =
                            classGroups[currentTimetable.classes[classId].number];
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
            &classes[classId]);
        ImGui::Unindent();
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();

    // Deselect/select all classrooms
    ImGui::Text("classrooms");
    if (ImGui::Checkbox(
            (std::string(allClassrooms ? gettext("Deselect all") : gettext("Select all")) + "##2")
                .c_str(),
            &allClassrooms))
    {
        LogInfo("Clicked allClassrooms in lesson with id " + std::to_string(lessonId));
        for (auto& classroom: currentTimetable.classrooms)
            classrooms[classroom.first] = allClassrooms;
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
        ImGui::Checkbox(classroom.second.name.c_str(), &classrooms[classroom.first]);
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok while editing a lesson with id " + std::to_string(lessonId));
        timetable.lessons[lessonId].classIds.clear();
        for (auto& classPair: currentTimetable.classes)
        {
            if (classes[classPair.first])
                timetable.lessons[lessonId].classIds.push_back(classPair.first);
        }
        timetable.lessons[lessonId].classroomIds.clear();
        for (auto& classroom: currentTimetable.classrooms)
        {
            if (classrooms[classroom.first])
                timetable.lessons[lessonId].classroomIds.push_back(classroom.first);
        }
        prevTimetable->lessons = timetable.lessons;
        prevTimetable->maxLessonId = timetable.maxLessonId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
