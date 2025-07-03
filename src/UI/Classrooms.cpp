#include "Settings.hpp"
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
    if (!ImGui::Begin((newClassroom ? labels["New Classroom"] : labels["Edit Classroom"]).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    if (newClassroom)
    {
        if (ImGui::InputInt(labels["start number"].c_str(), &classroomsStartNumber))
        {
            classroomsEndNumber = classroomsStartNumber + classroomsAmount - 1;
        }
        if (ImGui::InputInt(labels["end number"].c_str(), &classroomsEndNumber))
        {
            classroomsAmount = classroomsEndNumber - classroomsStartNumber + 1;
        }
        if (ImGui::InputInt(labels["amount"].c_str(), &classroomsAmount))
        {
            classroomsEndNumber = classroomsStartNumber + classroomsAmount - 1;
        }
    }
    else
    {
        ImGui::InputText(labels["name"].c_str(), &tmpTmpTimetable.classrooms[currentClassroomID].name);
    }
    ImGui::Separator();
    if (ImGui::Button(labels["Ok"].c_str()))
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
    if (ImGui::Button(labels["Cancel"].c_str())) *isOpen = false;
    ImGui::End();

}

bool isClassrooms = false;
void ShowClassrooms(bool* isOpen)
{
    if (!ImGui::Begin(labels["Classrooms"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }
    if (ImGui::Button(labels["+"].c_str()))
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
        if (ImGui::Button(labels["-"].c_str()))
        {
            ImGui::PopID();
            it = tmpTimetable.classrooms.erase(it);
            continue;
        }
        ImGui::SameLine();
        if (ImGui::Button(labels["Edit"].c_str()))
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
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        currentTimetable.classrooms = tmpTimetable.classrooms;
        currentTimetable.maxClassroomID = tmpTimetable.maxClassroomID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str())) *isOpen = false;
    ImGui::End();
}
