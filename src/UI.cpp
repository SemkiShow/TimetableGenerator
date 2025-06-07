#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

bool isSettings = false;
bool isClassrooms = false;

int menuOffset = 20;
int windowSize[2] = {16*50*2, 9*50*2};

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
            currentTimetable = Timetable();
            LoadTimetable(filePathName, &currentTimetable);
        }
        ImGuiFileDialog::Instance()->Close();
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
                isClassrooms = true;
                ShowClassrooms(&isClassrooms);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    return;
}
