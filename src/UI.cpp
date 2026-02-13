// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI.hpp"
#include "Crashes.hpp"
#include "Logging.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "System.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "Updates.hpp"
#include <cmath>
#include <filesystem>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <raylib.h>
#include <rlImGui.h>
#include <string>
#include <thread>

int menuOffset = 20;
int windowSize[2] = {16 * 50, 9 * 50};
std::string weekDays[7] = {"Monday", "Tuesday",  "Wednesday", "Thursday",
                           "Friday", "Saturday", "Sunday"};
std::string styleValues = "";

std::vector<std::string> timetableFiles;

std::vector<Texture2D> faqScreenshots;

int timetableSaveTimer = GetTime();

std::string generateTimetableStatus = "";

bool lastVsync = vsync;
bool lastMergedFont = mergedFont;
int lastFontSize = fontSize;

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

void LoadFAQScreenshots()
{
    std::vector<std::string> faqScreenshotFiles;
    ListFiles("resources/faq-screenshots", &faqScreenshotFiles);
    for (auto& texture: faqScreenshots) UnloadTexture(texture);
    for (size_t i = 0; i < faqScreenshotFiles.size(); i++)
    {
        faqScreenshots.push_back(LoadTexture(faqScreenshotFiles[i].c_str()));
    }
}

void OpenClassrooms()
{
    LogInfo("Opening classrooms");
    tmpTimetable.classrooms = currentTimetable.classrooms;
    tmpTimetable.maxClassroomID = currentTimetable.maxClassroomID;
    isClassrooms = true;
}

void OpenLessons()
{
    LogInfo("Opening lessons");
    tmpTimetable.lessons = currentTimetable.lessons;
    tmpTimetable.maxLessonID = currentTimetable.maxLessonID;
    isLessons = true;
}

void OpenTeachers()
{
    LogInfo("Opening teachers");
    tmpTimetable.teachers = currentTimetable.teachers;
    tmpTimetable.maxTeacherID = currentTimetable.maxTeacherID;
    isTeachers = true;
}

void OpenClasses()
{
    LogInfo("Opening classes");
    tmpTimetable.classes = currentTimetable.classes;
    tmpTimetable.maxClassID = currentTimetable.maxClassID;
    tmpTimetable.orderedClasses = currentTimetable.orderedClasses;
    tmpLessons = currentTimetable.lessons;
    tmpTimetable.year = currentTimetable.year;
    isClasses = true;
}

void OpenWizard()
{
    LogInfo("Opening the setup wizard");
    isWizard = true;
}

bool isSettings = false;
void ShowSettings(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Settings"), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputScalar(gettext("days per week"), ImGuiDataType_U32, &daysPerWeek);
    if (daysPerWeek < 1) daysPerWeek = 1;
    ImGui::InputScalar(gettext("lessons per day"), ImGuiDataType_U32, &lessonsPerDay);
    if (lessonsPerDay < 1) lessonsPerDay = 1;
    int styleInt = static_cast<int>(style);
    if (ImGui::Combo(gettext("style"), &styleInt, styleValues.c_str()))
    {
        LoadStyle();
        style = static_cast<Style>(styleInt);
    }
    if (ImGui::Combo(gettext("language"), &languageId, languageValues.c_str()))
    {
        language = availableLanguages[languageId];
        ReloadLabels();
    }
    if (ImGui::TreeNode(gettext("Developer options")))
    {
        ImGui::Checkbox(gettext("vsync"), &vsync);
        ImGui::Checkbox(gettext("merged-font"), &mergedFont);
        ImGui::SliderInt(gettext("timetable-autosave-interval"), &timetableAutosaveInterval, 0,
                         600);
        ImGui::InputInt(gettext("font-size"), &fontSize);
        if (fontSize < 5) fontSize = 5;
        ImGui::SliderFloat(gettext("error-bonus-ratio"), &errorBonusRatio, 0.1f, 100.0f);
        ImGui::SliderInt(gettext("timetables-per-generation-step"), &timetablesPerGenerationStep, 1,
                         100);
        ImGui::SliderInt(gettext("min-timetables-per-generation"), &minTimetablesPerGeneration, 10,
                         10000);
        ImGui::SliderInt(gettext("max-timetables-per-generation"), &maxTimetablesPerGeneration, 10,
                         10000);
        if (maxTimetablesPerGeneration < minTimetablesPerGeneration)
            maxTimetablesPerGeneration = minTimetablesPerGeneration;
        ImGui::SliderInt(gettext("max-iterations"), &maxIterations, -1, 10000);
        ImGui::SliderInt(gettext("additional-bonus-points"), &additionalBonusPoints, 0, 100);
        if (ImGui::Checkbox(gettext("verbose-logging"), &verboseLogging))
        {
            while (iterationData.threadLock) COMPILER_BARRIER();
            iterationData.threadLock = true;
            threadsNumber =
                (verboseLogging ? 1
                                : std::max(std::thread::hardware_concurrency(), (unsigned int)1));
            iterationData.threadLock = false;
        }
        ImGui::Checkbox(gettext("use-prereleases"), &usePrereleases);
        ImGui::TreePop();
    }
    ImGui::End();
}

bool isAbout = false;
void ShowAbout(bool* isOpen)
{
    if (!ImGui::Begin(gettext("About"), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", (std::string(gettext("TimetableGenerator")) + " " + version).c_str());
    ImGui::Text("%s", gettext("A tool for creating timetables easily"));
    ImGui::Text("%s", gettext("Developed by SemkiShow"));
    ImGui::Text("%s", gettext("Licensed under GPLv3 License"));
    ImGui::End();
}

bool isNewVersion = false;
void ShowNewVersion(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Updates"), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s",
                (std::string(gettext("The latest version is")) + " " + latestVersion).c_str());
    ImGui::Text("%s", (std::string(gettext("Your version is")) + " " + version).c_str());
    if (version == latestVersion)
        ImGui::Text("%s", gettext("There are no new versions available"));
    else if (latestVersion != gettext("loading..."))
    {
        ImGui::Text("%s", gettext("A new version is available!"));
        if (ImGui::TreeNode(gettext("Release notes")))
        {
            for (int i = 0; i < (int)releaseNotes.size() - 2; i++)
            {
                ImGui::Text("%s", releaseNotes[i].c_str());
            }
            ImGui::TreePop();
        }
        if (downloadStatus != "") ImGui::Text("%s", downloadStatus.c_str());
        if (ImGui::Button(gettext("Update")))
        {
            std::thread updateThread(UpdateToLatestVersion);
            updateThread.detach();
        }
    }
    ImGui::End();
}

bool isNewTimetable = false;
bool newTimetable = false;
std::string timetableName = "";
void ShowNewTimetable(bool* isOpen)
{
    if (!ImGui::Begin((newTimetable ? gettext("New timetable") : gettext("Save timetable as")),
                      isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", gettext("Enter the timetable name\n(for example, the name of the school)"));
    ImGui::InputText("##", &timetableName);
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Creating a new timetable at templates/" + timetableName + ".json");
        if (newTimetable) currentTimetable = Timetable();
        currentTimetable.name = timetableName;
        currentTimetable.Save("templates/" + timetableName + ".json");
        currentTimetable = Timetable();
        currentTimetable.Load("templates/" + timetableName + ".json");
        currentTimetable.Save("templates/" + timetableName + ".json");
        if (newTimetable) OpenWizard();
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}

bool isOpenTimetable = false;
void ShowOpenTimetable(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Open timetable"), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", gettext("Select a timetable to open"));
    for (size_t i = 0; i < timetableFiles.size(); i++)
    {
        if (ImGui::Button(timetableFiles[i].c_str()))
        {
            LogInfo("Opening a timetable at templates/" + timetableFiles[i] + ".json");
            currentTimetable = Timetable();
            currentTimetable.Load("templates/" + timetableFiles[i] + ".json");
            *isOpen = false;
        }
    }
    ImGui::End();
}

bool wasGenerateTimetable = false;
bool isGenerateTimetable = false;
void ShowGenerateTimetable(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Generate timetable"), isOpen))
    {
        ImGui::End();
        return;
    }
    if (generateTimetableStatus == gettext("Timetable generating done!"))
    {
        ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s", gettext("Timetable generating done!"));
    }
    else
    {
        ImGui::Text("%s", generateTimetableStatus.c_str());
    }
    if (generateTimetableStatus == gettext("Allocating memory for the timetables..."))
    {
        ImGui::LabelText("##1", "\n\n\n\n\n\n\n");
    }
    else
    {
        ImGui::Text("%s", (std::string(gettext("Iteration:")) + " " +
                           std::to_string(iterationData.iteration))
                              .c_str());
        ImGui::Text("%s", (std::string(gettext("The best score is")) + " " +
                           std::to_string(iterationData.allTimeBestScore))
                              .c_str());
        ImGui::Text("%s", (std::string(gettext("The best timetable has")) + " " +
                           std::to_string(
                               iterationData.timetables[iterationData.bestTimetableIndex].errors) +
                           " " + gettext("errors"))
                              .c_str());
        ImGui::Text("%s",
                    (std::string(gettext("The best timetable has")) + " " +
                     std::to_string(
                         iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints) +
                     " " + gettext("bonus points"))
                        .c_str());
        ImGui::Text("%s", (std::to_string(iterationData.iterationsPerChange) + " " +
                           gettext("iterations have passed since last score improvement"))
                              .c_str());
        float progressPercentage = 1;
        if (generateTimetableStatus ==
            gettext("Generating a timetable that matches the requirements..."))
        {
            progressPercentage =
                (-(iterationData.maxErrors * 1.0f / 100) * iterationData.minErrors + 100) / 100;
        }
        else if (generateTimetableStatus == gettext("Finding additional bonus points..."))
        {
            progressPercentage = (iterationData.maxBonusPoints - iterationData.startBonusPoints) *
                                 1.0f / additionalBonusPoints;
        }
        ImGui::ProgressBar(pow(progressPercentage, 2));
        ImGui::PlotLines(gettext("errors"), iterationData.errorValues,
                         iterationData.errorValuesPoints, 0, NULL, FLT_MAX, FLT_MAX,
                         ImVec2(0, 100));
    }
    ImGui::End();
}

void DrawImage(Texture2D texture)
{
    ImGui::Image((ImTextureID)(uintptr_t)texture.id,
                 ImVec2(texture.width * 1.0f * fontSize / DEFAULT_FONT_SIZE,
                        texture.height * 1.0f * fontSize / DEFAULT_FONT_SIZE));
}

bool isFAQ = false;
void ShowFAQ(bool* isOpen)
{
    if (!ImGui::Begin(gettext("FAQ"), isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::TreeNode(gettext("How do I contact the developer?")))
    {
        ImGui::Text("%s",
                    gettext("You can contact me by sending an email to mgdeveloper123@gmail.com"));
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(gettext("How do I add multiple lessons to one timetable cell?")))
    {
        ImGui::Text(
            "%s",
            gettext(
                "To add multiple lessons to one timetable cell, click\nCombine lessons while editing a class."));
        DrawImage(faqScreenshots[0]);
        ImGui::Text("%s", gettext("Select the lessons and teachers to combine and press Ok."));
        DrawImage(faqScreenshots[1]);
        ImGui::Text("%s", gettext("Then set the amount per week for the created combined lesson."));
        DrawImage(faqScreenshots[2]);
        ImGui::TreePop();
    }
    ImGui::End();
}

void ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(gettext("File")))
        {
            if (ImGui::MenuItem(gettext("New")))
            {
                LogInfo("Creating a new timetable");
                newTimetable = true;
                timetableName = "";
                isNewTimetable = true;
            }
            if (ImGui::MenuItem(gettext("Open")))
            {
                LogInfo("Opening a timetable");
                ListFiles("templates/", &timetableFiles);
                for (size_t i = 0; i < timetableFiles.size(); i++)
                    timetableFiles[i] = std::filesystem::path(timetableFiles[i]).stem().string();
                isOpenTimetable = true;
            }
            if (ImGui::MenuItem(gettext("Save")))
            {
                LogInfo("Manually saving a timetable");
                currentTimetable.Save("templates/" + currentTimetable.name + ".json");
            }
            if (ImGui::MenuItem(gettext("Save As")))
            {
                LogInfo("Saving a timetable as");
                newTimetable = false;
                timetableName = currentTimetable.name;
                isNewTimetable = true;
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
            if (ImGui::MenuItem(gettext("Settings"))) isSettings = true;
            ImGui::EndMenu();
        }
        if (currentTimetable.name != "" && ImGui::BeginMenu(currentTimetable.name.c_str()))
        {
            if (ImGui::MenuItem(gettext("Setup wizard"))) OpenWizard();
            if (ImGui::MenuItem(gettext("Classrooms"))) OpenClassrooms();
            if (ImGui::MenuItem(gettext("Lessons"))) OpenLessons();
            if (ImGui::MenuItem(gettext("Teachers"))) OpenTeachers();
            if (ImGui::MenuItem(gettext("Classes"))) OpenClasses();
            if (ImGui::MenuItem(gettext("Generate timetable")))
            {
                if (iterationData.timetables != nullptr)
                {
                    iterationData.isDone = true;
                    generateTimetableStatus = gettext("Allocating memory for the timetables...");
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
            if (ImGui::MenuItem(gettext("FAQ"))) isFAQ = true;
            if (ImGui::MenuItem(gettext("Check for updates"))) CheckForUpdates();
            if (ImGui::MenuItem(gettext("About"))) isAbout = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    return;
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

    // Draw GUI
    ShowMenuBar();
    if (isSettings) ShowSettings(&isSettings);
    if (isEditClassroom) ShowEditClassroom(&isEditClassroom);
    if (isClassrooms) ShowClassrooms(&isClassrooms);
    if (isEditLesson) ShowEditLesson(&isEditLesson);
    if (isLessons) ShowLessons(&isLessons);
    if (isEditTeacher) ShowEditTeacher(&isEditTeacher);
    if (isTeachers) ShowTeachers(&isTeachers);
    if (isCombineLessons) ShowCombineLessons(&isCombineLessons);
    if (isRules) ShowRules(&isRules);
    if (isEditClass) ShowEditClass(&isEditClass);
    if (isClasses) ShowClasses(&isClasses);
    if (isAbout) ShowAbout(&isAbout);
    ShowWizard(&isWizard);
    if (isNewVersion) ShowNewVersion(&isNewVersion);
    if (isNewTimetable) ShowNewTimetable(&isNewTimetable);
    if (isOpenTimetable) ShowOpenTimetable(&isOpenTimetable);
    if (isGenerateTimetable) ShowGenerateTimetable(&isGenerateTimetable);
    if (isCrashReport) ShowCrashReport(&isCrashReport);
    if (isFAQ) ShowFAQ(&isFAQ);

    // Stop searching for a perfect timetable if the Generate timetable window is closed
    if (!isGenerateTimetable && wasGenerateTimetable)
    {
        wasGenerateTimetable = false;
        iterationData.isDone = true;
        while (iterationData.threadLock) COMPILER_BARRIER();
        std::thread stopSearchingThread(StopSearching);
        stopSearchingThread.detach();
    }

    // Autosave the timetable
    if (GetTime() - timetableSaveTimer > timetableAutosaveInterval)
    {
        timetableSaveTimer = GetTime();
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
