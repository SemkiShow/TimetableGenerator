#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

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
