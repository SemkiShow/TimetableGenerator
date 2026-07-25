// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Timetable/Edit.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/Wizard.hpp"
#include <imgui.h>
#include <memory>
#include <misc/cpp/imgui_stdlib.h>

std::shared_ptr<NewTimetableMenu> g_newTimetableMenu;

void NewTimetableMenu::Draw()
{
    if (!ImGui::Begin((newTimetable_ ? gettext("New timetable") : gettext("Save timetable as")),
                      &visible_))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", gettext("Enter the timetable name\n(for example, the name of the school)"));
    ImGui::InputText("##", &timetableName_);
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Creating a new timetable at templates/%s.json", timetableName_.c_str());
        if (newTimetable_) g_currentTimetable = Timetable();
        g_currentTimetable.name = timetableName_;
        g_currentTimetable.Save("templates/" + timetableName_ + ".json");
        g_currentTimetable = Timetable();
        g_currentTimetable.Load("templates/" + timetableName_ + ".json");
        g_currentTimetable.Save("templates/" + timetableName_ + ".json");
        if (newTimetable_) g_wizardMenu->Open();
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
