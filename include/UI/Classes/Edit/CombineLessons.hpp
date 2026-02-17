// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The combine lessons edit class menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <unordered_map>

class CombineLessonsMenu : public Window
{
  public:
    virtual ~CombineLessonsMenu() = default;

    void Draw() override;

    void Open(Timetable* prevTimetable, bool newCombinedLesson, int classId, int lessonId,
              const std::unordered_map<std::string, bool>& classLessons,
              const std::unordered_map<std::string, bool>& classLessonTeachers);

  private:
    void Open() override {}

    bool newCombinedLesson = false;
    int classId = 0, lessonId = 0;
    std::unordered_map<int, bool> lessons;
    std::unordered_map<std::string, bool> lessonTeachers;
    std::unordered_map<std::string, bool> classLessons;
    std::unordered_map<std::string, bool> classLessonTeachers;

    Timetable* prevTimetable = nullptr;
    Timetable timetable;
};

extern std::shared_ptr<CombineLessonsMenu> combineLessonsMenu;
