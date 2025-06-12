#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

int currentTeacherID = 0;
bool newTeacher = false;
bool allTeacherLessons = true;
std::unordered_map<int, bool> teacherLessons;
bool allAvailableTeacherLessonsVertical[7];
std::vector<bool> allAvailableTeacherLessonsHorizontal;
std::unordered_map<int, bool> availableTeacherLessons;

static void ResetVariables()
{
    allTeacherLessons = false;
    for (int i = 0; i < 7; i++)
        allAvailableTeacherLessonsVertical[i] = true;
    allAvailableTeacherLessonsHorizontal.clear();
    for (int i = 0; i < lessonsPerDay; i++)
        allAvailableTeacherLessonsHorizontal.push_back(true);
    if (newTeacher)
    {
        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
                tmpTimetable.teachers[currentTeacherID].workDays[i].lessonNumbers.push_back(j);
        }
    }
    teacherLessons.clear();
    for (auto& lesson: currentTimetable.lessons)
        teacherLessons[lesson.first] = false;
    for (int i = 0; i < tmpTimetable.teachers[currentTeacherID].lessonIDs.size(); i++)
        teacherLessons[tmpTimetable.teachers[currentTeacherID].lessonIDs[i]] = true;
    availableTeacherLessons.clear();
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < lessonsPerDay; j++)
            availableTeacherLessons[i*lessonsPerDay+j] = newTeacher;
    }
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < tmpTimetable.teachers[currentTeacherID].workDays[i].lessonNumbers.size(); j++)
            availableTeacherLessons[i*lessonsPerDay + tmpTimetable.teachers[currentTeacherID].workDays[i].lessonNumbers[j]] = true;
    }
}

bool isEditTeacher = false;
void ShowEditTeacher(bool* isOpen)
{
    if (!ImGui::Begin(((newTeacher ? "New" : "Edit") + std::string(" Teacher")).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::InputText("name", &tmpTmpTimetable.teachers[currentTeacherID].name);
    ImGui::Separator();
    ImGui::Text("lessons");
    if (ImGui::Checkbox((allTeacherLessons ? "Deselect all##1" : "Select all##1"), &allTeacherLessons))
    {
        for (auto& lesson: currentTimetable.lessons)
            teacherLessons[lesson.first] = allTeacherLessons;
    }
    if (currentTimetable.lessons.size() == 0) ImGui::TextColored(ImVec4(255, 0, 0, 255), "You need to add lessons\nin the Lessons menu\nto select lessons for this teacher!");
    ImGui::Columns(3);
    for (auto& lesson: currentTimetable.lessons)
    {
        ImGui::PushID(lesson.first);
        ImGui::Checkbox("", &teacherLessons[lesson.first]);
        ImGui::SameLine();
        ImGui::Text(lesson.second.name.c_str());
        ImGui::NextColumn();
        std::string classNames = "";
        for (int j = 0; j < lesson.second.classIDs.size(); j++)
        {
            classNames += currentTimetable.classes[lesson.second.classIDs[j]].number;
            classNames += currentTimetable.classes[lesson.second.classIDs[j]].letter;
            if (j < lesson.second.classIDs.size()-1) classNames += ' ';
        }
        ImGui::Text(classNames.c_str());
        ImGui::NextColumn();
        std::string lessonClassrooms = "";
        for (int j = 0; j < lesson.second.classroomIDs.size(); j++)
        {
            lessonClassrooms += currentTimetable.classrooms[lesson.second.classroomIDs[j]].name;
            if (j < lesson.second.classroomIDs.size()-1) lessonClassrooms += ' ';
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
                for (int k = 0; k < tmpTmpTimetable.teachers[currentTeacherID].workDays[j].lessonNumbers.size(); k++)
                {
                    if (tmpTmpTimetable.teachers[currentTeacherID].workDays[j].lessonNumbers[k] == i)
                    {
                        tmpTmpTimetable.teachers[currentTeacherID].workDays[j].lessonNumbers.erase(
                            tmpTmpTimetable.teachers[currentTeacherID].workDays[j].lessonNumbers.begin() + k);
                        availableTeacherLessons[j*lessonsPerDay+i] = availableTeacherLessonsHorizontal;
                        k--;
                    }
                }
                if (availableTeacherLessonsHorizontal)
                {
                    tmpTmpTimetable.teachers[currentTeacherID].workDays[j].lessonNumbers.push_back(i);
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
        tmpTmpTimetable.teachers[currentTeacherID].lessonIDs.clear();
        for (auto& lesson: currentTimetable.lessons)
        {
            if (teacherLessons[lesson.first])
                tmpTmpTimetable.teachers[currentTeacherID].lessonIDs.push_back(lesson.first);
        }
        for (int i = 0; i < 7; i++)
        {
            tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonNumbers.clear();
            for (int j = 0; j < lessonsPerDay; j++)
            {
                if (availableTeacherLessons[i*lessonsPerDay+j])
                    tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonNumbers.push_back(j);
            }
        }
        tmpTimetable.teachers = tmpTmpTimetable.teachers;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel"))
    {
        if (newTeacher)
        {
            tmpTimetable.teachers.erase(currentTeacherID);
            tmpTimetable.maxTeacherID--;
        }
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
        tmpTimetable.maxTeacherID++;
        tmpTimetable.teachers[tmpTimetable.maxTeacherID] = Teacher();
        currentTeacherID = tmpTimetable.maxTeacherID;
        newTeacher = true;
        ResetVariables();
        tmpTmpTimetable.teachers = tmpTimetable.teachers;
        isEditTeacher = true;
    }
    ImGui::Separator();
    ImGui::Columns(2);
    for (auto it = tmpTimetable.teachers.begin(); it != tmpTimetable.teachers.end();)
    {
        ImGui::PushID(it->first);
        if (ImGui::Button("-"))
        {
            ImGui::PopID();
            it = tmpTimetable.teachers.erase(it);
            continue;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            currentTeacherID = it->first;
            newTeacher = false;
            ResetVariables();
            tmpTmpTimetable.teachers = tmpTimetable.teachers;
            isEditTeacher = true;
        }
        ImGui::SameLine();
        ImGui::Text(it->second.name.c_str());
        ImGui::NextColumn();
        std::string lessonNames = "";
        for (int j = 0; j < tmpTimetable.teachers[it->first].lessonIDs.size(); j++)
        {
            lessonNames += currentTimetable.lessons[tmpTimetable.teachers[it->first].lessonIDs[j]].name;
            if (j < tmpTimetable.teachers[it->first].lessonIDs.size()-1) lessonNames += ' ';
        }
        ImGui::Text(lessonNames.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
        it++;
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (ImGui::Button("Ok"))
    {
        currentTimetable.teachers = tmpTimetable.teachers;
        currentTimetable.maxTeacherID = tmpTimetable.maxTeacherID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}
