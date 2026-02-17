// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Lessons.hpp"
#include "Logging.hpp"
#include "Translations.hpp"
#include "UI/Lessons/Edit.hpp"
#include <imgui.h>
#include <string>

std::shared_ptr<LessonsMenu> lessonsMenu;

void LessonsMenu::Draw()
{
    if (!ImGui::Begin(gettext("Lessons"), &visible))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(gettext("+")))
    {
        LogInfo("Adding a new lesson with id " + std::to_string(timetable.maxLessonId + 1));
        editLessonMenu->Open(&timetable, true, timetable.maxLessonId + 1);
    }
    ImGui::Separator();

    ImGui::Columns(3);
    for (auto it = timetable.lessons.begin(); it != timetable.lessons.end();)
    {
        ImGui::PushID(it->first);

        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a lesson with id " + std::to_string(it->first));
            ImGui::PopID();
            it = timetable.lessons.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a lesson with id " + std::to_string(it->first));
            editLessonMenu->Open(&timetable, false, it->first);
        }
        ImGui::SameLine();

        ImGui::Text("%s", it->second.name.c_str());
        ImGui::NextColumn();

        std::string classNames = "";
        for (size_t i = 0; i < it->second.classIds.size(); i++)
        {
            classNames += prevTimetable->classes[it->second.classIds[i]].number;
            classNames += prevTimetable->classes[it->second.classIds[i]].letter;
            if (i < it->second.classIds.size() - 1) classNames += ' ';
        }
        ImGui::Text("%s", classNames.c_str());
        ImGui::NextColumn();

        std::string lessonClassrooms = "";
        for (size_t i = 0; i < it->second.classroomIds.size(); i++)
        {
            lessonClassrooms += prevTimetable->classrooms[it->second.classroomIds[i]].name;
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
        prevTimetable->lessons = timetable.lessons;
        prevTimetable->maxLessonId = timetable.maxLessonId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
