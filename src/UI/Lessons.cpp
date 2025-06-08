#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

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
