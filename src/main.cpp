#include "Settings.hpp"
#include "UI.hpp"
#include "Timetable.hpp"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

int main()
{
    srand(time(0));
    Timetable timetableSave;
    timetableSave.name = "save";
    GenerateRandomTimetable(&timetableSave);
    SaveTimetable("timetables/save.json", &timetableSave);

    Timetable timetableLoad;
    LoadTimetable("timetables/save.json", &timetableLoad);
    SaveTimetable("timetables/load.json", &timetableLoad);
    Load("settings.txt");

    #if !defined(PLATFORM_WEB)
    int flags = 0;
    if (vsync) flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);
	#endif

    InitWindow(windowSize[0], windowSize[1], "Timetable Generator");
    SetExitKey(-1);

    rlImGuiSetup(true);
    #ifdef IMGUI_HAS_DOCK
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    #endif

    LoadFonts();

    while (!WindowShouldClose())
    {
        DrawFrame();
    }

    Save("settings.txt");
    rlImGuiShutdown();
	CloseWindow();

	return 0;
}
