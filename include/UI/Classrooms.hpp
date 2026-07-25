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
        timetable_.classrooms = prevTimetable_->classrooms;
        timetable_.maxClassroomId = prevTimetable_->maxClassroomId;
        Window::Open();
    }

private:
    Timetable* prevTimetable_ = &g_currentTimetable;
    Timetable timetable_;
};

extern std::shared_ptr<ClassroomsMenu> g_classroomsMenu;
