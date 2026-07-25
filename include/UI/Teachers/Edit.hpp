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
#include <string>
#include <unordered_map>
#include <vector>

class EditTeacherMenu : public Window
{
public:
    virtual ~EditTeacherMenu() = default;

    void Draw() override;

    void Open(Timetable* prevTimetable, bool newTeacher, int teacherId);

private:
    void Open() override {}

    void ResetTeacherLessonValues();

    bool newTeacher_ = false;
    int teacherId_ = 0;
    bool allLessons_ = true;
    std::unordered_map<int, bool> lessons_;
    std::vector<int> allAvailableLessonsVertical_;
    std::vector<int> allAvailableLessonsHorizontal_;
    std::unordered_map<int, int> availableLessons_;
    std::string lessonValues_;

    Timetable* prevTimetable_ = nullptr;
    Timetable timetable_;
};

extern std::shared_ptr<EditTeacherMenu> g_editTeacherMenu;
