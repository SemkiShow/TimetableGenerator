// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI.hpp"
#include "Logging.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "System.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/About.hpp"
#include "UI/Classes.hpp"
#include "UI/Classes/Edit.hpp"
#include "UI/Classes/Edit/CombineLessons.hpp"
#include "UI/Classes/Edit/Rules.hpp"
#include "UI/Classrooms.hpp"
#include "UI/Classrooms/Edit.hpp"
#include "UI/Crashes.hpp"
#include "UI/Faq.hpp"
#include "UI/Lessons.hpp"
#include "UI/Lessons/Edit.hpp"
#include "UI/NewVersion.hpp"
#include "UI/Settings.hpp"
#include "UI/Teachers.hpp"
#include "UI/Teachers/Edit.hpp"
#include "UI/Timetable/Edit.hpp"
#include "UI/Timetable/Generate.hpp"
#include "UI/Timetable/Open.hpp"
#include "UI/Wizard.hpp"
#include "Updates.hpp"
#include "Utils.hpp"
#include <filesystem>
#include <imgui.h>
#include <raylib.h>
#include <rlImGui.h>
#include <string>
#include <thread>

Vector2 windowSize = {16 * 50, 9 * 50};
std::string weekDays[7] = {"Monday", "Tuesday",  "Wednesday", "Thursday",
                           "Friday", "Saturday", "Sunday"};

double timetableAutosaveTimer = GetTime();

bool lastVsync = vsync;
bool lastMergedFont = mergedFont;
int lastFontSize = fontSize;

std::shared_ptr<Application> app;

void LoadResources()
{
    auto faqScreenshotFiles = ListFiles("resources/faq-screenshots");
    for (auto& texture: faqScreenshots) UnloadTexture(texture);
    for (size_t i = 0; i < faqScreenshotFiles.size(); i++)
    {
        faqScreenshots.push_back(LoadTexture(faqScreenshotFiles[i].c_str()));
    }
}

void LoadFonts()
{
    LogInfo("Loading fonts");
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("resources/fonts/ProggyClean.ttf", (float)fontSize);
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.AddText(u8"ęóąśłżźćńĘÓĄŚŁŻŹĆŃ’—");
    ImVector<ImWchar> glyphRanges;
    builder.BuildRanges(&glyphRanges);
    ImFontConfig fontConfig;
    fontConfig.MergeMode = mergedFont;
    fontConfig.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("resources/fonts/DroidSansMono.ttf", (float)fontSize, &fontConfig,
                                 glyphRanges.Data);
    io.Fonts->Build();
}

void LoadStyle()
{
    LogInfo("Loading style");
    switch (style)
    {
    case Style::Dark:
        ImGui::StyleColorsDark();
        break;
    case Style::Light:
        ImGui::StyleColorsLight();
        break;
    case Style::Classic:
        ImGui::StyleColorsClassic();
        break;
    }
}

void InitUI()
{
    app = std::make_shared<Application>();

    settingsMenu = std::make_shared<SettingsMenu>();
    settingsMenu->Close();
    app->AddWindow(settingsMenu);

    aboutMenu = std::make_shared<AboutMenu>();
    aboutMenu->Close();
    app->AddWindow(aboutMenu);

    newVersionMenu = std::make_shared<NewVersionMenu>();
    newVersionMenu->Close();
    app->AddWindow(newVersionMenu);

    newTimetableMenu = std::make_shared<NewTimetableMenu>();
    newTimetableMenu->Close();
    app->AddWindow(newTimetableMenu);

    openTimetableMenu = std::make_shared<OpenTimetableMenu>();
    openTimetableMenu->Close();
    app->AddWindow(openTimetableMenu);

    generateTimetableMenu = std::make_shared<GenerateTimetableMenu>();
    generateTimetableMenu->Close();
    app->AddWindow(generateTimetableMenu);

    wizardMenu = std::make_shared<WizardMenu>();
    wizardMenu->Close();
    app->AddWindow(wizardMenu);

    faqMenu = std::make_shared<FaqMenu>();
    faqMenu->Close();
    app->AddWindow(faqMenu);

    classroomsMenu = std::make_shared<ClassroomsMenu>();
    classroomsMenu->Close();
    app->AddWindow(classroomsMenu);

    editClassroomMenu = std::make_shared<EditClassroomMenu>();
    editClassroomMenu->Close();
    app->AddWindow(editClassroomMenu);

    lessonsMenu = std::make_shared<LessonsMenu>();
    lessonsMenu->Close();
    app->AddWindow(lessonsMenu);

    editLessonMenu = std::make_shared<EditLessonMenu>();
    editLessonMenu->Close();
    app->AddWindow(editLessonMenu);

    teachersMenu = std::make_shared<TeachersMenu>();
    teachersMenu->Close();
    app->AddWindow(teachersMenu);

    editTeacherMenu = std::make_shared<EditTeacherMenu>();
    editTeacherMenu->Close();
    app->AddWindow(editTeacherMenu);

    classesMenu = std::make_shared<ClassesMenu>();
    classesMenu->Close();
    app->AddWindow(classesMenu);

    editClassMenu = std::make_shared<EditClassMenu>();
    editClassMenu->Close();
    app->AddWindow(editClassMenu);

    combineLessonsMenu = std::make_shared<CombineLessonsMenu>();
    combineLessonsMenu->Close();
    app->AddWindow(combineLessonsMenu);

    rulesMenu = std::make_shared<RulesMenu>();
    rulesMenu->Close();
    app->AddWindow(rulesMenu);

    crashesMenu = std::make_shared<CrashesMenu>();
    crashesMenu->Close();
    app->AddWindow(crashesMenu);
}

void DrawMenuBar()
{
    if (!ImGui::BeginMainMenuBar()) return;
    if (ImGui::BeginMenu(gettext("File")))
    {
        if (ImGui::MenuItem(gettext("New")))
        {
            LogInfo("Creating a new timetable");
            newTimetableMenu->Open(true, "");
        }
        if (ImGui::MenuItem(gettext("Open")))
        {
            LogInfo("Opening a timetable");
            openTimetableMenu->Open();
        }
        if (ImGui::MenuItem(gettext("Save")))
        {
            LogInfo("Manually saving a timetable");
            currentTimetable.Save("templates/" + currentTimetable.name + ".json");
        }
        if (ImGui::MenuItem(gettext("Save As")))
        {
            LogInfo("Saving a timetable as");
            newTimetableMenu->Open(false, currentTimetable.name);
        }
        if (currentTimetable.name != "" && ImGui::BeginMenu(gettext("Export As")))
        {
            if (ImGui::MenuItem(gettext("Excel")))
            {
                LogInfo("Exporting a timetable as Excel");
                currentTimetable.ExportAsXlsx();
                OpenInFileManager("timetables/");
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem(gettext("Settings"))) settingsMenu->Open();
        ImGui::EndMenu();
    }
    if (currentTimetable.name != "" && ImGui::BeginMenu(currentTimetable.name.c_str()))
    {
        if (ImGui::MenuItem(gettext("Setup wizard"))) wizardMenu->Open();
        if (ImGui::MenuItem(gettext("Classrooms"))) classroomsMenu->Open();
        if (ImGui::MenuItem(gettext("Lessons"))) lessonsMenu->Open();
        if (ImGui::MenuItem(gettext("Teachers"))) teachersMenu->Open();
        if (ImGui::MenuItem(gettext("Classes"))) classesMenu->Open();
        if (ImGui::MenuItem(gettext("Generate timetable")))
        {
            if (iterationData.timetables != nullptr)
            {
                iterationData.isDone = true;
                generateTimetableMenu->SetStatus(
                    GetText("Allocating memory for the timetables..."));
                while (iterationData.threadLock) COMPILER_BARRIER();
                StopSearching();
            }
            while (iterationData.threadLock) COMPILER_BARRIER();
            std::thread beginSearchingThread(BeginSearching, currentTimetable);
            beginSearchingThread.detach();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(gettext("Help")))
    {
        if (ImGui::MenuItem(gettext("FAQ"))) faqMenu->Open();
        if (ImGui::MenuItem(gettext("Check for updates"))) CheckForUpdates();
        if (ImGui::MenuItem(gettext("About"))) aboutMenu->Open();
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void DrawFrame()
{
    // Begin imgui drawing
    if (lastMergedFont != mergedFont || lastFontSize != fontSize)
    {
        lastMergedFont = mergedFont;
        fontSize = std::max(5, fontSize);
        lastFontSize = fontSize;
        LoadFonts();
    }
    ImGuiIO& io = ImGui::GetIO();
    auto dpi = GetWindowScaleDPI();
    io.DisplayFramebufferScale = {dpi.x, dpi.y};
    ImGui::PushFont(io.Fonts->Fonts.back());

    BeginDrawing();

    // Change the background color based on the style
    if (style == Style::Dark || style == Style::Dark) ClearBackground(BLACK);
    if (style == Style::Light) ClearBackground(WHITE);

    rlImGuiBegin();

    // Draw UI
    DrawMenuBar();
    app->Update();
    app->Draw();

    // Autosave the timetable
    if (GetTime() - timetableAutosaveTimer > timetableAutosaveInterval)
    {
        timetableAutosaveTimer = GetTime();
        currentTimetable.Save("templates/" + currentTimetable.name + ".json");
    }

    // Change vsync state
    if (lastVsync != vsync)
    {
        lastVsync = vsync;
        if (!vsync)
            ClearWindowState(FLAG_VSYNC_HINT);
        else
            SetWindowState(FLAG_VSYNC_HINT);
    }

    // End imgui drawing
    ImGui::PopFont();
    rlImGuiEnd();

    EndDrawing();
}

void FreeResources()
{
    for (auto& texture: faqScreenshots)
    {
        UnloadTexture(texture);
    }
    faqScreenshots.clear();
}
