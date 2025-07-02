#include "UI.hpp"
#include "Timetable.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

bool newClassroom = false;
int currentClassroomID = 0;
int classroomsStartNumber, classroomsEndNumber, classroomsAmount;

bool isEditClassroom = false;
void ShowEditClassroom(bool* isOpen)
{
    if (!ImGui::Begin(((newClassroom ? "New" : "Edit") + std::string(" Classroom")).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    if (newClassroom)
    {
        if (ImGui::InputInt("start-number", &classroomsStartNumber))
        {
            classroomsEndNumber = classroomsStartNumber + classroomsAmount - 1;
        }
        if (ImGui::InputInt("end-number", &classroomsEndNumber))
        {
            classroomsAmount = classroomsEndNumber - classroomsStartNumber + 1;
        }
        if (ImGui::InputInt("amount", &classroomsAmount))
        {
            classroomsEndNumber = classroomsStartNumber + classroomsAmount - 1;
        }
    }
    else
    {
        ImGui::InputText("name", &tmpTmpTimetable.classrooms[currentClassroomID].name);
    }
    ImGui::Separator();
    if (ImGui::Button("Ok"))
    {
        if (newClassroom)
        {
            for (int i = classroomsStartNumber; i <= classroomsEndNumber; i++)
            {
                tmpTimetable.maxClassroomID++;
                tmpTmpTimetable.classrooms[tmpTimetable.maxClassroomID] = Classroom();
                tmpTmpTimetable.classrooms[tmpTimetable.maxClassroomID].name = std::to_string(i);
            }
        }
        tmpTimetable.classrooms = tmpTmpTimetable.classrooms;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();

}

bool isClassrooms = false;
void ShowClassrooms(bool* isOpen)
{
    if (!ImGui::Begin("Classrooms", isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::Button("+"))
    {
        newClassroom = true;
        try
        {
            classroomsStartNumber = classroomsEndNumber = stoi(tmpTimetable.classrooms[tmpTimetable.maxClassroomID].name)+1;
        }
        catch (const std::exception&)
        {

        }
        classroomsAmount = 1;
        isEditClassroom = true;
    }
    for (auto it = tmpTimetable.classrooms.begin(); it != tmpTimetable.classrooms.end();)
    {
        ImGui::PushID(it->first);
        if (ImGui::Button("-"))
        {
            ImGui::PopID();
            it = tmpTimetable.classrooms.erase(it);
            continue;
        }
        ImGui::SameLine();
        if (ImGui::Button("Edit"))
        {
            newClassroom = false;
            currentClassroomID = it->first;
            isEditClassroom = true;
        }
        ImGui::SameLine();
        ImGui::LabelText("", "%s", it->second.name.c_str());
        ImGui::PopID();
        ++it;
    }
    ImGui::Separator();
    if (ImGui::Button("Ok"))
    {
        currentTimetable.classrooms = tmpTimetable.classrooms;
        currentTimetable.maxClassroomID = tmpTimetable.maxClassroomID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) *isOpen = false;
    ImGui::End();
}
