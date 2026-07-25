// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The rules edit class menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <climits>
#include <memory>
#include <string>
#include <unordered_map>

class RulesMenu : public Window
{
public:
    virtual ~RulesMenu() = default;

    void Draw() override;

    void Open(Timetable* prevTimetable, bool newRule, int ruleId, int classId,
              const std::unordered_map<std::string, bool>& lessons,
              const std::unordered_map<std::string, int>& lessonCounts,
              const std::unordered_map<std::string, bool>& lessonTeachers);

private:
    void Open() override {}

    bool newRule_ = false;
    int ruleId_ = 0, classId_ = 0;
    std::unordered_map<std::string, bool> lessons_;
    std::unordered_map<std::string, int> lessonCounts_;
    std::unordered_map<std::string, bool> lessonTeachers_;
    std::string timetableLessonValues_;
    int minTimetableLessonCount_ = INT_MAX;

    Timetable* prevTimetable_ = nullptr;
    Timetable timetable_;
};

extern std::shared_ptr<RulesMenu> g_rulesMenu;
