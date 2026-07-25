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
#include <memory>
#include <string>
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

    bool newCombinedLesson_ = false;
    int classId_ = 0, lessonId_ = 0;
    std::unordered_map<int, bool> lessons_;
    std::unordered_map<std::string, bool> lessonTeachers_;
    std::unordered_map<std::string, bool> classLessons_;
    std::unordered_map<std::string, bool> classLessonTeachers_;

    Timetable* prevTimetable_ = nullptr;
    Timetable timetable_;
};

extern std::shared_ptr<CombineLessonsMenu> g_combineLessonsMenu;
