// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Settings.hpp"
// #include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "UI/Crashes.hpp"
#include "Updates.hpp"
#include <ctime>
#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

int main()
{
    BeginLogging();
    srand(time(0));

    // Timetable save and load testing
    // Timetable timetableSave;
    // timetableSave.name = "save";
    // GenerateRandomTimetable(&timetableSave);
    // SaveTimetable("timetables/save.json", &timetableSave);
    // Timetable timetableLoad;
    // LoadTimetable("timetables/save.json", &timetableLoad);
    // SaveTimetable("timetables/load.json", &timetableLoad);

    // Load settings
    InitUI();
    settings.Load();
    if (settings.hasCrashed) crashesMenu->Open();
    settings.hasCrashed = true;
    settings.Save();
    CheckForUpdates(false);

    // Set raylib config flags
    int flags = 0;
    if (settings.vsync) flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);

    // Init raylib
    InitWindow(windowSize.x, windowSize.y,
               (GetText("Timetable Generator") + " " + version).c_str());
    SetExitKey(-1);

    LoadResources();

    // Init imgui
    rlImGuiSetup(true);
    LoadFonts();
    LoadStyle();
#ifdef IMGUI_HAS_DOCK
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
#endif

    // Main loop
    while (!WindowShouldClose())
    {
        DrawFrame();
    }

    // Save settings and close the program
    settings.hasCrashed = false;
    settings.Save();
    EndLogging();
    rlImGuiShutdown();
    FreeResources();
    CloseWindow();

    return 0;
}
