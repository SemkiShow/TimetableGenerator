// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Settings.hpp"
// #include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "UI/Crashes.hpp"
#include "Updates.hpp"
#include <cstdlib>
#include <ctime>
#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

int main()
{
    srand(time(nullptr));

    // Load settings
    InitUI();
    g_settings.Load();
    if (g_settings.hasCrashed) g_crashesMenu->Open();
    g_settings.hasCrashed = true;
    g_settings.Save();
    CheckForUpdates(false);

    // Set raylib config flags
    int flags = 0;
    if (g_settings.vsync) flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);

    // Init raylib
    InitWindow((int)g_windowSize.x, (int)g_windowSize.y,
               (GetText("Timetable Generator") + " " + g_version).c_str());
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
    g_settings.hasCrashed = false;
    g_settings.Save();
    rlImGuiShutdown();
    CloseWindow();

    return 0;
}
