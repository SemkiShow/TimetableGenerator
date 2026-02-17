// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The edit teacher menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <memory>
#include <unordered_map>

class EditTeacherMenu : public Window
{
  public:
    virtual ~EditTeacherMenu() = default;

    void Draw() override;

    void Open(Timetable* prevTimetable, bool newTeacher, int teacherId);

  private:
    void Open() override {}

    void ResetTeacherLessonValues();

    bool newTeacher = false;
    int teacherId = 0;
    bool allLessons = true;
    std::unordered_map<int, bool> lessons;
    std::vector<int> allAvailableLessonsVertical;
    std::vector<int> allAvailableLessonsHorizontal;
    std::unordered_map<int, int> availableLessons;
    std::string lessonValues = "";

    Timetable* prevTimetable = nullptr;
    Timetable timetable;
};

extern std::shared_ptr<EditTeacherMenu> editTeacherMenu;
