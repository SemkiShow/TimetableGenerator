// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classrooms/Edit.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "Widgets/Window.hpp"
#include <cctype>
#include <cstddef>
#include <exception>
#include <imgui.h>
#include <memory>
#include <misc/cpp/imgui_stdlib.h>
#include <string>

std::shared_ptr<EditClassroomMenu> editClassroomMenu;

void EditClassroomMenu::Open(Timetable* prevTimetable, bool newClassroom, int classroomId,
                             int startNumber, int endNumber)
{
    this->prevTimetable = prevTimetable;
    this->newClassroom = newClassroom;
    this->classroomId = classroomId;
    this->startNumber = startNumber;
    this->endNumber = endNumber;

    if (newClassroom)
    {
        try
        {
            timetable.classrooms[timetable.maxClassroomId + 1].name =
                std::to_string(stoi(timetable.classrooms[timetable.maxClassroomId].name) + 1);
        }
        catch (const std::exception&)
        {
        }
    }

    timetable = *prevTimetable;

    Window::Open();
}

void EditClassroomMenu::Draw()
{
    if (!ImGui::Begin((newClassroom ? gettext("New classroom") : gettext("Edit classroom")),
                      &visible))
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
            for (size_t i = 0; i < timetable.classrooms[classroomId].name.size(); i++)
            {
                if (!std::isdigit(timetable.classrooms[classroomId].name[i]))
                {
                    isNameANumber = false;
                    break;
                }
            }
            if (isNameANumber)
            {
                if (ImGui::InputInt(gettext("start number"), &startNumber))
                {
                    LogInfo("Changed startNumber to %d in classroom with id %d", startNumber,
                            classroomId);
                    timetable.classrooms[classroomId].name = std::to_string(startNumber);
                }
                if (startNumber < 0) startNumber = 0;
                ImGui::InputInt(gettext("end number"), &endNumber);
                if (endNumber < 0) endNumber = 0;
                if (endNumber < startNumber) endNumber = startNumber;
            }
        }
        catch (const std::exception&)
        {
        }
        if (startNumber == endNumber)
        {
            if (ImGui::InputText(gettext("name"), &timetable.classrooms[classroomId].name))
            {
                LogInfo("Changed classroom name to %s in classroom with id %d",
                        timetable.classrooms[classroomId].name.c_str(), classroomId);
                try
                {
                    startNumber = endNumber = stoi(timetable.classrooms[classroomId].name);
                }
                catch (const std::exception&)
                {
                }
            }
        }
    }
    else
    {
        ImGui::InputText(gettext("name"), &timetable.classrooms[classroomId].name);
    }
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed Ok while editing a classroom with id %d", classroomId);
        if (newClassroom)
        {
            if (startNumber != endNumber)
            {
                for (int i = startNumber; i <= endNumber; i++)
                {
                    timetable.classrooms[timetable.maxClassroomId] = Classroom();
                    timetable.classrooms[timetable.maxClassroomId].name = std::to_string(i);
                    timetable.maxClassroomId++;
                }
            }
        }
        prevTimetable->classrooms = timetable.classrooms;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
