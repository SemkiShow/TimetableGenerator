// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Timetable/Edit.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/Wizard.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

std::shared_ptr<NewTimetableMenu> newTimetableMenu;

void NewTimetableMenu::Draw()
{
    if (!ImGui::Begin((newTimetable ? gettext("New timetable") : gettext("Save timetable as")),
                      &visible))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", gettext("Enter the timetable name\n(for example, the name of the school)"));
    ImGui::InputText("##", &timetableName);
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Creating a new timetable at templates/" + timetableName + ".json");
        if (newTimetable) currentTimetable = Timetable();
        currentTimetable.name = timetableName;
        currentTimetable.Save("templates/" + timetableName + ".json");
        currentTimetable = Timetable();
        currentTimetable.Load("templates/" + timetableName + ".json");
        currentTimetable.Save("templates/" + timetableName + ".json");
        if (newTimetable) wizardMenu->Open();
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
