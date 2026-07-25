// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Timetable/Open.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "Utils.hpp"
#include "Widgets/Window.hpp"
#include <cstddef>
#include <filesystem>
#include <imgui.h>
#include <memory>
#include <string>
#include <vector>

std::shared_ptr<OpenTimetableMenu> g_openTimetableMenu;

static std::vector<std::string> g_timetableFiles;

void OpenTimetableMenu::Open()
{
    g_timetableFiles = ListFiles("templates/");
    for (auto& file: g_timetableFiles) file = std::filesystem::path(file).stem().string();
    Window::Open();
}

void OpenTimetableMenu::Draw()
{
    if (!ImGui::Begin(gettext("Open timetable"), &visible_))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", gettext("Select a timetable to open"));
    for (size_t i = 0; i < g_timetableFiles.size(); i++)
    {
        if (ImGui::Button(g_timetableFiles[i].c_str()))
        {
            LogInfo("Opening a timetable at templates/%s.json", g_timetableFiles[i].c_str());
            g_currentTimetable = Timetable();
            g_currentTimetable.Load("templates/" + g_timetableFiles[i] + ".json");
            Close();
        }
    }
    ImGui::End();
}
