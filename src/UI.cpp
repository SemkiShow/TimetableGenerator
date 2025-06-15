#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

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
        ImGui::TreePop();
    }
    ImGui::End();
}

void ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
            {
                IGFD::FileDialogConfig config;
                config.path = "templates";
                config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
                ImGuiFileDialog::Instance()->OpenDialog("New Template", "New File", ".json", config);
            }
            if (ImGui::MenuItem("Open"))
            {
                IGFD::FileDialogConfig config;
                config.path = "templates";
                ImGuiFileDialog::Instance()->OpenDialog("Choose Template", "Choose File", ".json", config);
            }
            if (ImGui::MenuItem("Save"))
                SaveTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
            if (ImGui::MenuItem("Save As"))
            {
                IGFD::FileDialogConfig config;
                config.path = "templates";
                config.flags = ImGuiFileDialogFlags_ConfirmOverwrite;
                ImGuiFileDialog::Instance()->OpenDialog("Save Template As", "Save File As", ".json", config);
            }
            if (ImGui::MenuItem("Settings")) isSettings = true;
            ImGui::EndMenu();
        }
        if (currentTimetable.name != "" && ImGui::BeginMenu(currentTimetable.name.c_str()))
        {
            if (ImGui::MenuItem("Classrooms"))
            {
                tmpTimetable.classrooms = currentTimetable.classrooms;
                tmpTimetable.maxClassroomID = currentTimetable.maxClassroomID;
                isClassrooms = true;
            }
            if (ImGui::MenuItem("Lessons"))
            {
                tmpTimetable.lessons = currentTimetable.lessons;
                tmpTimetable.maxLessonID = currentTimetable.maxLessonID;
                isLessons = true;
            }
            if (ImGui::MenuItem("Teachers"))
            {
                teacherLessonValues = "";
                for (auto& lesson: currentTimetable.lessons)
                {
                    if (lesson.second.name == "")
                        teacherLessonValues += "test\0";
                    teacherLessonValues += lesson.second.name + '\0';
                }
                teacherLessonValues += '\0';
                tmpTimetable.teachers = currentTimetable.teachers;
                tmpTimetable.maxTeacherID = currentTimetable.maxTeacherID;
                isTeachers = true;
            }
            if (ImGui::MenuItem("Classes"))
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
    if (ImGuiFileDialog::Instance()->Display("New Template", ImGuiWindowFlags_NoCollapse, ImVec2(750.f, 500.f)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            currentTimetable = Timetable();
            currentTimetable.lessons[0] = Lesson();
            currentTimetable.lessons[0].name = "no lesson";
            currentTimetable.lessons[1] = Lesson();
            currentTimetable.lessons[1].name = "any lesson";
            SaveTimetable(filePathName, &currentTimetable);
            LoadTimetable(filePathName, &currentTimetable);
        }
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGuiFileDialog::Instance()->Display("Choose Template", ImGuiWindowFlags_NoCollapse, ImVec2(750.f, 500.f)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            SaveTimetable("templates/" + currentTimetable.name + ".json", &currentTimetable);
            currentTimetable = Timetable();
            LoadTimetable(filePathName, &currentTimetable);
            SaveTimetable(filePathName, &currentTimetable);
        }
        ImGuiFileDialog::Instance()->Close();
    }
    if (ImGuiFileDialog::Instance()->Display("Save Template As", ImGuiWindowFlags_NoCollapse, ImVec2(750.f, 500.f)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            SaveTimetable(filePathName, &currentTimetable);
            LoadTimetable(filePathName, &currentTimetable);
        }
        ImGuiFileDialog::Instance()->Close();
    }

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
