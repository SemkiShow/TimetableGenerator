// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The edit class menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <memory>
#include <unordered_map>

class EditClassMenu : public Window
{
  public:
    virtual ~EditClassMenu() = default;

    void Draw() override;

    /**
     * @note @p bulkAmount is ignored if @p bulkEdit is false
     *
     * @param prevTimetable
     * @param newClass
     * @param classId
     * @param bulkEdit
     * @param bulkAmount
     */
    void Open(Timetable* prevTimetable, bool newClass, int classId, bool bulkEdit, int bulkAmount);

  private:
    void Open() override {}

    bool newClass = false;
    int classId = 0;
    bool bulkEdit = false;
    int bulkAmount = 1;
    int teacherIndex = 0;
    std::string teacherValues = "";
    std::vector<int> teacherIds;
    bool allLessons = true;
    std::unordered_map<std::string, bool> lessons;
    std::unordered_map<std::string, int> lessonAmounts;
    std::unordered_map<int, bool> allLessonTeachers;
    std::unordered_map<std::string, bool> lessonTeachers;
    std::vector<bool> allAvailableLessonsVertical;
    std::vector<bool> allAvailableLessonsHorizontal;

    Timetable* prevTimetable = nullptr;
    Timetable timetable;
};

extern std::shared_ptr<EditClassMenu> editClassMenu;
