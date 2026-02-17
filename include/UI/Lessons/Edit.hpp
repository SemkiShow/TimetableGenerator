// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The edit lesson menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <memory>
#include <unordered_map>

class EditLessonMenu : public Window
{
  public:
    virtual ~EditLessonMenu() = default;

    void Draw() override;

    void Open(Timetable* prevTimetable, bool newLesson, int lessonId);

  private:
    void Open() override {}

    bool newLesson = false;
    int lessonId = 0;
    bool allClasses = true;
    std::unordered_map<std::string, bool> classGroups;
    std::unordered_map<int, bool> classes;
    bool allClassrooms = true;
    std::unordered_map<int, bool> classrooms;

    Timetable* prevTimetable = nullptr;
    Timetable timetable;
};

extern std::shared_ptr<EditLessonMenu> editLessonMenu;
