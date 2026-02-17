// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Timetable/Open.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "Utils.hpp"
#include <imgui.h>

std::shared_ptr<OpenTimetableMenu> openTimetableMenu;

std::vector<std::string> timetableFiles;

void OpenTimetableMenu::Open()
{
    timetableFiles = ListFiles("templates/");
    for (size_t i = 0; i < timetableFiles.size(); i++)
        timetableFiles[i] = std::filesystem::path(timetableFiles[i]).stem().string();
    Window::Open();
}

void OpenTimetableMenu::Draw()
{
    if (!ImGui::Begin(gettext("Open timetable"), &visible))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", gettext("Select a timetable to open"));
    for (size_t i = 0; i < timetableFiles.size(); i++)
    {
        if (ImGui::Button(timetableFiles[i].c_str()))
        {
            LogInfo("Opening a timetable at templates/" + timetableFiles[i] + ".json");
            currentTimetable = Timetable();
            currentTimetable.Load("templates/" + timetableFiles[i] + ".json");
            Close();
        }
    }
    ImGui::End();
}
