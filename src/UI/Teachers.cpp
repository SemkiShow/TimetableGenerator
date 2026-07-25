// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Teachers.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI/Teachers/Edit.hpp"
#include <algorithm>
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <string>

std::shared_ptr<TeachersMenu> g_teachersMenu;

void TeachersMenu::Draw()
{
    if (!ImGui::Begin(gettext("Teachers"), &visible_))
    {
        ImGui::End();
        return;
    }

    ImGui::InputInt(gettext("min free periods"), &g_settings.minFreePeriods);
    g_settings.minFreePeriods = std::max(g_settings.minFreePeriods, 0);
    ImGui::InputInt(gettext("max free periods"), &g_settings.maxFreePeriods);
    g_settings.maxFreePeriods = std::max(g_settings.maxFreePeriods, 0);
    g_settings.maxFreePeriods = std::max(g_settings.maxFreePeriods, g_settings.minFreePeriods);

    if (ImGui::Button(gettext("+")))
    {
        LogInfo("Adding a new teacher with id %d", timetable_.maxTeacherId + 1);
        g_editTeacherMenu->Open(&timetable_, true, timetable_.maxTeacherId + 1);
    }
    ImGui::Separator();

    ImGui::Columns(2);
    for (auto it = timetable_.teachers.begin(); it != timetable_.teachers.end();)
    {
        ImGui::PushID(it->first);
        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a teacher with id %d", it->first);
            ImGui::PopID();
            it = timetable_.teachers.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a teacher with id %d", it->first);
            g_editTeacherMenu->Open(&timetable_, false, it->first);
        }
        ImGui::SameLine();

        ImGui::Text("%s", it->second.name.c_str());
        ImGui::NextColumn();

        std::string lessonNames;
        for (size_t j = 0; j < timetable_.teachers[it->first].lessonIds.size(); j++)
        {
            lessonNames +=
                prevTimetable_->lessons[timetable_.teachers[it->first].lessonIds[j]].name;
            if (j < timetable_.teachers[it->first].lessonIds.size() - 1) lessonNames += ' ';
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
        prevTimetable_->teachers = timetable_.teachers;
        prevTimetable_->maxTeacherId = timetable_.maxTeacherId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
