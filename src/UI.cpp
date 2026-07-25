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
#include "Widgets/Application.hpp"
#include <cstddef>
#include <filesystem>
#include <imgui.h>
#include <memory>
#include <raylib.h>
#include <rlImGui.h>
#include <string>
#include <thread>
#include <vector>

constexpr Vector2 INITIAL_WINDOW_SIZE = {16 * 50, 9 * 50};
Vector2 g_windowSize = INITIAL_WINDOW_SIZE;
std::vector<const char*> g_weekDays = {"Monday", "Tuesday",  "Wednesday", "Thursday",
                                       "Friday", "Saturday", "Sunday"};

static double g_timetableAutosaveTimer = GetTime();

static bool g_lastVsync = g_settings.vsync;
static bool g_lastMergedFont = g_settings.mergedFont;
static int g_lastFontSize = g_settings.fontSize;

std::shared_ptr<Application> g_app;

void LoadResources()
{
    auto faqScreenshotFiles = ListFiles("resources/faq-screenshots");
    for (auto& texture: g_faqScreenshots) UnloadTexture(texture);
    for (size_t i = 0; i < faqScreenshotFiles.size(); i++)
    {
        g_faqScreenshots.push_back(LoadTexture(faqScreenshotFiles[i].c_str()));
    }
}

void LoadFonts()
{
    LogInfo("Loading fonts");
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromFileTTF("resources/fonts/ProggyClean.ttf",
                                 static_cast<float>(g_settings.fontSize));
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.AddText(u8"ęóąśłżźćńĘÓĄŚŁŻŹĆŃ’—");
    ImVector<ImWchar> glyphRanges;
    builder.BuildRanges(&glyphRanges);
    ImFontConfig fontConfig;
    fontConfig.MergeMode = g_settings.mergedFont;
    fontConfig.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("resources/fonts/DroidSansMono.ttf",
                                 static_cast<float>(g_settings.fontSize), &fontConfig,
                                 glyphRanges.Data);
    io.Fonts->Build();
}

void LoadStyle()
{
    LogInfo("Loading style");
    switch (g_settings.style)
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
    g_app = std::make_shared<Application>();

    g_settingsMenu = std::make_shared<SettingsMenu>();
    g_settingsMenu->Close();
    g_app->AddWindow(g_settingsMenu);

    g_aboutMenu = std::make_shared<AboutMenu>();
    g_aboutMenu->Close();
    g_app->AddWindow(g_aboutMenu);

    g_newVersionMenu = std::make_shared<NewVersionMenu>();
    g_newVersionMenu->Close();
    g_app->AddWindow(g_newVersionMenu);

    g_newTimetableMenu = std::make_shared<NewTimetableMenu>();
    g_newTimetableMenu->Close();
    g_app->AddWindow(g_newTimetableMenu);

    g_openTimetableMenu = std::make_shared<OpenTimetableMenu>();
    g_openTimetableMenu->Close();
    g_app->AddWindow(g_openTimetableMenu);

    g_generateTimetableMenu = std::make_shared<GenerateTimetableMenu>();
    g_generateTimetableMenu->Close();
    g_app->AddWindow(g_generateTimetableMenu);

    g_wizardMenu = std::make_shared<WizardMenu>();
    g_wizardMenu->Close();
    g_app->AddWindow(g_wizardMenu);

    g_faqMenu = std::make_shared<FaqMenu>();
    g_faqMenu->Close();
    g_app->AddWindow(g_faqMenu);

    g_classroomsMenu = std::make_shared<ClassroomsMenu>();
    g_classroomsMenu->Close();
    g_app->AddWindow(g_classroomsMenu);

    g_editClassroomMenu = std::make_shared<EditClassroomMenu>();
    g_editClassroomMenu->Close();
    g_app->AddWindow(g_editClassroomMenu);

    g_lessonsMenu = std::make_shared<LessonsMenu>();
    g_lessonsMenu->Close();
    g_app->AddWindow(g_lessonsMenu);

    g_editLessonMenu = std::make_shared<EditLessonMenu>();
    g_editLessonMenu->Close();
    g_app->AddWindow(g_editLessonMenu);

    g_teachersMenu = std::make_shared<TeachersMenu>();
    g_teachersMenu->Close();
    g_app->AddWindow(g_teachersMenu);

    g_editTeacherMenu = std::make_shared<EditTeacherMenu>();
    g_editTeacherMenu->Close();
    g_app->AddWindow(g_editTeacherMenu);

    g_classesMenu = std::make_shared<ClassesMenu>();
    g_classesMenu->Close();
    g_app->AddWindow(g_classesMenu);

    g_editClassMenu = std::make_shared<EditClassMenu>();
    g_editClassMenu->Close();
    g_app->AddWindow(g_editClassMenu);

    g_combineLessonsMenu = std::make_shared<CombineLessonsMenu>();
    g_combineLessonsMenu->Close();
    g_app->AddWindow(g_combineLessonsMenu);

    g_rulesMenu = std::make_shared<RulesMenu>();
    g_rulesMenu->Close();
    g_app->AddWindow(g_rulesMenu);

    g_crashesMenu = std::make_shared<CrashesMenu>();
    g_crashesMenu->Close();
    g_app->AddWindow(g_crashesMenu);
}

static void DrawMenuBar()
{
    if (!ImGui::BeginMainMenuBar()) return;
    if (ImGui::BeginMenu(gettext("File")))
    {
        if (ImGui::MenuItem(gettext("New")))
        {
            LogInfo("Creating a new timetable");
            g_newTimetableMenu->Open(true, "");
        }
        if (ImGui::MenuItem(gettext("Open")))
        {
            LogInfo("Opening a timetable");
            g_openTimetableMenu->Open();
        }
        if (ImGui::MenuItem(gettext("Save")))
        {
            LogInfo("Manually saving a timetable");
            g_currentTimetable.Save("templates/" + g_currentTimetable.name + ".json");
        }
        if (ImGui::MenuItem(gettext("Save As")))
        {
            LogInfo("Saving a timetable as");
            g_newTimetableMenu->Open(false, g_currentTimetable.name);
        }
        if (g_currentTimetable.name != "" && ImGui::BeginMenu(gettext("Export As")))
        {
            if (ImGui::MenuItem(gettext("Excel")))
            {
                LogInfo("Exporting a timetable as Excel");
                g_currentTimetable.ExportAsXlsx();
                OpenInFileManager("timetables/");
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem(gettext("Settings"))) g_settingsMenu->Open();
        ImGui::EndMenu();
    }
    if (g_currentTimetable.name != "" && ImGui::BeginMenu(g_currentTimetable.name.c_str()))
    {
        if (ImGui::MenuItem(gettext("Setup wizard"))) g_wizardMenu->Open();
        if (ImGui::MenuItem(gettext("Classrooms"))) g_classroomsMenu->Open();
        if (ImGui::MenuItem(gettext("Lessons"))) g_lessonsMenu->Open();
        if (ImGui::MenuItem(gettext("Teachers"))) g_teachersMenu->Open();
        if (ImGui::MenuItem(gettext("Classes"))) g_classesMenu->Open();
        if (ImGui::MenuItem(gettext("Generate timetable")))
        {
            std::thread beginSearchingThread(BeginSearching, g_currentTimetable);
            beginSearchingThread.detach();
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(gettext("Help")))
    {
        if (ImGui::MenuItem(gettext("FAQ"))) g_faqMenu->Open();
        if (ImGui::MenuItem(gettext("Check for updates"))) CheckForUpdates();
        if (ImGui::MenuItem(gettext("About"))) g_aboutMenu->Open();
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void DrawFrame()
{
    // Begin imgui drawing
    if (g_lastMergedFont != g_settings.mergedFont || g_lastFontSize != g_settings.fontSize)
    {
        g_lastMergedFont = g_settings.mergedFont;
        g_lastFontSize = g_settings.fontSize;
        LoadFonts();
    }
    ImGuiIO& io = ImGui::GetIO();
    auto dpi = GetWindowScaleDPI();
    io.DisplayFramebufferScale = {dpi.x, dpi.y};
    ImGui::PushFont(io.Fonts->Fonts.back());

    BeginDrawing();

    // Change the background color based on the style
    if (g_settings.style == Style::Dark || g_settings.style == Style::Dark) ClearBackground(BLACK);
    if (g_settings.style == Style::Light) ClearBackground(WHITE);

    rlImGuiBegin();

    // Draw UI
    DrawMenuBar();
    g_app->Update();
    g_app->Draw();

    // Autosave the timetable
    if (GetTime() - g_timetableAutosaveTimer > g_settings.autosaveInterval)
    {
        g_timetableAutosaveTimer = GetTime();
        g_currentTimetable.Save("templates/" + g_currentTimetable.name + ".json");
    }

    // Change vsync state
    if (g_lastVsync != g_settings.vsync)
    {
        g_lastVsync = g_settings.vsync;
        if (!g_settings.vsync)
            ClearWindowState(FLAG_VSYNC_HINT);
        else
            SetWindowState(FLAG_VSYNC_HINT);
    }

    // End imgui drawing
    ImGui::PopFont();
    rlImGuiEnd();

    EndDrawing();
}
