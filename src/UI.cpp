#include "UI.hpp"
#include "Crashes.hpp"
#include "Logging.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "System.hpp"
#include "Timetable.hpp"
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
int windowSize[2] = {16 * 50 * 2, 9 * 50 * 2};
std::string weekDays[7] = {"Monday", "Tuesday",  "Wednesday", "Thursday",
                           "Friday", "Saturday", "Sunday"};
std::vector<std::string> timetableFiles;

Texture2D* faqScreenshots;

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
    ImFont* defaultFont =
        io.Fonts->AddFontFromFileTTF("resources/ProggyClean.ttf", (float)fontSize);
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.AddText(u8"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ’—");
    ImVector<ImWchar> glyphRanges;
    builder.BuildRanges(&glyphRanges);
    ImFontConfig fontConfig;
    fontConfig.MergeMode = mergedFont;
    fontConfig.PixelSnapH = true;
    ImFont* font = io.Fonts->AddFontFromFileTTF("resources/DroidSansMono.ttf", (float)fontSize,
                                                &fontConfig, glyphRanges.Data);
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

void LoadFAQScreenshots()
{
    std::vector<std::string> faqScreenshotFiles;
    ListFiles("resources/faq-screenshots", &faqScreenshotFiles);
    faqScreenshots = new Texture2D[faqScreenshotFiles.size()];
    for (int i = 0; i < faqScreenshotFiles.size(); i++)
    {
        faqScreenshots[i] = LoadTexture(faqScreenshotFiles[i].c_str());
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
    ResetTeacherLessonValues();
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
    if (!ImGui::Begin(labels["Settings"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputInt(labels["days per week"].c_str(), &daysPerWeek);
    if (daysPerWeek < 1) daysPerWeek = 1;
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
        ImGui::SliderInt(labels["timetable-autosave-interval"].c_str(), &timetableAutosaveInterval,
                         0, 600);
        ImGui::InputInt(labels["font-size"].c_str(), &fontSize);
        if (fontSize < 5) fontSize = 5;
        ImGui::SliderFloat(labels["error-bonus-ratio"].c_str(), &errorBonusRatio, 0.1f, 100.0f);
        ImGui::SliderInt(labels["timetables-per-generation-step"].c_str(),
                         &timetablesPerGenerationStep, 1, 100);
        ImGui::SliderInt(labels["min-timetables-per-generation"].c_str(),
                         &minTimetablesPerGeneration, 10, 10000);
        ImGui::SliderInt(labels["max-timetables-per-generation"].c_str(),
                         &maxTimetablesPerGeneration, 10, 10000);
        if (maxTimetablesPerGeneration < minTimetablesPerGeneration)
            maxTimetablesPerGeneration = minTimetablesPerGeneration;
        ImGui::SliderInt(labels["max-iterations"].c_str(), &maxIterations, -1, 10000);
        ImGui::SliderInt(labels["additional-bonus-points"].c_str(), &additionalBonusPoints, 0, 100);
        if (ImGui::Checkbox(labels["verbose-logging"].c_str(), &verboseLogging))
        {
            while (iterationData.threadLock)
                COMPILER_BARRIER();
            iterationData.threadLock = true;
            threadsNumber =
                (verboseLogging ? 1
                                : std::max(std::thread::hardware_concurrency(), (unsigned int)1));
            iterationData.threadLock = false;
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
    if (version == latestVersion)
        ImGui::Text("%s", labels["There are no new versions available"].c_str());
    else if (latestVersion != labels["loading..."])
    {
        ImGui::Text("%s", labels["A new version is available!"].c_str());
        if (ImGui::TreeNode(labels["Release notes"].c_str()))
        {
            for (int i = 0; i < (int)releaseNotes.size() - 2; i++)
            {
                ImGui::Text("%s", releaseNotes[i].c_str());
            }
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
    if (!ImGui::Begin(
            (newTimetable ? labels["New timetable"] : labels["Save timetable as"]).c_str(), isOpen))
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
        OpenWizard();
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

bool wasGenerateTimetable = false;
bool isGenerateTimetable = false;
void ShowGenerateTimetable(bool* isOpen)
{
    if (!ImGui::Begin(labels["Generate timetable"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    if (generateTimetableStatus == labels["Timetable generating done!"])
    {
        ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s",
                           labels["Timetable generating done!"].c_str());
    }
    else
    {
        ImGui::Text("%s", generateTimetableStatus.c_str());
    }
    if (generateTimetableStatus == labels["Allocating memory for the timetables..."])
    {
        ImGui::LabelText("##1", "\n\n\n\n\n\n\n");
    }
    else
    {
        ImGui::Text("%s",
                    (labels["Iteration:"] + " " + std::to_string(iterationData.iteration)).c_str());
        ImGui::Text("%s", (labels["The best score is"] + " " +
                           std::to_string(iterationData.allTimeBestScore))
                              .c_str());
        ImGui::Text("%s", (labels["The best timetable has"] + " " +
                           std::to_string(
                               iterationData.timetables[iterationData.bestTimetableIndex].errors) +
                           " " + labels["errors"])
                              .c_str());
        ImGui::Text("%s",
                    (labels["The best timetable has"] + " " +
                     std::to_string(
                         iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints) +
                     " " + labels["bonus points"])
                        .c_str());
        ImGui::Text("%s", (std::to_string(iterationData.iterationsPerChange) + " " +
                           labels["iterations have passed since last score improvement"])
                              .c_str());
        float progressPercentage = 1;
        if (generateTimetableStatus ==
            labels["Generating a timetable that matches the requirements..."])
        {
            progressPercentage =
                (-(iterationData.maxErrors * 1.0f / 100) * iterationData.minErrors + 100) / 100;
        }
        else if (generateTimetableStatus == labels["Finding additional bonus points..."])
        {
            progressPercentage = (iterationData.maxBonusPoints - iterationData.startBonusPoints) *
                                 1.0f / additionalBonusPoints;
        }
        ImGui::ProgressBar(pow(progressPercentage, 2));
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
    if (!ImGui::Begin(labels["FAQ"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::TreeNode(labels["How do I contact the developer?"].c_str()))
    {
        ImGui::Text(
            "%s",
            labels["You can contact me by sending an email to mgdeveloper123@gmail.com"].c_str());
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(labels["How do I add multiple lessons to one timetable cell?"].c_str()))
    {
        ImGui::Text("%s", labels["To add multiple lessons to one timetable cell, click"].c_str());
        ImGui::Text("%s", labels["Combine lessons while editing a class."].c_str());
        DrawImage(faqScreenshots[0]);
        ImGui::Text("%s",
                    labels["Select the lessons and teachers to combine and press Ok."].c_str());
        DrawImage(faqScreenshots[1]);
        ImGui::Text(
            "%s", labels["Then set the amount per week for the created combined lesson."].c_str());
        DrawImage(faqScreenshots[2]);
        ImGui::TreePop();
    }
    ImGui::End();
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
            if (ImGui::MenuItem(labels["Setup wizard"].c_str())) OpenWizard();
            if (ImGui::MenuItem(labels["Classrooms"].c_str())) OpenClassrooms();
            if (ImGui::MenuItem(labels["Lessons"].c_str())) OpenLessons();
            if (ImGui::MenuItem(labels["Teachers"].c_str())) OpenTeachers();
            if (ImGui::MenuItem(labels["Classes"].c_str())) OpenClasses();
            if (ImGui::MenuItem(labels["Generate timetable"].c_str()))
            {
                if (iterationData.timetables != nullptr)
                {
                    iterationData.isDone = true;
                    generateTimetableStatus = labels["Allocating memory for the timetables..."];
                    while (iterationData.threadLock)
                        COMPILER_BARRIER();
                    StopSearching();
                }
                while (iterationData.threadLock)
                    COMPILER_BARRIER();
                std::thread beginSearchingThread(BeginSearching, &currentTimetable);
                beginSearchingThread.detach();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(labels["Help"].c_str()))
        {
            if (ImGui::MenuItem(labels["FAQ"].c_str())) isFAQ = true;
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
    if (isFAQ) ShowFAQ(&isFAQ);

    // Stop searching for a perfect timetable if the Generate timetable window is closed
    if (!isGenerateTimetable && wasGenerateTimetable)
    {
        wasGenerateTimetable = false;
        iterationData.isDone = true;
        while (iterationData.threadLock)
            COMPILER_BARRIER();
        std::thread stopSearchingThread(StopSearching);
        stopSearchingThread.detach();
    }

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
