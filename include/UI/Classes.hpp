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
        timetable.classes = prevTimetable->classes;
        timetable.maxClassId = prevTimetable->maxClassId;
        timetable.orderedClasses = prevTimetable->orderedClasses;
        timetable.lessons = prevTimetable->lessons;
        timetable.year = prevTimetable->year;
        Window::Open();
    }

  private:
    Timetable* prevTimetable = &currentTimetable;
    Timetable timetable;
};

extern std::shared_ptr<ClassesMenu> classesMenu;
