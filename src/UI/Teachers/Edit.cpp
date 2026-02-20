// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Teachers/Edit.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

std::shared_ptr<EditTeacherMenu> editTeacherMenu;

void EditTeacherMenu::ResetTeacherLessonValues()
{
    lessonValues = "";
    lessonValues += gettext("no lesson");
    lessonValues += '\0';
    lessonValues += gettext("any lesson");
    lessonValues += '\0';
    for (auto& lesson: currentTimetable.lessons)
    {
        if (!lessons[lesson.first]) continue;
        if (lesson.second.name == "")
            lessonValues += gettext("error");
        else
            lessonValues += lesson.second.name;
        lessonValues += '\0';
    }
    lessonValues += '\0';
}

void EditTeacherMenu::Open(Timetable* prevTimetable, bool newTeacher, int teacherId)
{
    this->prevTimetable = prevTimetable;
    this->newTeacher = newTeacher;
    this->teacherId = teacherId;

    timetable = *prevTimetable;
    if (newTeacher) timetable.teachers[teacherId] = Teacher();

    LogInfo("Resetting teacher variables");

    allLessons = false;

    allAvailableLessonsVertical.clear();
    allAvailableLessonsVertical.resize(settings.daysPerWeek, 1);

    allAvailableLessonsHorizontal.clear();
    allAvailableLessonsHorizontal.resize(settings.lessonsPerDay, 1);

    lessons.clear();
    for (auto& lesson: currentTimetable.lessons) lessons[lesson.first] = false;
    for (auto& lessonId: timetable.teachers[teacherId].lessonIds) lessons[lessonId] = true;

    availableLessons.clear();
    for (size_t i = 0; i < settings.daysPerWeek; i++)
    {
        for (size_t j = 0; j < settings.lessonsPerDay; j++)
            availableLessons[i * settings.lessonsPerDay + j] = 1;
    }

    timetable.teachers[teacherId].workDays.resize(settings.daysPerWeek);
    for (size_t i = 0; i < settings.daysPerWeek; i++)
    {
        for (size_t j = 0; j < timetable.teachers[teacherId].workDays[i].lessonIds.size(); j++)
        {
            int lessonId = timetable.teachers[teacherId].workDays[i].lessonIds[j];
            if (lessonId == ANY_LESSON || lessonId == NO_LESSON)
                availableLessons[i * settings.lessonsPerDay + j] = lessonId + 3;
            else
            {
                int counter = 2;
                for (auto& lesson: currentTimetable.lessons)
                {
                    if (lessonId == lesson.first)
                    {
                        availableLessons[i * settings.lessonsPerDay + j] = counter;
                        break;
                    }
                    counter++;
                }
            }
        }
    }
    if (newTeacher)
    {
        timetable.teachers[teacherId].workDays.resize(settings.daysPerWeek);
        for (size_t i = 0; i < settings.daysPerWeek; i++)
        {
            timetable.teachers[teacherId].workDays[i].lessonIds.resize(settings.lessonsPerDay);
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
                timetable.teachers[teacherId].workDays[i].lessonIds.push_back(1);
        }
    }

    ResetTeacherLessonValues();

    Window::Open();
}

void EditTeacherMenu::Draw()
{
    if (!ImGui::Begin((newTeacher ? gettext("New teacher") : gettext("Edit teacher")), &visible))
    {
        ImGui::End();
        return;
    }

    ImGui::InputText(gettext("name"), &timetable.teachers[teacherId].name);
    ImGui::Separator();
    ImGui::Text("%s", gettext("lessons"));

    // Deselect/select all lessons
    if (ImGui::Checkbox(
            (std::string(allLessons ? gettext("Deselect all") : gettext("Select all")) + "##1")
                .c_str(),
            &allLessons))
    {
        LogInfo("Clicked allLessons in a teacher with id " + std::to_string(teacherId));
        for (auto& lesson: currentTimetable.lessons) lessons[lesson.first] = allLessons;
    }

    // No lessons warning
    if (currentTimetable.lessons.size() == 0)
    {
        ImGui::TextColored(
            ImVec4(255, 0, 0, 255), "%s",
            gettext(
                "You need to add lessons\nin the Lessons menu\nto select lessons for this teacher!"));
    }

    // Lesons
    ImGui::Columns(3);
    int pushId = 0;
    for (auto& lesson: currentTimetable.lessons)
    {
        ImGui::PushID(pushId);
        if (ImGui::Checkbox(lesson.second.name.c_str(), &lessons[lesson.first]))
        {
            ResetTeacherLessonValues();
        }
        ImGui::NextColumn();
        std::string classNames = "";
        for (size_t j = 0; j < lesson.second.classIds.size(); j++)
        {
            classNames += currentTimetable.classes[lesson.second.classIds[j]].number;
            classNames += currentTimetable.classes[lesson.second.classIds[j]].letter;
            if (j < lesson.second.classIds.size() - 1) classNames += ' ';
        }
        ImGui::Text("%s", classNames.c_str());
        ImGui::NextColumn();
        std::string lessonClassrooms = "";
        for (size_t j = 0; j < lesson.second.classroomIds.size(); j++)
        {
            lessonClassrooms += currentTimetable.classrooms[lesson.second.classroomIds[j]].name;
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
    ImGui::Columns(settings.daysPerWeek + 1);
    ImGui::LabelText("##1", "%s", "");
    ImGui::LabelText("##2", "%s", "");
    allAvailableLessonsHorizontal.resize(settings.lessonsPerDay, 1);
    for (size_t i = 0; i < settings.lessonsPerDay; i++)
    {
        ImGui::PushID(pushId);
        if (ImGui::Combo(std::to_string(i).c_str(), &allAvailableLessonsHorizontal[i],
                         lessonValues.c_str()))
        {
            LogInfo("Clicked allAvailableLessonsHorizontal in a teacher with id " +
                    std::to_string(teacherId));
            for (size_t j = 0; j < settings.daysPerWeek; j++)
            {
                availableLessons[j * settings.lessonsPerDay + i] = allAvailableLessonsHorizontal[i];
            }
        }
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();
    allAvailableLessonsVertical.resize(settings.daysPerWeek, 1);
    for (size_t i = 0; i < settings.daysPerWeek; i++)
    {
        int weekDay = i;
        while (weekDay >= 7) weekDay -= 7;
        ImGui::Text("%s", weekDays[weekDay].c_str());
        ImGui::PushID(pushId);
        if (ImGui::Combo("", &allAvailableLessonsVertical[i], lessonValues.c_str()))
        {
            LogInfo("Clicked allAvailableLessonsVertical in a teacher with id " +
                    std::to_string(teacherId));
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
                availableLessons[i * settings.lessonsPerDay + j] = allAvailableLessonsVertical[i];
        }
        ImGui::PopID();
        pushId++;
        for (size_t j = 0; j < settings.lessonsPerDay; j++)
        {
            ImGui::PushID(pushId);
            ImGui::Combo("", &availableLessons[i * settings.lessonsPerDay + j],
                         lessonValues.c_str());
            ImGui::PopID();
            pushId++;
        }
        ImGui::NextColumn();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok while editing a teacher with id " + std::to_string(teacherId));
        timetable.teachers[teacherId].lessonIds.clear();
        for (auto& lesson: currentTimetable.lessons)
        {
            if (lessons[lesson.first])
                timetable.teachers[teacherId].lessonIds.push_back(lesson.first);
        }
        timetable.teachers[teacherId].workDays.resize(settings.daysPerWeek);
        for (size_t i = 0; i < settings.daysPerWeek; i++)
        {
            timetable.teachers[teacherId].workDays[i].lessonIds.clear();
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
            {
                if (availableLessons[i * settings.lessonsPerDay + j] == 0 ||
                    availableLessons[i * settings.lessonsPerDay + j] == 1)
                    timetable.teachers[teacherId].workDays[i].lessonIds.push_back(
                        availableLessons[i * settings.lessonsPerDay + j] - 3);
                else
                {
                    availableLessons[i * settings.lessonsPerDay + j] -= 2;
                    for (auto& lesson: currentTimetable.lessons)
                    {
                        if (availableLessons[i * settings.lessonsPerDay + j] <= 0)
                        {
                            timetable.teachers[teacherId].workDays[i].lessonIds.push_back(
                                lesson.first);
                            break;
                        }
                        availableLessons[i * settings.lessonsPerDay + j]--;
                    }
                }
            }
        }
        prevTimetable->teachers = timetable.teachers;
        prevTimetable->maxTeacherId = timetable.maxTeacherId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
