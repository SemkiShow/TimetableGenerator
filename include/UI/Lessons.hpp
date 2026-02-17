// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The lessons menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <memory>

class LessonsMenu : public Window
{
  public:
    virtual ~LessonsMenu() = default;

    void Draw() override;

    void Open() override
    {
        timetable.lessons = prevTimetable->lessons;
        timetable.maxLessonId = prevTimetable->maxLessonId;
        Window::Open();
    }

  private:
    Timetable* prevTimetable = &currentTimetable;
    Timetable timetable;
};

extern std::shared_ptr<LessonsMenu> lessonsMenu;
