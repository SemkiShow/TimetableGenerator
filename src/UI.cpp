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

bool isSettings = false;
void ShowSettings(bool* isOpen)
{
    if (!ImGui::Begin("Settings", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputInt("lessons-per-day", &lessonsPerDay);
    if (ImGui::TreeNode("Developer options"))
    {
        ImGui::Checkbox("vsync", &vsync);
        ImGui::Checkbox("merged-font", &mergedFont);
        ImGui::DragInt("timetable-autosave-interval", &timetableAutosaveInterval, 1.0f, 0, 600);
        ImGui::TreePop();
    }
    ImGui::End();
}

bool isClassrooms = false;
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

bool isEditLesson = false;
int currentLessonIndex = 0;
bool newLesson = false;
bool allLessonClasses = true;
std::unordered_map<std::string, bool> lessonClassGroups;
std::unordered_map<std::string, bool> lessonClasses;
bool allLessonClassrooms = true;
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
    if (ImGui::Checkbox((allLessonClasses ? "Deselect all##1" : "Select all##1"), &allLessonClasses))
    {
        for (int i = 0; i < tmpTmpTimetable.classes.size(); i++)
        {
            lessonClassGroups[tmpTmpTimetable.classes[i].number] = allLessonClasses;
            lessonClasses[tmpTmpTimetable.classes[i].number + tmpTmpTimetable.classes[i].letter] = allLessonClasses;
        }
    }
    if (tmpTmpTimetable.classes.size() == 0) ImGui::TextColored(ImVec4(255, 0, 0, 255), "You need to add classes\nin the Classes menu\nto select classes for this lesson!");
    std::string lastClassNumber = "";
    for (int i = 0; i < tmpTmpTimetable.classes.size(); i++)
    {
        if (lastClassNumber != tmpTmpTimetable.classes[i].number)
        {
            lastClassNumber = tmpTmpTimetable.classes[i].number;
            if (ImGui::Checkbox(tmpTmpTimetable.classes[i].number.c_str(), &lessonClassGroups[tmpTmpTimetable.classes[i].number]))
            {
                for (int j = 0; j < tmpTmpTimetable.classes.size(); j++)
                {
                    if (tmpTmpTimetable.classes[j].number == tmpTmpTimetable.classes[i].number)
                        lessonClasses[tmpTmpTimetable.classes[j].number + tmpTmpTimetable.classes[j].letter] =
                            lessonClassGroups[tmpTmpTimetable.classes[i].number];
                }
            }
        }
        ImGui::Indent();
        ImGui::Checkbox((tmpTmpTimetable.classes[i].number + tmpTmpTimetable.classes[i].letter).c_str(),
            &lessonClasses[tmpTmpTimetable.classes[i].number + tmpTmpTimetable.classes[i].letter]);
        ImGui::Unindent();
    }
    ImGui::NextColumn();
    ImGui::Text("classrooms");
    if (ImGui::Checkbox((allLessonClassrooms ? "Deselect all##2" : "Select all##2"), &allLessonClassrooms))
    {
        for (int i = 0; i < tmpTmpTimetable.classrooms.size(); i++)
            lessonClassrooms[tmpTmpTimetable.classrooms[i].name] = allLessonClassrooms;
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
        tmpTimetable = tmpTmpTimetable;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        if (newLesson) tmpTimetable.lessons.pop_back();
        tmpTmpTimetable = tmpTimetable;
        *isOpen = false;
    }
    ImGui::End();
}

bool isLessons = false;
void ShowLessons(bool* isOpen)
{
    if (!ImGui::Begin("Lessons", isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::Button("+"))
    {
        allLessonClasses = allLessonClassrooms = true;
        int maxLessonID = 0;
        for (int i = 0; i < tmpTimetable.lessons.size(); i++)
        {
            if (stoi(tmpTimetable.lessons[i].id) > maxLessonID)
                maxLessonID = stoi(tmpTimetable.lessons[i].id);
        }
        tmpTimetable.lessons.push_back(Lesson());
        currentLessonIndex = tmpTimetable.lessons.size()-1;
        tmpTimetable.lessons[currentLessonIndex].id = std::to_string(maxLessonID+1);
        newLesson = true;
        lessonClassGroups.clear();
        lessonClasses.clear();
        for (int i = 0; i < tmpTimetable.classes.size(); i++)
        {
            lessonClassGroups[tmpTimetable.classes[i].number] = true;
            lessonClasses[tmpTimetable.classes[i].number + tmpTimetable.classes[i].letter] = true;
        }
        lessonClassrooms.clear();
        for (int i = 0; i < tmpTimetable.classrooms.size(); i++)
            lessonClassrooms[tmpTimetable.classrooms[i].name] = true;
        tmpTmpTimetable = tmpTimetable;
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
            tmpTmpTimetable = tmpTimetable;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            allLessonClasses = allLessonClassrooms = true;
            currentLessonIndex = i;
            newLesson = false;
            lessonClassGroups.clear();
            lessonClasses.clear();
            for (int i = 0; i < tmpTimetable.classes.size(); i++)
            {
                lessonClassGroups[tmpTimetable.classes[i].number] = true;
                lessonClasses[tmpTimetable.classes[i].number + tmpTimetable.classes[i].letter] = false;
            }
            for (int j = 0; j < tmpTimetable.lessons[i].classNames.size(); j++)
                lessonClasses[tmpTimetable.lessons[i].classNames[j]] = true;
            lessonClassrooms.clear();
            for (int j = 0; j < tmpTimetable.classrooms.size(); j++)
                lessonClassrooms[tmpTimetable.classrooms[j].name] = false;
            for (int j = 0; j < tmpTimetable.lessons[i].classrooms.size(); j++)
                lessonClassrooms[tmpTimetable.lessons[i].classrooms[j]->name] = true;
            tmpTmpTimetable = tmpTimetable;
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
        currentTimetable = tmpTimetable;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}

bool isEditTeacher = false;
int currentTeacherIndex = 0;
bool newTeacher = false;
bool allTeacherLessons = true;
std::unordered_map<std::string, bool> teacherLessons;
bool allAvailableTeacherLessonsVertical[7];
std::vector<bool> allAvailableTeacherLessonsHorizontal;
std::unordered_map<int, bool> availableTeacherLessons;
void ShowEditTeacher(bool* isOpen)
{
    if (!ImGui::Begin(((newTeacher ? "New" : "Edit") + std::string(" Teacher")).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputText("name", &tmpTmpTimetable.teachers[currentTeacherIndex].name);
    ImGui::Separator();
    ImGui::Text("lessons");
    if (ImGui::Checkbox((allTeacherLessons ? "Deselect all##1" : "Select all##1"), &allTeacherLessons))
    {
        for (int i = 0; i < tmpTmpTimetable.lessons.size(); i++)
            teacherLessons[tmpTmpTimetable.lessons[i].id] = allTeacherLessons;
    }
    if (tmpTmpTimetable.lessons.size() == 0) ImGui::TextColored(ImVec4(255, 0, 0, 255), "You need to add lessons\nin the Lessons menu\nto select lessons for this teacher!");
    ImGui::Columns(3);
    for (int i = 0; i < tmpTmpTimetable.lessons.size(); i++)
    {
        ImGui::PushID(i);
        ImGui::Checkbox("", &teacherLessons[tmpTmpTimetable.lessons[i].id]);
        ImGui::SameLine();
        ImGui::Text(tmpTmpTimetable.lessons[i].name.c_str());
        ImGui::NextColumn();
        std::string classNames = "";
        for (int j = 0; j < tmpTmpTimetable.lessons[i].classNames.size(); j++)
        {
            classNames += tmpTmpTimetable.lessons[i].classNames[j];
            if (j < tmpTmpTimetable.lessons[i].classNames.size()-1) classNames += ' ';
        }
        ImGui::Text(classNames.c_str());
        ImGui::NextColumn();
        std::string lessonClassrooms = "";
        for (int j = 0; j < tmpTmpTimetable.lessons[i].classrooms.size(); j++)
        {
            lessonClassrooms += tmpTmpTimetable.lessons[i].classrooms[j]->name;
            if (j < tmpTmpTimetable.lessons[i].classrooms.size()-1) lessonClassrooms += ' ';
        }
        ImGui::Text(lessonClassrooms.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Text("available-lessons");
    ImGui::Columns(8);
    ImGui::LabelText("##1", "");
    ImGui::LabelText("##2", "");
    for (int i = 0; i < lessonsPerDay; i++)
    {
        ImGui::PushID(i+3);
        bool availableTeacherLessonsHorizontal = allAvailableTeacherLessonsHorizontal[i];
        if (ImGui::Checkbox(std::to_string(i).c_str(), &availableTeacherLessonsHorizontal))
        {
            allAvailableTeacherLessonsHorizontal[i] = availableTeacherLessonsHorizontal;
            for (int j = 0; j < 7; j++)
            {
                for (int k = 0; k < tmpTmpTimetable.teachers[currentTeacherIndex].workDays[j].lessonNumbers.size(); k++)
                {
                    if (tmpTmpTimetable.teachers[currentTeacherIndex].workDays[j].lessonNumbers[k] == i)
                    {
                        tmpTmpTimetable.teachers[currentTeacherIndex].workDays[j].lessonNumbers.erase(
                            tmpTmpTimetable.teachers[currentTeacherIndex].workDays[j].lessonNumbers.begin() + k);
                        availableTeacherLessons[j*lessonsPerDay+i] = availableTeacherLessonsHorizontal;
                    }
                }
                if (availableTeacherLessonsHorizontal)
                {
                    tmpTmpTimetable.teachers[currentTeacherIndex].workDays[j].lessonNumbers.push_back(i);
                    availableTeacherLessons[j*lessonsPerDay+i] = availableTeacherLessonsHorizontal;
                }
            }
        }
        ImGui::PopID();
    }
    ImGui::NextColumn();
    for (int i = 0; i < 7; i++)
    {
        ImGui::Text(weekDays[i].c_str());
        ImGui::PushID(i*(lessonsPerDay+1)+7+tmpTmpTimetable.lessons.size());
        if (ImGui::Checkbox((allAvailableTeacherLessonsVertical[i] ? "Deselect all" : "Select all"), &allAvailableTeacherLessonsVertical[i]))
        {
            for (int j = 0; j < lessonsPerDay; j++)
                availableTeacherLessons[i*lessonsPerDay+j] = allAvailableTeacherLessonsVertical[i];
        }
        ImGui::PopID();
        for (int j = 0; j < lessonsPerDay; j++)
        {
            ImGui::PushID(i*(lessonsPerDay+1)+j+tmpTmpTimetable.lessons.size());
            ImGui::Checkbox("", &availableTeacherLessons[i*lessonsPerDay+j]);
            ImGui::PopID();
        }
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    if (ImGui::Button("Ok"))
    {
        tmpTmpTimetable.teachers[currentTeacherIndex].lessons.clear();
        for (int i = 0; i < tmpTmpTimetable.lessons.size(); i++)
        {
            if (teacherLessons[tmpTmpTimetable.lessons[i].id])
                tmpTmpTimetable.teachers[currentTeacherIndex].lessons.push_back(&tmpTmpTimetable.lessons[i]);
        }
        for (int i = 0; i < 7; i++)
        {
            tmpTmpTimetable.teachers[currentTeacherIndex].workDays[i].lessonNumbers.clear();
            for (int j = 0; j < lessonsPerDay; j++)
            {
                if (availableTeacherLessons[i*lessonsPerDay+j])
                    tmpTmpTimetable.teachers[currentTeacherIndex].workDays[i].lessonNumbers.push_back(j);
            }
        }
        tmpTimetable = tmpTmpTimetable;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        if (newTeacher) tmpTimetable.teachers.pop_back();
        tmpTmpTimetable = tmpTimetable;
        *isOpen = false;
    }
    ImGui::End();
}

bool isTeachers = false;
void ShowTeachers(bool* isOpen)
{
    if (!ImGui::Begin("Teachers", isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::Button("+"))
    {
        allTeacherLessons = false;
        for (int j = 0; j < 7; j++)
            allAvailableTeacherLessonsVertical[j] = true;
        allAvailableTeacherLessonsHorizontal.clear();
        for (int j = 0; j < lessonsPerDay; j++)
            allAvailableTeacherLessonsHorizontal.push_back(true);
        tmpTimetable.teachers.push_back(Teacher());
        currentTeacherIndex = tmpTimetable.teachers.size()-1;
        newTeacher = true;
        teacherLessons.clear();
        for (int i = 0; i < tmpTimetable.lessons.size(); i++)
            teacherLessons[tmpTimetable.lessons[i].id] = false;
        availableTeacherLessons.clear();
        for (int i = 0; i < 7; i++)
        {
            tmpTimetable.teachers[currentTeacherIndex].workDays[i].lessonNumbers.clear();
            for (int j = 0; j < lessonsPerDay; j++)
            {
                availableTeacherLessons[i*lessonsPerDay+j] = true;
                tmpTimetable.teachers[currentTeacherIndex].workDays[i].lessonNumbers.push_back(j);
            }
        }
        tmpTmpTimetable = tmpTimetable;
        isEditTeacher = true;
    }
    ImGui::Separator();
    ImGui::Columns(2);
    for (int i = 0; i < tmpTimetable.teachers.size(); i++)
    {
        ImGui::PushID(i);
        if (ImGui::Button("-"))
        {
            tmpTimetable.teachers.erase(tmpTimetable.teachers.begin() + i);
            if (i >= tmpTimetable.teachers.size())
            {
                ImGui::PopID();
                continue;
            }
            tmpTmpTimetable = tmpTimetable;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            allTeacherLessons = false;
            for (int j = 0; j < 7; j++)
                allAvailableTeacherLessonsVertical[j] = true;
            allAvailableTeacherLessonsHorizontal.clear();
            for (int j = 0; j < lessonsPerDay; j++)
                allAvailableTeacherLessonsHorizontal.push_back(true);
            currentTeacherIndex = i;
            newTeacher = false;
            teacherLessons.clear();
            for (int j = 0; j < tmpTimetable.lessons.size(); j++)
                teacherLessons[tmpTimetable.lessons[j].id] = false;
            for (int j = 0; j < tmpTimetable.teachers[i].lessons.size(); j++)
                teacherLessons[tmpTimetable.teachers[i].lessons[j]->id] = true;
            availableTeacherLessons.clear();
            for (int j = 0; j < 7; j++)
            {
                for (int k = 0; k < lessonsPerDay; k++)
                    availableTeacherLessons[j*lessonsPerDay+k] = false;
            }
            for (int j = 0; j < 7; j++)
            {
                for (int k = 0; k < tmpTimetable.teachers[currentTeacherIndex].workDays[j].lessonNumbers.size(); k++)
                    availableTeacherLessons[j*lessonsPerDay + tmpTimetable.teachers[currentTeacherIndex].workDays[j].lessonNumbers[k]] = true;
            }
            tmpTmpTimetable = tmpTimetable;
            isEditTeacher = true;
        }
        ImGui::SameLine();
        ImGui::Text(tmpTimetable.teachers[i].name.c_str());
        ImGui::NextColumn();
        std::string lessonNames = "";
        for (int j = 0; j < tmpTimetable.teachers[i].lessons.size(); j++)
        {
            lessonNames += tmpTimetable.teachers[i].lessons[j]->name;
            if (j < tmpTimetable.teachers[i].lessons.size()-1) lessonNames += ' ';
        }
        ImGui::Text(lessonNames.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (ImGui::Button("Ok"))
    {
        currentTimetable = tmpTimetable;
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
            if (ImGui::MenuItem("Settings")) isSettings = true;
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
            }
            if (ImGui::MenuItem("Lessons"))
            {
                tmpTimetable = currentTimetable;
                isLessons = true;
            }
            if (ImGui::MenuItem("Teachers"))
            {
                tmpTimetable = currentTimetable;
                isTeachers = true;
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
    if (isTeachers) ShowTeachers(&isTeachers);
    if (isEditTeacher) ShowEditTeacher(&isEditTeacher);
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
            SaveTimetable(filePathName, &currentTimetable);
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
