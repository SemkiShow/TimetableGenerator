#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

bool isSettings = false;
bool isClassrooms = false;
bool isLessons = false;
bool isEditLesson = false;

int menuOffset = 20;
int windowSize[2] = {16*50*2, 9*50*2};

int timetableSaveTimer = GetTime();

bool lastVsync = vsync;
bool lastMergedFont = mergedFont;

void LoadFonts()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontDefault();
    ImFontGlyphRangesBuilder builder;
    builder.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    builder.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    builder.AddText(u8"ąćęłńóśźżĄĆĘŁŃÓŚŹŻ");
    ImVector<ImWchar> glyphRanges;
    builder.BuildRanges(&glyphRanges);
    ImFontConfig fontConfig;
    fontConfig.MergeMode = mergedFont;
    fontConfig.PixelSnapH = true;
    ImFont* font = io.Fonts->AddFontFromFileTTF("resources/DroidSansMono.ttf", 13.0f, &fontConfig, glyphRanges.Data);
    io.Fonts->TexID = 0;
    io.Fonts->Build();
    rlImGuiReloadFonts();
}

void ShowSettings(bool* isOpen)
{
    if (!ImGui::Begin("Settings", isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::TreeNode("Developer options"))
    {
        ImGui::Checkbox("vsync", &vsync);
        ImGui::Checkbox("merged-font", &mergedFont);
        ImGui::DragInt("timetable-autosave-interval", &timetableAutosaveInterval, 1.0f, 0, 600);
        ImGui::TreePop();
    }
    ImGui::End();
}

std::string classrooms = "";
void ShowClassrooms(bool* isOpen)
{
    if (!ImGui::Begin("Classrooms", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputTextMultiline("##", &classrooms);
    if (ImGui::Button("Ok"))
    {
        currentTimetable.classrooms.clear();
        std::vector<std::string> classroomsVector = Split(classrooms, '\n');
        for (int i = 0; i < classroomsVector.size(); i++)
        {
            if (classroomsVector[i] == "") continue;
            currentTimetable.classrooms.push_back(Classroom());
            currentTimetable.classrooms[currentTimetable.classrooms.size()-1].name = classroomsVector[i];
        }
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}

int currentLessonIndex = 0;
bool newLesson = false;
bool allClasses = true;
std::unordered_map<std::string, bool> lessonClasses;
bool allClassrooms = true;
std::unordered_map<std::string, bool> lessonClassrooms;
void ShowEditLesson(bool* isOpen)
{
    if (!ImGui::Begin(((newLesson ? "New" : "Edit") + std::string(" Lesson")).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputText("name", &tmpTmpTimetable.lessons[currentLessonIndex].name);
    ImGui::Columns(2);
    ImGui::Text("classes");
    if (ImGui::Checkbox((allClasses ? "Deselect all##1" : "Select all##1"), &allClasses))
    {
        for (int i = 0; i < tmpTmpTimetable.classes.size(); i++)
            lessonClasses[tmpTmpTimetable.classes[i].number + tmpTmpTimetable.classes[i].letter] = allClasses;
    }
    if (tmpTmpTimetable.classes.size() == 0) ImGui::TextColored(ImVec4(255, 0, 0, 255), "You need to add classes\nin the Classes menu\nto select classes for this lesson!");
    for (int i = 0; i < tmpTmpTimetable.classes.size(); i++)
        ImGui::Checkbox((tmpTmpTimetable.classes[i].number + tmpTmpTimetable.classes[i].letter).c_str(),
            &lessonClasses[tmpTmpTimetable.classes[i].number + tmpTmpTimetable.classes[i].letter]);
    ImGui::NextColumn();
    ImGui::Text("classrooms");
    if (ImGui::Checkbox((allClassrooms ? "Deselect all##2" : "Select all##2"), &allClassrooms))
    {
        for (int i = 0; i < tmpTmpTimetable.classrooms.size(); i++)
            lessonClassrooms[tmpTmpTimetable.classrooms[i].name] = allClassrooms;
    }
    if (tmpTmpTimetable.classrooms.size() == 0) ImGui::TextColored(ImVec4(255, 0, 0, 255), "You need to add classrooms\nin the Classrooms menu\nto select classrooms for this lesson!");
    for (int i = 0; i < tmpTmpTimetable.classrooms.size(); i++)
        ImGui::Checkbox(tmpTmpTimetable.classrooms[i].name.c_str(), &lessonClassrooms[tmpTmpTimetable.classrooms[i].name]);
    ImGui::NextColumn();
    ImGui::Columns(1);
    if (ImGui::Button("Ok"))
    {
        tmpTmpTimetable.lessons[currentLessonIndex].classNames.clear();
        for (int i = 0; i < tmpTmpTimetable.classes.size(); i++)
        {
            if (lessonClasses[tmpTmpTimetable.classes[i].number + tmpTmpTimetable.classes[i].letter])
                tmpTmpTimetable.lessons[currentLessonIndex].classNames.push_back(tmpTmpTimetable.classes[i].number + tmpTmpTimetable.classes[i].letter);
        }
        tmpTmpTimetable.lessons[currentLessonIndex].classrooms.clear();
        for (int i = 0; i < tmpTmpTimetable.classrooms.size(); i++)
        {
            if (lessonClassrooms[tmpTmpTimetable.classrooms[i].name])
                tmpTmpTimetable.lessons[currentLessonIndex].classrooms.push_back(&tmpTmpTimetable.classrooms[i]);
        }
        if (newLesson) tmpTimetable.lessons.push_back(Lesson());
        tmpTimetable.lessons[currentLessonIndex] = tmpTmpTimetable.lessons[currentLessonIndex];
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        if (newLesson) tmpTmpTimetable.lessons.pop_back();
        else tmpTmpTimetable.lessons[currentLessonIndex] = tmpTimetable.lessons[currentLessonIndex];
        *isOpen = false;
    }
    ImGui::End();
}

void ShowLessons(bool* isOpen)
{
    if (!ImGui::Begin("Lessons", isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::Button("+"))
    {
        allClasses = allClassrooms = true;
        tmpTimetable.lessons.push_back(Lesson());
        currentLessonIndex = tmpTimetable.lessons.size()-1;
        newLesson = true;
        lessonClasses.clear();
        for (int i = 0; i < tmpTimetable.classes.size(); i++)
            lessonClasses[tmpTimetable.classes[i].number + tmpTimetable.classes[i].letter] = true;
        lessonClassrooms.clear();
        for (int i = 0; i < tmpTimetable.classrooms.size(); i++)
            lessonClassrooms[tmpTimetable.classrooms[i].name] = true;
        tmpTmpTimetable.lessons = tmpTimetable.lessons;
        isEditLesson = true;
    }
    ImGui::Separator();
    ImGui::Columns(3);
    for (int i = 0; i < tmpTimetable.lessons.size(); i++)
    {
        ImGui::PushID(i);
        if (ImGui::Button("-"))
        {
            tmpTimetable.lessons.erase(tmpTimetable.lessons.begin() + i);
            if (i >= tmpTimetable.lessons.size())
            {
                ImGui::PopID();
                continue;
            }
            tmpTmpTimetable.lessons = tmpTimetable.lessons;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            allClasses = allClassrooms = true;
            currentLessonIndex = i;
            newLesson = false;
            lessonClasses.clear();
            for (int j = 0; j < tmpTmpTimetable.classes.size(); j++)
                lessonClasses[tmpTmpTimetable.classes[j].number + tmpTmpTimetable.classes[j].letter] = false;
            for (int j = 0; j < tmpTmpTimetable.lessons[i].classNames.size(); j++)
                lessonClasses[tmpTmpTimetable.lessons[i].classNames[j]] = true;
            lessonClassrooms.clear();
            for (int j = 0; j < tmpTmpTimetable.classrooms.size(); j++)
                lessonClassrooms[tmpTmpTimetable.classrooms[j].name] = false;
            for (int j = 0; j < tmpTmpTimetable.lessons[i].classrooms.size(); j++)
                lessonClassrooms[tmpTmpTimetable.lessons[i].classrooms[j]->name] = true;
            tmpTmpTimetable.lessons = tmpTimetable.lessons;
            isEditLesson = true;
        }
        ImGui::SameLine();
        ImGui::Text(tmpTimetable.lessons[i].name.c_str());
        ImGui::NextColumn();
        std::string classNames = "";
        for (int j = 0; j < tmpTimetable.lessons[i].classNames.size(); j++)
        {
            classNames += tmpTimetable.lessons[i].classNames[j];
            if (j < tmpTimetable.lessons[i].classNames.size()-1) classNames += ' ';
        }
        ImGui::Text(classNames.c_str());
        ImGui::NextColumn();
        std::string lessonClassrooms = "";
        for (int j = 0; j < tmpTimetable.lessons[i].classrooms.size(); j++)
        {
            lessonClassrooms += tmpTimetable.lessons[i].classrooms[j]->name;
            if (j < tmpTimetable.lessons[i].classrooms.size()-1) lessonClassrooms += ' ';
        }
        ImGui::Text(lessonClassrooms.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (ImGui::Button("Ok"))
    {
        currentTimetable.lessons = tmpTimetable.lessons;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
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
            if (ImGui::MenuItem("Settings"))
            {
                isSettings = true;
                ShowSettings(&isSettings);
            }
            ImGui::EndMenu();
        }
        if (currentTimetable.name != "" && ImGui::BeginMenu(currentTimetable.name.c_str()))
        {
            if (ImGui::MenuItem("Classrooms"))
            {
                classrooms = "";
                for (int i = 0; i < currentTimetable.classrooms.size(); i++)
                {
                    classrooms += currentTimetable.classrooms[i].name;
                    if (i < currentTimetable.classrooms.size()-1) classrooms += "\n";
                }
                isClassrooms = true;
                ShowClassrooms(&isClassrooms);
            }
            if (ImGui::MenuItem("Lessons"))
            {
                tmpTimetable.lessons = currentTimetable.lessons;
                isLessons = true;
                ShowLessons(&isLessons);
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

    if (lastMergedFont != mergedFont)
    {
        lastMergedFont = mergedFont;
        LoadFonts();
    }
    rlImGuiBegin();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushFont(io.Fonts->Fonts.back());

    ClearBackground(BLACK);

    ShowMenuBar();
    if (isSettings) ShowSettings(&isSettings);
    if (isClassrooms) ShowClassrooms(&isClassrooms);
    if (isLessons) ShowLessons(&isLessons);
    if (isEditLesson) ShowEditLesson(&isEditLesson);
    if (ImGuiFileDialog::Instance()->Display("New Template", ImGuiWindowFlags_NoCollapse, ImVec2(750.f, 500.f)))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
        {
            std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
            currentTimetable = Timetable();
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
