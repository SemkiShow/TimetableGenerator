#include "UI.hpp"
#include "Crashes.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Updates.hpp"
#include "Searching.hpp"
#include "System.hpp"
#include <cmath>
#include <filesystem>
#include <string>
#include <thread>
#include <raylib.h>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <rlImGui.h>

int menuOffset = 20;
int windowSize[2] = {16*50*2, 9*50*2};
std::string weekDays[7] = {
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};
std::vector<std::string> timetableFiles;

int timetableSaveTimer = GetTime();

bool lastVsync = vsync;
bool lastMergedFont = mergedFont;
int lastFontSize = fontSize;

void LoadFonts()
{
    LogInfo("Loading fonts");
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("resources/ProggyClean.ttf", (float)fontSize);
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.AddText(u8"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ’—");
    ImVector<ImWchar> glyphRanges;
    builder.BuildRanges(&glyphRanges);
    ImFontConfig fontConfig;
    fontConfig.MergeMode = mergedFont;
    fontConfig.PixelSnapH = true;
    ImFont* font = io.Fonts->AddFontFromFileTTF("resources/DroidSansMono.ttf", (float)fontSize, &fontConfig, glyphRanges.Data);
    io.Fonts->TexID = 0;
    io.Fonts->Build();
    rlImGuiReloadFonts();
}

void LoadStyle()
{
    LogInfo("Loading style");
    if (style == STYLE_DARK) ImGui::StyleColorsDark();
    if (style == STYLE_LIGHT) ImGui::StyleColorsLight();
    if (style == STYLE_CLASSIC) ImGui::StyleColorsClassic();
}

bool isSettings = false;
void ShowSettings(bool* isOpen)
{
    if (!ImGui::Begin(labels["Settings"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputInt(labels["days per week"].c_str(), &daysPerWeek);
    ImGui::InputInt(labels["lessons per day"].c_str(), &lessonsPerDay);
    if (lessonsPerDay < 1) lessonsPerDay = 1;
    if (ImGui::Combo(labels["style"].c_str(), &style, styleValues.c_str()))
    {
        LoadStyle();
    }
    if (ImGui::Combo(labels["language"].c_str(), &languageID, languageValues.c_str()))
    {
        language = availableLanguages[languageID];
        ReloadLabels();
    }
    if (ImGui::TreeNode(labels["Developer options"].c_str()))
    {
        ImGui::Checkbox(labels["vsync"].c_str(), &vsync);
        ImGui::Checkbox(labels["merged-font"].c_str(), &mergedFont);
        ImGui::SliderInt(labels["timetable-autosave-interval"].c_str(), &timetableAutosaveInterval, 0, 600);
        ImGui::InputInt(labels["font-size"].c_str(), &fontSize);
        if (fontSize < 5) fontSize = 5;
        ImGui::InputInt(labels["max-mutations"].c_str(), &maxMutations);
        if (maxMutations < 1) maxMutations = 1;
        ImGui::SliderFloat(labels["error-bonus-ratio"].c_str(), &errorBonusRatio, 0.1f, 100.0f);
        ImGui::SliderInt(labels["timetables-per-generation"].c_str(), &timetablesPerGeneration, 10, 10000);
        ImGui::SliderInt(labels["max-iterations"].c_str(), &maxIterations, -1, 10000);
        if (ImGui::Checkbox(labels["verbose-logging"].c_str(), &verboseLogging))
        {
            threadsNumber = (verboseLogging ? 1 : std::max(std::thread::hardware_concurrency(), (unsigned int)1));
        }
        ImGui::Checkbox(labels["use-prereleases"].c_str(), &usePrereleases);
        ImGui::TreePop();
    }
    ImGui::End();
}

bool isAbout = false;
void ShowAbout(bool* isOpen)
{
    if (!ImGui::Begin(labels["About"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", (labels["TimetableGenerator"] + " " + version).c_str());
    ImGui::Text("%s", labels["A tool for creating timetables easily"].c_str());
    ImGui::Text("%s", labels["Developed by SemkiShow"].c_str());
    ImGui::Text("%s", labels["Licensed under GPLv3 License"].c_str());
    ImGui::End();
}

bool isNewVersion = false;
void ShowNewVersion(bool* isOpen)
{
    if (!ImGui::Begin(labels["Updates"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", (labels["The latest version is"] + " " + latestVersion).c_str());
    ImGui::Text("%s", (labels["Your version is"] + " " + version).c_str());
    if (version == latestVersion) ImGui::Text("%s", labels["There are no new versions available"].c_str());
    else if (latestVersion != labels["loading..."])
    {
        ImGui::Text("%s", labels["A new version is available!"].c_str());
        if (ImGui::TreeNode(labels["Release notes"].c_str()))
        {
            for (int i = 0; i < releaseNotes.size()-2; i++)
                ImGui::Text("%s", releaseNotes[i].c_str());
            ImGui::TreePop();
        }
        if (downloadStatus != "") ImGui::Text("%s", downloadStatus.c_str());
        if (ImGui::Button(labels["Update"].c_str()))
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
    if (!ImGui::Begin((newTimetable ? labels["New timetable"] : labels["Save timetable as"]).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", labels["Enter the timetable name"].c_str());
    ImGui::Text("%s", labels["(for example, the name of the school)"].c_str());
    ImGui::InputText("##", &timetableName);
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Creating a new timetable at templates/" + timetableName + ".json");
        if (newTimetable) currentTimetable = Timetable();
        SaveTimetable("templates/" + timetableName + ".json", &currentTimetable);
        currentTimetable = Timetable();
        LoadTimetable("templates/" + timetableName + ".json", &currentTimetable);
        SaveTimetable("templates/" + timetableName + ".json", &currentTimetable);
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str())) *isOpen = false;
    ImGui::End();
}

bool isOpenTimetable = false;
void ShowOpenTimetable(bool* isOpen)
{
    if (!ImGui::Begin(labels["Open timetable"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", labels["Select a timetable to open"].c_str());
    for (int i = 0; i < timetableFiles.size(); i++)
    {
        if (ImGui::Button(timetableFiles[i].c_str()))
        {
            LogInfo("Opening a timetable at templates/" + timetableFiles[i] + ".json");
            currentTimetable = Timetable();
            LoadTimetable("templates/" + timetableFiles[i] + ".json", &currentTimetable);
            *isOpen = false;
        }
    }
    ImGui::End();
}

bool isGenerateTimetable = false;
void ShowGenerateTimetable(bool* isOpen)
{
    if (!ImGui::Begin(labels["Generate timetable"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    if (iterationData.isDone)
    {
        ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s", labels["Timetable generating done!"].c_str());
    }
    else
    {
        ImGui::Text("%s", labels["Generating a timetable that matches the requirements..."].c_str());
    }
    ImGui::Text("%s", (labels["Iteration:"] + " " + std::to_string(iterationData.iteration)).c_str());
    ImGui::Text("%s", (labels["The best score is"] + " " + std::to_string(iterationData.allTimeBestScore)).c_str());
    ImGui::Text("%s", (labels["The best timetable has"] + " " +
        std::to_string(iterationData.timetables[iterationData.bestTimetableIndex].errors) + " " + labels["errors"]).c_str());
    ImGui::Text("%s", (labels["The best timetable has"] + " " +
            std::to_string(iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints) + " " + labels["bonus points"]).c_str());
    ImGui::Text("%s", (std::to_string(iterationData.iterationsPerChange) + " " + labels["iterations have passed since last score improvement"]).c_str());
    float progressPercentage = (-(iterationData.maxErrors * 1.0f / 100) * iterationData.minErrors + 100) / 100;
    ImGui::ProgressBar(pow(progressPercentage, 2));
    ImGui::End();
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
    teacherLessonValues = "";
    teacherLessonValues += labels["no lesson"];
    teacherLessonValues += '\0';
    teacherLessonValues += labels["any lesson"];
    teacherLessonValues += '\0';
    for (auto& lesson: currentTimetable.lessons)
    {
        if (lesson.second.name == "")
            teacherLessonValues += labels["error"];
        else
            teacherLessonValues += lesson.second.name;
        teacherLessonValues += '\0';
    }
    teacherLessonValues += '\0';
    tmpTimetable.teachers = currentTimetable.teachers;
    tmpTimetable.maxTeacherID = currentTimetable.maxTeacherID;
    isTeachers = true;
}

void OpenClasses()
{
    LogInfo("Opening classes");
    classTeacherValues = "";
    for (auto& teacher: currentTimetable.teachers)
    {
        if (teacher.second.name == "")
            classTeacherValues += labels["error"];
        else
            classTeacherValues += teacher.second.name;
        classTeacherValues += '\0';
    }
    classTeacherValues += '\0';
    tmpTimetable.classes = currentTimetable.classes;
    tmpTimetable.maxClassID = currentTimetable.maxClassID;
    tmpTimetable.orderedClasses = currentTimetable.orderedClasses;
    isClasses = true;
}

void ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu(labels["File"].c_str()))
        {
            if (ImGui::MenuItem(labels["New"].c_str()))
            {
                LogInfo("Creating a new timatable");
                newTimetable = true;
                timetableName = "";
                isNewTimetable = true;
            }
            if (ImGui::MenuItem(labels["Open"].c_str()))
            {
                LogInfo("Opening a timetable");
                ListFiles("templates/", &timetableFiles);
                for (int i = 0; i < timetableFiles.size(); i++)
                    timetableFiles[i] = std::filesystem::path(timetableFiles[i]).stem().string();
                isOpenTimetable = true;
            }
            if (ImGui::MenuItem(labels["Save"].c_str()))
            {
                LogInfo("Manually saving a timetable");
                SaveTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
            }
            if (ImGui::MenuItem(labels["Save As"].c_str()))
            {
                LogInfo("Saving a timetable as");
                newTimetable = false;
                timetableName = currentTimetable.name;
                isNewTimetable = true;
            }
            if (currentTimetable.name != "" && ImGui::BeginMenu(labels["Export As"].c_str()))
            {
                if (ImGui::MenuItem(labels["Excel"].c_str()))
                {
                    LogInfo("Exporting a timetable as Excel");
                    ExportTimetableAsXlsx(&currentTimetable);
                    OpenInFileManager("timetables/");
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem(labels["Settings"].c_str())) isSettings = true;
            ImGui::EndMenu();
        }
        if (currentTimetable.name != "" && ImGui::BeginMenu(currentTimetable.name.c_str()))
        {
            if (ImGui::MenuItem(labels["Setup wizard"].c_str()))
            {
                LogInfo("Opening the setup wizard");
                isWizard = true;
            }
            if (ImGui::MenuItem(labels["Classrooms"].c_str())) OpenClassrooms();
            if (ImGui::MenuItem(labels["Lessons"].c_str())) OpenLessons();
            if (ImGui::MenuItem(labels["Teachers"].c_str())) OpenTeachers();
            if (ImGui::MenuItem(labels["Classes"].c_str())) OpenClasses();
            if (ImGui::MenuItem(labels["Generate timetable"].c_str())) BeginSearching(&currentTimetable);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(labels["Help"].c_str()))
        {
            if (ImGui::MenuItem(labels["Check for updates"].c_str())) CheckForUpdates();
            if (ImGui::MenuItem(labels["About"].c_str())) isAbout = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    return;
}

void DrawFrame()
{
    BeginDrawing();

    // Begin imgui drawing
    if (lastMergedFont != mergedFont || lastFontSize != fontSize)
    {
        lastMergedFont = mergedFont;
        fontSize = std::max(5, fontSize);
        lastFontSize = fontSize;
        LoadFonts();
    }
    rlImGuiBegin();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts.back());

    // Change the background color based on the style
    if (style == STYLE_DARK || style == STYLE_CLASSIC)
    {
        ClearBackground(BLACK);
    }
    if (style == STYLE_LIGHT)
    {
        ClearBackground(WHITE);
    }

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
    if (isEditClass) ShowEditClass(&isEditClass);
    if (isClasses) ShowClasses(&isClasses);
    if (isAbout) ShowAbout(&isAbout);
    ShowWizard(&isWizard);
    if (isNewVersion) ShowNewVersion(&isNewVersion);
    if (isNewTimetable) ShowNewTimetable(&isNewTimetable);
    if (isOpenTimetable) ShowOpenTimetable(&isOpenTimetable);
    if (isGenerateTimetable) ShowGenerateTimetable(&isGenerateTimetable);
    if (isCrashReport) ShowCrashReport(&isCrashReport);

    // Stop searching for a perfect timetable if the Generate timetable window is closed
    if (!isGenerateTimetable) iterationData.isDone = true;

    // Autosave the timetable
    if (GetTime() - timetableSaveTimer > timetableAutosaveInterval)
    {
        timetableSaveTimer = GetTime();
        SaveTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
    }

    // Change vsync state
    if (lastVsync != vsync)
    {
        lastVsync = vsync;
        if (!vsync) ClearWindowState(FLAG_VSYNC_HINT);
        else SetWindowState(FLAG_VSYNC_HINT);
    }

    // End imgui drawing
    ImGui::PopFont();
    rlImGuiEnd();

    EndDrawing();
}
