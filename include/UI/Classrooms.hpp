// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The classrooms menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <memory>

class ClassroomsMenu : public Window
{
  public:
    virtual ~ClassroomsMenu() = default;

    void Draw() override;

    void Open() override
    {
        timetable.classrooms = prevTimetable->classrooms;
        timetable.maxClassroomId = prevTimetable->maxClassroomId;
        Window::Open();
    }

  private:
    Timetable* prevTimetable = &currentTimetable;
    Timetable timetable;
};

extern std::shared_ptr<ClassroomsMenu> classroomsMenu;
