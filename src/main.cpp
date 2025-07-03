#include "Crashes.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include "Timetable.hpp"
#include "Updates.hpp"
#include "Logging.hpp"
#include <ctime>
#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>

int main()
{
    BeginLogging();
    srand(time(0));

    // Timetable save and load testing
    Timetable timetableSave;
    timetableSave.name = "save";
    GenerateRandomTimetable(&timetableSave);
    SaveTimetable("timetables/save.json", &timetableSave);
    Timetable timetableLoad;
    LoadTimetable("timetables/save.json", &timetableLoad);
    SaveTimetable("timetables/load.json", &timetableLoad);

    // Load settings
    Load("settings.txt");
    if (hasCrashed) OpenCrashReport();
    hasCrashed = true;
    Save("settings.txt");
    CheckForUpdates(false);

    // Set raylib config flags
    int flags = 0;
    if (vsync) flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);

    // Init raylib
    InitWindow(windowSize[0], windowSize[1], (labels["Timetable Generator"] + " " + version).c_str());
    SetExitKey(-1);

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
    hasCrashed = false;
    Save("settings.txt");
    EndLogging();
    rlImGuiShutdown();
	CloseWindow();

	return 0;
}
