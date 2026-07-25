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
#include <string>
#include <unordered_map>

class EditLessonMenu : public Window
{
public:
    virtual ~EditLessonMenu() = default;

    void Draw() override;

    void Open(Timetable* prevTimetable, bool newLesson, int lessonId);

private:
    void Open() override {}

    bool newLesson_ = false;
    int lessonId_ = 0;
    bool allClasses_ = true;
    std::unordered_map<std::string, bool> classGroups_;
    std::unordered_map<int, bool> classes_;
    bool allClassrooms_ = true;
    std::unordered_map<int, bool> classrooms_;

    Timetable* prevTimetable_ = nullptr;
    Timetable timetable_;
};

extern std::shared_ptr<EditLessonMenu> g_editLessonMenu;
