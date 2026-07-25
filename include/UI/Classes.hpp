// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The classes menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <memory>
#include <vector>

class ClassesMenu : public Window
{
public:
    virtual ~ClassesMenu() = default;

    void Draw() override;

    void Open() override
    {
        timetable_.classes = prevTimetable_->classes;
        timetable_.maxClassId = prevTimetable_->maxClassId;
        timetable_.orderedClasses = prevTimetable_->orderedClasses;
        timetable_.lessons = prevTimetable_->lessons;
        timetable_.year = prevTimetable_->year;
        Window::Open();
    }

private:
    Timetable* prevTimetable_ = &g_currentTimetable;
    Timetable timetable_;
};

extern std::shared_ptr<ClassesMenu> g_classesMenu;
  