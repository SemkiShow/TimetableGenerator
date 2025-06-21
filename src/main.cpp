#include "Settings.hpp"
#include "UI.hpp"
#include "Timetable.hpp"
#include "Updates.hpp"
#include "Searching.hpp"

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
    CheckForUpdates(false);

    Timetable testTimetable = currentTimetable;
    RandomizeTimetable(&testTimetable);
    ScoreTimetable(&testTimetable);
    std::cout << "The current timetable score is " << testTimetable.bonusPoints - testTimetable.errors << '\n';

    #if !defined(PLATFORM_WEB)
    int flags = 0;
    if (vsync) flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);
	#endif

    InitWindow(windowSize[0], windowSize[1], ("Timetable Generator " + version).c_str());
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
