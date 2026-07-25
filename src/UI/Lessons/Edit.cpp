// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Lessons/Edit.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "Widgets/Window.hpp"
#include <imgui.h>
#include <memory>
#include <misc/cpp/imgui_stdlib.h>
#include <string>

std::shared_ptr<EditLessonMenu> g_editLessonMenu;

void EditLessonMenu::Open(Timetable* prevTimetable, bool newLesson, int lessonId)
{
    this->prevTimetable_ = prevTimetable;
    this->newLesson_ = newLesson;
    this->lessonId_ = lessonId;

    timetable_ = *prevTimetable;
    if (newLesson) timetable_.lessons[lessonId] = Lesson();

    LogInfo("Resetting EditLessonMenu variables");

    allClasses_ = allClassrooms_ = true;

    classGroups_.clear();
    classes_.clear();
    for (auto& classPair: g_currentTimetable.classes)
    {
        classGroups_[classPair.second.number] = true;
        classes_[classPair.first] = newLesson;
    }

    for (auto& classId: timetable_.lessons[lessonId].classIds) classes_[classId] = true;

    classrooms_.clear();
    for (auto& classroom: g_currentTimetable.classrooms) classrooms_[classroom.first] = newLesson;
    for (auto& classroomId: timetable_.lessons[lessonId].classroomIds)
        classrooms_[classroomId] = true;

    Window::Open();
}

void EditLessonMenu::Draw()
{
    if (!ImGui::Begin((newLesson_ ? gettext("New lesson") : gettext("Edit lesson")), &visible_))
    {
        ImGui::End();
        return;
    }

    // Basic lesson data
    ImGui::InputText(gettext("name"), &timetable_.lessons[lessonId_].name);
    ImGui::Columns(2);
    ImGui::Text("%s", gettext("classes"));

    // Deselect/select all classes
    if (ImGui::Checkbox(
            (std::string(allClasses_ ? gettext("Deselect all") : gettext("Select all")) + "##1")
                .c_str(),
            &allClasses_))
    {
        LogInfo("Clicked allClasses in a lesson with id %d", lessonId_);
        for (auto& classPair: g_currentTimetable.classes)
        {
            classGroups_[classPair.second.number] = allClasses_;
            classes_[classPair.first] = allClasses_;
        }
    }

    // No classes warning
    if (g_currentTimetable.classes.size() == 0)
    {
        ImGui::TextColored(
            COLOR_ERROR, "%s",
            gettext(
                "You need to add classes\nin the Classes menu\nto select classes for this lesson!"));
    }

    // Classes
    std::string lastClassNumber;
    int pushId = 0;
    for (int classId: g_currentTimetable.orderedClasses)
    {
        if (lastClassNumber != g_currentTimetable.classes[classId].number)
        {
            ImGui::PushID(pushId);
            lastClassNumber = g_currentTimetable.classes[classId].number;

            // Class group select
            if (ImGui::Checkbox(g_currentTimetable.classes[classId].number.c_str(),
                                &classGroups_[g_currentTimetable.classes[classId].number]))
            {
                LogInfo("Clicked classGroups in class id %d in lesson with id %d", classId,
                        lessonId_);
                for (auto& classPair: g_currentTimetable.classes)
                {
                    if (classPair.second.number == g_currentTimetable.classes[classId].number)
                        classes_[classPair.first] =
                            classGroups_[g_currentTimetable.classes[classId].number];
                }
            }
            ImGui::PopID();
            pushId++;
        }
        ImGui::PushID(pushId);
        ImGui::Indent();

        // Individual class select
        ImGui::Checkbox((g_currentTimetable.classes[classId].number +
                         g_currentTimetable.classes[classId].letter)
                            .c_str(),
                        &classes_[classId]);
        ImGui::Unindent();
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();

    // Deselect/select all classrooms
    ImGui::Text("classrooms");
    if (ImGui::Checkbox(
            (std::string(allClassrooms_ ? gettext("Deselect all") : gettext("Select all")) + "##2")
                .c_str(),
            &allClassrooms_))
    {
        LogInfo("Clicked allClassrooms in lesson with id %d", lessonId_);
        for (auto& classroom: g_currentTimetable.classrooms)
            classrooms_[classroom.first] = allClassrooms_;
    }

    // No classrooms warning
    if (g_currentTimetable.classrooms.size() == 0)
    {
        ImGui::TextColored(
            COLOR_ERROR, "%s",
            gettext(
                "You need to add classrooms\nin the Classrooms menu\nto select classrooms for this lesson!"));
    }

    // Classrooms
    for (auto& classroom: g_currentTimetable.classrooms)
    {
        ImGui::PushID(pushId);
        ImGui::Checkbox(classroom.second.name.c_str(), &classrooms_[classroom.first]);
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok while editing a lesson with id %d", lessonId_);
        timetable_.lessons[lessonId_].classIds.clear();
        for (auto& classPair: g_currentTimetable.classes)
        {
            if (classes_[classPair.first])
                timetable_.lessons[lessonId_].classIds.push_back(classPair.first);
        }
        timetable_.lessons[lessonId_].classroomIds.clear();
        for (auto& classroom: g_currentTimetable.classrooms)
        {
            if (classrooms_[classroom.first])
                timetable_.lessons[lessonId_].classroomIds.push_back(classroom.first);
        }
        prevTimetable_->lessons = timetable_.lessons;
        prevTimetable_->maxLessonId = timetable_.maxLessonId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
