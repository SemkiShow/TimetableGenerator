// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classrooms.hpp"
#include "Logging.hpp"
#include "Translations.hpp"
#include "UI/Classrooms/Edit.hpp"
#include <cstdlib>
#include <imgui.h>
#include <memory>
#include <string>

std::shared_ptr<ClassroomsMenu> g_classroomsMenu;

void ClassroomsMenu::Draw()
{
    if (!ImGui::Begin(gettext("Classrooms"), &visible_))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(gettext("+")))
    {
        int classroomNumber =
            atoi(timetable_.classrooms[timetable_.maxClassroomId].name.c_str()) + 1;
        g_editClassroomMenu->Open(&timetable_, true, timetable_.maxClassroomId + 1, classroomNumber,
                                  classroomNumber);
        LogInfo("Adding a new classroom with id %d", timetable_.maxClassroomId + 1);
    }

    for (auto it = timetable_.classrooms.begin(); it != timetable_.classrooms.end();)
    {
        ImGui::PushID(it->first);

        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a classroom with id %d", it->first);
            ImGui::PopID();
            it = timetable_.classrooms.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            g_editClassroomMenu->Open(&timetable_, false, it->first, 0, 0);
            LogInfo("Editing a classroom with id %d", it->first);
        }
        ImGui::SameLine();

        ImGui::LabelText("", "%s", it->second.name.c_str());
        ImGui::PopID();
        ++it;
    }
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed Ok in the classrooms menu");
        prevTimetable_->classrooms = timetable_.classrooms;
        prevTimetable_->maxClassroomId = timetable_.maxClassroomId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
