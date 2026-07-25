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
        timetable_.teachers = prevTimetable_->teachers;
        timetable_.maxTeacherId = prevTimetable_->maxTeacherId;
        Window::Open();
    }

private:
    Timetable* prevTimetable_ = &g_currentTimetable;
    Timetable timetable_;
};

extern std::shared_ptr<TeachersMenu> g_teachersMenu;
