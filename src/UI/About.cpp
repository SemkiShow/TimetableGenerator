// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/About.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include <imgui.h>

std::shared_ptr<AboutMenu> aboutMenu;

void AboutMenu::Draw()
{
    if (!ImGui::Begin(gettext("About"), &visible))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", (GetText("TimetableGenerator") + " " + version).c_str());
    ImGui::Text("%s", gettext("A tool for creating timetables easily"));
    ImGui::Text("%s", gettext("Developed by SemkiShow"));
    ImGui::Text("%s", gettext("Licensed under GPL v3 License"));
    ImGui::End();
}
