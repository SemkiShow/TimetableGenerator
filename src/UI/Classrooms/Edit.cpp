// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classrooms/Edit.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "Widgets/Window.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <imgui.h>
#include <memory>
#include <misc/cpp/imgui_stdlib.h>
#include <string>

std::shared_ptr<EditClassroomMenu> g_editClassroomMenu;

void EditClassroomMenu::Open(Timetable* prevTimetable, bool newClassroom, int classroomId,
                             int startNumber, int endNumber)
{
    this->prevTimetable_ = prevTimetable;
    this->newClassroom_ = newClassroom;
    this->classroomId_ = classroomId;
    this->startNumber_ = startNumber;
    this->endNumber_ = endNumber;

    if (newClassroom)
    {
        try
        {
            timetable_.classrooms[timetable_.maxClassroomId + 1].name =
                std::to_string(stoi(timetable_.classrooms[timetable_.maxClassroomId].name) + 1);
        }
        catch (const std::exception&)
        {
            timetable_.classrooms[timetable_.maxClassroomId + 1].name = "";
        }
    }

    timetable_ = *prevTimetable;

    Window::Open();
}

void EditClassroomMenu::Draw()
{
    if (!ImGui::Begin((newClassroom_ ? gettext("New classroom") : gettext("Edit classroom")),
                      &visible_))
    {
        ImGui::End();
        return;
    }

    // Classroom data
    if (newClassroom_)
    {
        bool isNameANumber = true;
        for (char c: timetable_.classrooms[classroomId_].name)
        {
            if (std::isdigit(c) == 0)
            {
                isNameANumber = false;
                break;
            }
        }
        if (isNameANumber)
        {
            if (ImGui::InputInt(gettext("start number"), &startNumber_))
            {
                LogInfo("Changed startNumber to %d in classroom with id %d", startNumber_,
                        classroomId_);
                timetable_.classrooms[classroomId_].name = std::to_string(startNumber_);
            }
            startNumber_ = std::max(startNumber_, 0);
            ImGui::InputInt(gettext("end number"), &endNumber_);
            endNumber_ = std::max(endNumber_, 0);
            endNumber_ = std::max(endNumber_, startNumber_);
        }

        if (startNumber_ == endNumber_)
        {
            if (ImGui::InputText(gettext("name"), &timetable_.classrooms[classroomId_].name))
            {
                LogInfo("Changed classroom name to %s in classroom with id %d",
                        timetable_.classrooms[classroomId_].name.c_str(), classroomId_);
                startNumber_ = endNumber_ = atoi(timetable_.classrooms[classroomId_].name.c_str());
            }
        }
    }
    else
    {
        ImGui::InputText(gettext("name"), &timetable_.classrooms[classroomId_].name);
    }
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed Ok while editing a classroom with id %d", classroomId_);
        if (newClassroom_)
        {
            if (startNumber_ != endNumber_)
            {
                for (int i = startNumber_; i <= endNumber_; i++)
                {
                    timetable_.classrooms[timetable_.maxClassroomId] = Classroom();
                    timetable_.classrooms[timetable_.maxClassroomId].name = std::to_string(i);
                    timetable_.maxClassroomId++;
                }
            }
        }
        prevTimetable_->classrooms = timetable_.classrooms;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
