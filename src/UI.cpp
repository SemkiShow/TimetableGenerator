#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Updates.hpp"
#include "Searching.hpp"

int menuOffset = 20;
int windowSize[2] = {16*50*2, 9*50*2};
std::unordered_map<int, std::string> weekDays{
    {0, "Monday"},
    {1, "Tuesday"},
    {2, "Wednesday"},
    {3, "Thursday"},
    {4, "Friday"},
    {5, "Saturday"},
    {6, "Sunday"}
};
std::vector<std::string> timetableFiles;

int timetableSaveTimer = GetTime();

bool lastVsync = vsync;
bool lastMergedFont = mergedFont;
int lastFontSize = fontSize;

void LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFont* defaultFont = io.Fonts->AddFontFromFileTTF("resources/ProggyClean.ttf", (float)fontSize);
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.AddText(u8"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ");
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

bool isSettings = false;
void ShowSettings(bool* isOpen)
{
    if (!ImGui::Begin("Settings", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputInt("lessons-per-day", &lessonsPerDay);
    if (lessonsPerDay < 0) lessonsPerDay = 0;
    if (ImGui::TreeNode("Developer options"))
    {
        ImGui::Checkbox("vsync", &vsync);
        ImGui::Checkbox("merged-font", &mergedFont);
        ImGui::DragInt("timetable-autosave-interval", &timetableAutosaveInterval, 1.0f, 0, 600);
        ImGui::InputInt("font-size", &fontSize);
        if (fontSize < 5) fontSize = 5;
        ImGui::InputInt("max-mutations", &maxMutations);
        if (maxMutations < 1) maxMutations = 1;
        ImGui::SliderFloat("error-bonus-ratio", &errorBonusRatio, 0.1f, 100.0f);
        ImGui::TreePop();
    }
    ImGui::End();
}

bool isAbout = false;
void ShowAbout(bool* isOpen)
{
    if (!ImGui::Begin("About", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", ("TimetableGenerator " + version).c_str());
    ImGui::Text("A tool for creating timetables easily");
    ImGui::Text("Developed by SemkiShow");
    ImGui::Text("Licensed under GPLv3 License.");
    ImGui::End();
}

bool isNewVersion = false;
void ShowNewVersion(bool* isOpen)
{
    if (!ImGui::Begin("Updates", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s", ("The latest version is " + latestVersion).c_str());
    ImGui::Text("%s", ("Your version is " + version).c_str());
    if (version == latestVersion) ImGui::Text("There are no new versions available");
    else if (latestVersion != "loading...")
    {
        ImGui::Text("A new version is available!");
        ImGui::Text("Download it using");
        ImGui::SameLine();
        ImGui::TextLinkOpenURL("this link", ("https://github.com/SemkiShow/TimetableGenerator/releases/tag/" + latestVersion).c_str());
        if (ImGui::TreeNode("Release notes"))
        {
            for (int i = 0; i < releaseNotes.size()-1; i++)
                ImGui::Text("%s", releaseNotes[i].c_str());
            ImGui::TreePop();
        }
    }
    ImGui::End();
}

bool isNewTimetable = false;
bool newTimetable = false;
std::string timetableName = "";
void ShowNewTimetable(bool* isOpen)
{
    if (!ImGui::Begin((newTimetable ? "New timetable" : "Save timetable as"), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("Enter the timetable name\n(for example, the name of the school)");
    ImGui::InputText("", &timetableName);
    if (ImGui::Button("Ok"))
    {
        if (newTimetable) currentTimetable = Timetable();
        SaveTimetable("templates/" + timetableName + ".json", &currentTimetable);
        LoadTimetable("templates/" + timetableName + ".json", &currentTimetable);
        SaveTimetable("templates/" + timetableName + ".json", &currentTimetable);
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}

bool isOpenTimetable = false;
void ShowOpenTimetable(bool* isOpen)
{
    if (!ImGui::Begin("Open timetable", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("Select a timetable to open");
    for (int i = 0; i < timetableFiles.size(); i++)
    {
        if (ImGui::Button(timetableFiles[i].c_str()))
        {
            LoadTimetable("templates/" + timetableFiles[i] + ".json", &currentTimetable);
            *isOpen = false;
        }
    }
    ImGui::End();
}

bool isGenerateTimetable = false;
void ShowGenerateTimetable(bool* isOpen)
{
    if (!ImGui::Begin("Generate timetable", isOpen))
    {
        ImGui::End();
        return;
    }
    if (iterationData.isDone)
    {
        ImGui::TextColored(ImVec4(0, 255, 0, 255), "Timetable generating done!");
        // ImGui::End();
        // return;
    }
    else
    {
        ImGui::Text("Generating a timetable that matches the requirements...");
    }
    ImGui::Text("Iteration: %d", iterationData.iteration);
    ImGui::Text("The current best score is %d", iterationData.bestScore);
    ImGui::Text("The best score is %d", iterationData.allTimeBestScore);
    ImGui::Text("The best timetable has %d errors", iterationData.timetables[iterationData.bestTimetableIndex].errors);
    ImGui::Text("The best timetable index is %d", iterationData.bestTimetableIndex);
    ImGui::Text("%d iterations have passed since last score improvement. ", iterationData.iterationsPerChange);
    ImGui::End();
}

void OpenClassrooms()
{
    tmpTimetable.classrooms = currentTimetable.classrooms;
    tmpTimetable.maxClassroomID = currentTimetable.maxClassroomID;
    isClassrooms = true;
}

void OpenLessons()
{
    tmpTimetable.lessons = currentTimetable.lessons;
    tmpTimetable.maxLessonID = currentTimetable.maxLessonID;
    isLessons = true;
}

void OpenTeachers()
{
    teacherLessonValues = "";
    teacherLessonValues += "no lesson";
    teacherLessonValues += '\0';
    teacherLessonValues += "any lesson";
    teacherLessonValues += '\0';
    for (auto& lesson: currentTimetable.lessons)
    {
        if (lesson.second.name == "")
            teacherLessonValues += "error";
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
    classTeacherValues = "";
    for (auto& teacher: currentTimetable.teachers)
    {
        if (teacher.second.name == "")
            classTeacherValues += "test\0";
        classTeacherValues += teacher.second.name + '\0';
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
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
            {
                newTimetable = true;
                timetableName = "";
                isNewTimetable = true;
            }
            if (ImGui::MenuItem("Open"))
            {
                ListFiles("templates/", &timetableFiles);
                for (int i = 0; i < timetableFiles.size(); i++)
                    timetableFiles[i] = std::filesystem::path(timetableFiles[i]).stem().string();
                isOpenTimetable = true;
            }
            if (ImGui::MenuItem("Save"))
                SaveTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
            if (ImGui::MenuItem("Save As"))
            {
                newTimetable = false;
                timetableName = currentTimetable.name;
                isNewTimetable = true;
            }
            if (ImGui::MenuItem("Settings")) isSettings = true;
            ImGui::EndMenu();
        }
        if (currentTimetable.name != "" && ImGui::BeginMenu(currentTimetable.name.c_str()))
        {
            if (ImGui::MenuItem("Setup wizard")) isWizard = true;
            if (ImGui::MenuItem("Classrooms")) OpenClassrooms();
            if (ImGui::MenuItem("Lessons")) OpenLessons();
            if (ImGui::MenuItem("Teachers")) OpenTeachers();
            if (ImGui::MenuItem("Classes")) OpenClasses();
            if (ImGui::MenuItem("Generate timetable")) BeginSearching(&currentTimetable);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help"))
        {
            if (ImGui::MenuItem("Check for updates")) CheckForUpdates();
            if (ImGui::MenuItem("About")) isAbout = true;
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    return;
}

void DrawFrame()
{
    BeginDrawing();

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

    ClearBackground(BLACK);

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

    if (!isGenerateTimetable) iterationData.isDone = true;
    if (!iterationData.isDone) RunASearchIteration();

    if (GetTime() - timetableSaveTimer > timetableAutosaveInterval)
    {
        timetableSaveTimer = GetTime();
        SaveTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
    }

    if (lastVsync != vsync)
    {
        lastVsync = vsync;
        if (!vsync) ClearWindowState(FLAG_VSYNC_HINT);
        else SetWindowState(FLAG_VSYNC_HINT);
    }

    ImGui::PopFont();
    rlImGuiEnd();

    EndDrawing();
}
