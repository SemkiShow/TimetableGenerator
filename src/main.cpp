#include "Settings.hpp"
#include "UI.hpp"
#include "Timetable.hpp"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

int main()
{
    srand(time(0));
    Load("settings.txt");

    Timetable timetableSave;
    GenerateRandomTimetable(&timetableSave);
    SaveTimetable("save", &timetableSave);

    Timetable timetableLoad;
    LoadTimetable("save", &timetableLoad);
    SaveTimetable("load", &timetableLoad);

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

    while (!WindowShouldClose())
    {
        DrawFrame();
    }

    Save("settings.txt");
    rlImGuiShutdown();
	CloseWindow();

	return 0;
}
