// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Lessons.hpp"
#include "Logging.hpp"
#include "Translations.hpp"
#include "UI/Lessons/Edit.hpp"
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <string>

std::shared_ptr<LessonsMenu> g_lessonsMenu;

void LessonsMenu::Draw()
{
    if (!ImGui::Begin(gettext("Lessons"), &visible_))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(gettext("+")))
    {
        LogInfo("Adding a new lesson with id %d", timetable_.maxLessonId + 1);
        g_editLessonMenu->Open(&timetable_, true, timetable_.maxLessonId + 1);
    }
    ImGui::Separator();

    ImGui::Columns(3);
    for (auto it = timetable_.lessons.begin(); it != timetable_.lessons.end();)
    {
        ImGui::PushID(it->first);

        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a lesson with id %d", it->first);
            ImGui::PopID();
            it = timetable_.lessons.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a lesson with id %d", it->first);
            g_editLessonMenu->Open(&timetable_, false, it->first);
        }
        ImGui::SameLine();

        ImGui::Text("%s", it->second.name.c_str());
        ImGui::NextColumn();

        std::string classNames;
        for (size_t i = 0; i < it->second.classIds.size(); i++)
        {
            classNames += prevTimetable_->classes[it->second.classIds[i]].number;
            classNames += prevTimetable_->classes[it->second.classIds[i]].letter;
            if (i < it->second.classIds.size() - 1) classNames += ' ';
        }
        ImGui::Text("%s", classNames.c_str());
        ImGui::NextColumn();

        std::string lessonClassrooms;
        for (size_t i = 0; i < it->second.classroomIds.size(); i++)
        {
            lessonClassrooms += prevTimetable_->classrooms[it->second.classroomIds[i]].name;
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
        prevTimetable_->lessons = timetable_.lessons;
        prevTimetable_->maxLessonId = timetable_.maxLessonId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
