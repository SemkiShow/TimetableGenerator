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
    SaveTimetable("timetables/save.json", &timetableSave);

    Timetable timetableLoad;
    LoadTimetable("timetables/save.json", &timetableLoad);
    SaveTimetable("timetables/load.json", &timetableLoad);

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

    // Load missing glyphs
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.AddText(u8"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ");
    ImVector<ImWchar> glyphRanges;
    builder.BuildRanges(&glyphRanges);
    ImFontConfig fontConfig;
    fontConfig.MergeMode = true;
    fontConfig.PixelSnapH = true;
    ImFont* font = io.Fonts->AddFontFromFileTTF("resources/Roboto-Regular.ttf", 16.0f, (mergedFont ? &fontConfig : nullptr), glyphRanges.Data);
    io.Fonts->Build();

    while (!WindowShouldClose())
    {
        DrawFrame();
    }

    Save("settings.txt");
    rlImGuiShutdown();
	CloseWindow();

	return 0;
}
