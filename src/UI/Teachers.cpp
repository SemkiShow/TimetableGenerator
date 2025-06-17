#include "UI.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"

int currentTeacherID = 0;
bool newTeacher = false;
bool allTeacherLessons = true;
std::unordered_map<int, bool> teacherLessons;
int allAvailableTeacherLessonsVertical[7];
std::vector<int> allAvailableTeacherLessonsHorizontal;
std::unordered_map<int, int> availableTeacherLessons;
std::string teacherLessonValues = "";

static void ResetVariables()
{
    allTeacherLessons = false;
    for (int i = 0; i < 7; i++)
        allAvailableTeacherLessonsVertical[i] = 1;
    allAvailableTeacherLessonsHorizontal.clear();
    for (int i = 0; i < lessonsPerDay; i++)
        allAvailableTeacherLessonsHorizontal.push_back(1);
    if (newTeacher)
    {
        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; j < lessonsPerDay; j++)
                tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.push_back(1);
        }
    }
    teacherLessons.clear();
    for (auto& lesson: currentTimetable.lessons)
        teacherLessons[lesson.first] = false;
    for (int i = 0; i < tmpTmpTimetable.teachers[currentTeacherID].lessonIDs.size(); i++)
        teacherLessons[tmpTmpTimetable.teachers[currentTeacherID].lessonIDs[i]] = true;
    availableTeacherLessons.clear();
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < lessonsPerDay; j++)
            availableTeacherLessons[i*lessonsPerDay+j] = 1;
    }
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j < tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.size(); j++)
        {
            int lessonID = tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs[j];
            if (lessonID == -1)
                availableTeacherLessons[i*lessonsPerDay+j] = 0;
            else if (lessonID == -2)
                availableTeacherLessons[i*lessonsPerDay+j] = 1;
            else
            {
                int counter = 2;
                for (auto& lesson: currentTimetable.lessons)
                {
                    if (lessonID == lesson.first)
                    {
                        availableTeacherLessons[i*lessonsPerDay+j] = counter;
                        break;
                    }
                    counter++;
                }
            }
        }
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
    int pushID = 0;
    for (auto& lesson: currentTimetable.lessons)
    {
        ImGui::PushID(pushID);
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
        pushID++;
    }
    ImGui::Columns(1);
    ImGui::Separator();
    ImGui::Text("available-lessons");
    ImGui::Columns(8);
    ImGui::LabelText("##1", "");
    ImGui::LabelText("##2", "");
    for (int i = 0; i < lessonsPerDay; i++)
    {
        ImGui::PushID(pushID);
        if (ImGui::Combo(std::to_string(i).c_str(), &allAvailableTeacherLessonsHorizontal[i], teacherLessonValues.c_str()))
        {
            for (int j = 0; j < 7; j++)
            {
                availableTeacherLessons[j*lessonsPerDay+i] = allAvailableTeacherLessonsHorizontal[i];
            }
        }
        ImGui::PopID();
        pushID++;
    }
    ImGui::NextColumn();
    for (int i = 0; i < 7; i++)
    {
        ImGui::Text(weekDays[i].c_str());
        ImGui::PushID(pushID);
        if (ImGui::Combo("", &allAvailableTeacherLessonsVertical[i], teacherLessonValues.c_str()))
        {
            for (int j = 0; j < lessonsPerDay; j++)
                availableTeacherLessons[i*lessonsPerDay+j] = allAvailableTeacherLessonsVertical[i];
        }
        ImGui::PopID();
        pushID++;
        for (int j = 0; j < lessonsPerDay; j++)
        {
            ImGui::PushID(pushID);
            ImGui::Combo("", &availableTeacherLessons[i*lessonsPerDay+j], teacherLessonValues.c_str());
            ImGui::PopID();
            pushID++;
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
            tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.clear();
            for (int j = 0; j < lessonsPerDay; j++)
            {
                if (availableTeacherLessons[i*lessonsPerDay+j] == 0)
                    tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.push_back(-1);
                else if (availableTeacherLessons[i*lessonsPerDay+j] == 1)
                    tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.push_back(-2);
                else
                {
                    availableTeacherLessons[i*lessonsPerDay+j] -= 2;
                    for (auto& lesson: currentTimetable.lessons)
                    {
                        if (availableTeacherLessons[i*lessonsPerDay+j] <= 0)
                        {
                            tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.push_back(lesson.first);
                            break;
                        }
                        availableTeacherLessons[i*lessonsPerDay+j]--;
                    }
                }
            }
        }
        tmpTimetable.teachers = tmpTmpTimetable.teachers;
        tmpTimetable.maxTeacherID = tmpTmpTimetable.maxTeacherID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
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
        newTeacher = true;
        tmpTmpTimetable.teachers = tmpTimetable.teachers;
        tmpTmpTimetable.maxTeacherID = tmpTimetable.maxTeacherID;
        tmpTmpTimetable.maxTeacherID++;
        tmpTmpTimetable.teachers[tmpTmpTimetable.maxTeacherID] = Teacher();
        currentTeacherID = tmpTmpTimetable.maxTeacherID;
        ResetVariables();
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
            newTeacher = false;
            tmpTmpTimetable.teachers = tmpTimetable.teachers;
            tmpTmpTimetable.maxTeacherID = tmpTimetable.maxTeacherID;
            currentTeacherID = it->first;
            ResetVariables();
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
