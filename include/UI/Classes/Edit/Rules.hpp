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
#include <unordered_map>

class RulesMenu : public Window
{
  public:
    virtual ~RulesMenu() = default;

    void Draw() override;

    void Open(Timetable* prevTimetable, bool newRule, int ruleId, int classId,
              const std::unordered_map<std::string, bool>& lessons,
              const std::unordered_map<std::string, int>& lessonAmounts,
              const std::unordered_map<std::string, bool>& lessonTeachers);

  private:
    void Open() override {}

    bool newRule = false;
    int ruleId = 0, classId = 0;
    std::unordered_map<std::string, bool> lessons;
    std::unordered_map<std::string, int> lessonAmounts;
    std::unordered_map<std::string, bool> lessonTeachers;
    std::string timetableLessonValues = "";
    int minTimetableLessonAmount = INT_MAX;

    Timetable* prevTimetable = nullptr;
    Timetable timetable;
};

extern std::shared_ptr<RulesMenu> rulesMenu;
