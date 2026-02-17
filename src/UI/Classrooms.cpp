// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classrooms.hpp"
#include "Logging.hpp"
#include "Translations.hpp"
#include "UI/Classrooms/Edit.hpp"
#include <imgui.h>
#include <string>

std::shared_ptr<ClassroomsMenu> classroomsMenu;

void ClassroomsMenu::Draw()
{
    if (!ImGui::Begin(gettext("Classrooms"), &visible))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(gettext("+")))
    {
        int classroomNumber = 0;
        try
        {
            classroomNumber = stoi(timetable.classrooms[timetable.maxClassroomId].name) + 1;
        }
        catch (const std::exception&)
        {
        }
        editClassroomMenu->Open(&timetable, true, timetable.maxClassroomId + 1, classroomNumber,
                                classroomNumber);
        LogInfo("Adding a new classroom with id " + std::to_string(timetable.maxClassroomId + 1));
    }

    for (auto it = timetable.classrooms.begin(); it != timetable.classrooms.end();)
    {
        ImGui::PushID(it->first);

        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a classroom with id " + std::to_string(it->first));
            ImGui::PopID();
            it = timetable.classrooms.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            editClassroomMenu->Open(&timetable, false, it->first, 0, 0);
            LogInfo("Editing a classroom with id " + std::to_string(it->first));
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
        prevTimetable->classrooms = timetable.classrooms;
        prevTimetable->maxClassroomId = timetable.maxClassroomId;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
