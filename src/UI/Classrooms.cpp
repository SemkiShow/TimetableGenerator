// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <cctype>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>

bool newClassroom = false;
int currentClassroomID = 0;
int classroomsStartNumber, classroomsEndNumber;

bool isEditClassroom = false;
void ShowEditClassroom(bool* isOpen)
{
    if (!ImGui::Begin((newClassroom ? gettext("New classroom") : gettext("Edit classroom")),
                      isOpen))
    {
        ImGui::End();
        return;
    }

    // Classroom data
    if (newClassroom)
    {
        try
        {
            bool isNameANumber = true;
            for (size_t i = 0; i < tmpTmpTimetable.classrooms[currentClassroomID].name.size(); i++)
            {
                if (!std::isdigit(tmpTmpTimetable.classrooms[currentClassroomID].name[i]))
                {
                    isNameANumber = false;
                    break;
                }
            }
            if (isNameANumber)
            {
                if (ImGui::InputInt(gettext("start number"), &classroomsStartNumber))
                {
                    LogInfo("Changed classroomsStartNumber to " +
                            std::to_string(classroomsStartNumber) + " in classroom with ID " +
                            std::to_string(currentClassroomID));
                    tmpTmpTimetable.classrooms[currentClassroomID].name =
                        std::to_string(classroomsStartNumber);
                }
                if (classroomsStartNumber < 0) classroomsStartNumber = 0;
                ImGui::InputInt(gettext("end number"), &classroomsEndNumber);
                if (classroomsEndNumber < 0) classroomsEndNumber = 0;
                if (classroomsEndNumber < classroomsStartNumber)
                    classroomsEndNumber = classroomsStartNumber;
            }
        }
        catch (const std::exception&)
        {
        }
        if (classroomsStartNumber == classroomsEndNumber)
        {
            if (ImGui::InputText(gettext("name"),
                                 &tmpTmpTimetable.classrooms[currentClassroomID].name))
            {
                LogInfo("Changed classroom name to " +
                        tmpTmpTimetable.classrooms[currentClassroomID].name +
                        " in classroom with ID " + std::to_string(currentClassroomID));
                try
                {
                    classroomsStartNumber = classroomsEndNumber =
                        stoi(tmpTmpTimetable.classrooms[currentClassroomID].name);
                }
                catch (const std::exception&)
                {
                }
            }
        }
    }
    else
    {
        ImGui::InputText(gettext("name"), &tmpTmpTimetable.classrooms[currentClassroomID].name);
    }
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed Ok while editing a classroom with ID " +
                std::to_string(currentClassroomID));
        if (newClassroom)
        {
            if (classroomsStartNumber != classroomsEndNumber)
            {
                for (int i = classroomsStartNumber; i <= classroomsEndNumber; i++)
                {
                    tmpTmpTimetable.classrooms[tmpTmpTimetable.maxClassroomID] = Classroom();
                    tmpTmpTimetable.classrooms[tmpTmpTimetable.maxClassroomID].name =
                        std::to_string(i);
                    tmpTmpTimetable.maxClassroomID++;
                }
            }
        }
        tmpTimetable.classrooms = tmpTmpTimetable.classrooms;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}

bool isClassrooms = false;
void ShowClassrooms(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Classrooms"), isOpen))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(gettext("+")))
    {
        newClassroom = true;
        tmpTmpTimetable.classrooms = tmpTimetable.classrooms;
        tmpTmpTimetable.maxClassroomID = tmpTimetable.maxClassroomID;
        try
        {
            classroomsStartNumber = classroomsEndNumber =
                stoi(tmpTmpTimetable.classrooms[tmpTmpTimetable.maxClassroomID].name) + 1;
            tmpTmpTimetable.classrooms[tmpTmpTimetable.maxClassroomID + 1].name =
                std::to_string(stoi(tmpTimetable.classrooms[tmpTimetable.maxClassroomID].name) + 1);
        }
        catch (const std::exception&)
        {
        }
        currentClassroomID = tmpTmpTimetable.maxClassroomID + 1;
        LogInfo("Adding a new classroom with ID " + std::to_string(currentClassroomID));
        isEditClassroom = true;
    }

    for (auto it = tmpTimetable.classrooms.begin(); it != tmpTimetable.classrooms.end();)
    {
        ImGui::PushID(it->first);

        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a classroom with ID " + std::to_string(it->first));
            ImGui::PopID();
            it = tmpTimetable.classrooms.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            newClassroom = false;
            currentClassroomID = it->first;
            LogInfo("Editing a classroom with ID " + std::to_string(currentClassroomID));
            isEditClassroom = true;
        }
        ImGui::SameLine();

        ImGui::LabelText("", "%s", it->second.name.c_str());
        ImGui::PopID();
        ++it;
    }
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed Ok in the classrooms menu");
        currentTimetable.classrooms = tmpTimetable.classrooms;
        currentTimetable.maxClassroomID = tmpTimetable.maxClassroomID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}
