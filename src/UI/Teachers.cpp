// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Teachers.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI/Teachers/Edit.hpp"
#include <imgui.h>
#include <string>

std::shared_ptr<TeachersMenu> teachersMenu;

void TeachersMenu::Draw()
{
    if (!ImGui::Begin(gettext("Teachers"), &visible))
    {
        ImGui::End();
        return;
    }

    ImGui::InputInt(gettext("min free periods"), &settings.minFreePeriods);
    if (settings.minFreePeriods < 0) settings.minFreePeriods = 0;
    ImGui::InputInt(gettext("max free periods"), &settings.maxFreePeriods);
    if (settings.maxFreePeriods < 0) settings.maxFreePeriods = 0;
    if (settings.maxFreePeriods < settings.minFreePeriods)
        settings.maxFreePeriods = settings.minFreePeriods;

    if (ImGui::Button(gettext("+")))
    {
        LogInfo("Adding a new teacher with id " + std::to_string(timetable.maxTeacherId + 1));
        editTeacherMenu->Open(&timetable, true, timetable.maxTeacherId + 1);
    }
    ImGui::Separator();

    ImGui::Columns(2);
    for (auto it = timetable.teachers.begin(); it != timetable.teachers.end();)
    {
        ImGui::PushID(it->first);
        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a teacher with id " + std::to_string(it->first));
            ImGui::PopID();
            it = timetable.teachers.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a teacher with id " + std::to_string(it->first));
            editTeacherMenu->Open(&timetable, false, it->first);
        }
        ImGui::SameLine();

        ImGui::Text("%s", it->second.name.c_str());
        ImGui::NextColumn();

        std::string lessonNames = "";
        for (size_t j = 0; j < timetable.teachers[it->first].lessonIds.size(); j++)
        {
            lessonNames += prevTimetable->lessons[timetable.teachers[it->first].lessonIds[j]].name;
            if (j < timetable.teachers[it->first].lessonIds.size() - 1) lessonNames += ' ';
        }
        ImGui::Text("%s", lessonNames.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
        ++it;
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok in the teachers menu");
        prevTimetable->teachers = timetable.teachers;
        prevTimetable->maxTeacherId = timetable.maxTeacherId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
