// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The teachers menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <memory>

class TeachersMenu : public Window
{
  public:
    virtual ~TeachersMenu() = default;

    void Draw() override;

    void Open() override
    {
        timetable.teachers = prevTimetable->teachers;
        timetable.maxTeacherId = prevTimetable->maxTeacherId;
        Window::Open();
    }

  private:
    Timetable* prevTimetable = &currentTimetable;
    Timetable timetable;
};

extern std::shared_ptr<TeachersMenu> teachersMenu;
