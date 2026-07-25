// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Teachers/Edit.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "Widgets/Window.hpp"
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <misc/cpp/imgui_stdlib.h>
#include <string>

std::shared_ptr<EditTeacherMenu> g_editTeacherMenu;

void EditTeacherMenu::ResetTeacherLessonValues()
{
    lessonValues_ = "";
    lessonValues_ += gettext("no lesson");
    lessonValues_ += '\0';
    lessonValues_ += gettext("any lesson");
    lessonValues_ += '\0';
    for (auto& lesson: g_currentTimetable.lessons)
    {
        if (!lessons_[lesson.first]) continue;
        if (lesson.second.name == "")
            lessonValues_ += gettext("error");
        else
            lessonValues_ += lesson.second.name;
        lessonValues_ += '\0';
    }
    lessonValues_ += '\0';
}

void EditTeacherMenu::Open(Timetable* prevTimetable, bool newTeacher, int teacherId)
{
    this->prevTimetable_ = prevTimetable;
    this->newTeacher_ = newTeacher;
    this->teacherId_ = teacherId;

    timetable_ = *prevTimetable;
    if (newTeacher) timetable_.teachers[teacherId] = Teacher();

    LogInfo("Resetting teacher variables");

    allLessons_ = false;

    allAvailableLessonsVertical_.clear();
    allAvailableLessonsVertical_.resize(g_settings.daysPerWeek, 1);

    allAvailableLessonsHorizontal_.clear();
    allAvailableLessonsHorizontal_.resize(g_settings.lessonsPerDay, 1);

    lessons_.clear();
    for (auto& lesson: g_currentTimetable.lessons) lessons_[lesson.first] = false;
    for (auto& lessonId: timetable_.teachers[teacherId].lessonIds) lessons_[lessonId] = true;

    availableLessons_.clear();
    for (int i = 0; i < g_settings.daysPerWeek; i++)
    {
        for (int j = 0; j < g_settings.lessonsPerDay; j++)
            availableLessons_[i * g_settings.lessonsPerDay + j] = 1;
    }

    timetable_.teachers[teacherId].workDays.resize(g_settings.daysPerWeek);
    for (int i = 0; i < g_settings.daysPerWeek; i++)
    {
        for (size_t j = 0; j < timetable_.teachers[teacherId].workDays[i].lessonIds.size(); j++)
        {
            int lessonId = timetable_.teachers[teacherId].workDays[i].lessonIds[j];
            if (lessonId == ANY_LESSON || lessonId == NO_LESSON)
                availableLessons_[i * g_settings.lessonsPerDay + (int)j] = lessonId + 3;
            else
            {
                int counter = 2;
                for (auto& lesson: g_currentTimetable.lessons)
                {
                    if (lessonId == lesson.first)
                    {
                        availableLessons_[i * g_settings.lessonsPerDay + (int)j] = counter;
                        break;
                    }
                    counter++;
                }
            }
        }
    }
    if (newTeacher)
    {
        timetable_.teachers[teacherId].workDays.resize(g_settings.daysPerWeek);
        for (int i = 0; i < g_settings.daysPerWeek; i++)
        {
            timetable_.teachers[teacherId].workDays[i].lessonIds.resize(g_settings.lessonsPerDay);
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
                timetable_.teachers[teacherId].workDays[i].lessonIds.push_back(1);
        }
    }

    ResetTeacherLessonValues();

    Window::Open();
}

void EditTeacherMenu::Draw()
{
    if (!ImGui::Begin((newTeacher_ ? gettext("New teacher") : gettext("Edit teacher")), &visible_))
    {
        ImGui::End();
        return;
    }

    ImGui::InputText(gettext("name"), &timetable_.teachers[teacherId_].name);
    ImGui::Separator();
    ImGui::Text("%s", gettext("lessons"));

    // Deselect/select all lessons
    if (ImGui::Checkbox(
            (std::string(allLessons_ ? gettext("Deselect all") : gettext("Select all")) + "##1")
                .c_str(),
            &allLessons_))
    {
        LogInfo("Clicked allLessons in a teacher with id %d", teacherId_);
        for (auto& lesson: g_currentTimetable.lessons) lessons_[lesson.first] = allLessons_;
    }

    // No lessons warning
    if (g_currentTimetable.lessons.size() == 0)
    {
        ImGui::TextColored(
            COLOR_ERROR, "%s",
            gettext(
                "You need to add lessons\nin the Lessons menu\nto select lessons for this teacher!"));
    }

    // Lesons
    ImGui::Columns(3);
    int pushId = 0;
    for (auto& lesson: g_currentTimetable.lessons)
    {
        ImGui::PushID(pushId);
        if (ImGui::Checkbox(lesson.second.name.c_str(), &lessons_[lesson.first]))
        {
            ResetTeacherLessonValues();
        }
        ImGui::NextColumn();

        std::string classNames;
        for (size_t j = 0; j < lesson.second.classIds.size(); j++)
        {
            classNames += g_currentTimetable.classes[lesson.second.classIds[j]].number;
            classNames += g_currentTimetable.classes[lesson.second.classIds[j]].letter;
            if (j < lesson.second.classIds.size() - 1) classNames += ' ';
        }
        ImGui::Text("%s", classNames.c_str());
        ImGui::NextColumn();

        std::string lessonClassrooms;
        for (size_t j = 0; j < lesson.second.classroomIds.size(); j++)
        {
            lessonClassrooms += g_currentTimetable.classrooms[lesson.second.classroomIds[j]].name;
            if (j < lesson.second.classroomIds.size() - 1) lessonClassrooms += ' ';
        }
        ImGui::Text("%s", lessonClassrooms.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
        pushId++;
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Available lessons
    ImGui::Text("%s", gettext("available lessons"));
    ImGui::Columns(g_settings.daysPerWeek + 1);
    ImGui::LabelText("##1", "%s", "");
    ImGui::LabelText("##2", "%s", "");
    allAvailableLessonsHorizontal_.resize(g_settings.lessonsPerDay, 1);
    for (int i = 0; i < g_settings.lessonsPerDay; i++)
    {
        ImGui::PushID(pushId);
        if (ImGui::Combo(std::to_string(i).c_str(), &allAvailableLessonsHorizontal_[i],
                         lessonValues_.c_str()))
        {
            LogInfo("Clicked allAvailableLessonsHorizontal in a teacher with id %d", teacherId_);
            for (int j = 0; j < g_settings.daysPerWeek; j++)
            {
                availableLessons_[j * g_settings.lessonsPerDay + i] =
                    allAvailableLessonsHorizontal_[i];
            }
        }
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();
    allAvailableLessonsVertical_.resize(g_settings.daysPerWeek, 1);
    for (int i = 0; i < g_settings.daysPerWeek; i++)
    {
        size_t weekDay = i;
        while (weekDay >= g_weekDays.size()) weekDay -= g_weekDays.size();
        ImGui::Text("%s", g_weekDays[weekDay]);
        ImGui::PushID(pushId);
        if (ImGui::Combo("", &allAvailableLessonsVertical_[i], lessonValues_.c_str()))
        {
            LogInfo("Clicked allAvailableLessonsVertical in a teacher with id %d", teacherId_);
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
                availableLessons_[i * g_settings.lessonsPerDay + j] =
                    allAvailableLessonsVertical_[i];
        }
        ImGui::PopID();
        pushId++;
        for (int j = 0; j < g_settings.lessonsPerDay; j++)
        {
            ImGui::PushID(pushId);
            ImGui::Combo("", &availableLessons_[i * g_settings.lessonsPerDay + j],
                         lessonValues_.c_str());
            ImGui::PopID();
            pushId++;
        }
        ImGui::NextColumn();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok while editing a teacher with id %d", teacherId_);
        timetable_.teachers[teacherId_].lessonIds.clear();
        for (auto& lesson: g_currentTimetable.lessons)
        {
            if (lessons_[lesson.first])
                timetable_.teachers[teacherId_].lessonIds.push_back(lesson.first);
        }
        timetable_.teachers[teacherId_].workDays.resize(g_settings.daysPerWeek);
        for (int i = 0; i < g_settings.daysPerWeek; i++)
        {
            timetable_.teachers[teacherId_].workDays[i].lessonIds.clear();
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
            {
                if (availableLessons_[i * g_settings.lessonsPerDay + j] == 0 ||
                    availableLessons_[i * g_settings.lessonsPerDay + j] == 1)
                    timetable_.teachers[teacherId_].workDays[i].lessonIds.push_back(
                        availableLessons_[i * g_settings.lessonsPerDay + j] - 3);
                else
                {
                    availableLessons_[i * g_settings.lessonsPerDay + j] -= 2;
                    for (auto& lesson: g_currentTimetable.lessons)
                    {
                        if (availableLessons_[i * g_settings.lessonsPerDay + j] <= 0)
                        {
                            timetable_.teachers[teacherId_].workDays[i].lessonIds.push_back(
                                lesson.first);
                            break;
                        }
                        availableLessons_[i * g_settings.lessonsPerDay + j]--;
                    }
                }
            }
        }
        prevTimetable_->teachers = timetable_.teachers;
        prevTimetable_->maxTeacherId = timetable_.maxTeacherId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
