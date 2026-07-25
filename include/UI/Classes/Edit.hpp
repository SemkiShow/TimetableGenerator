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
#include <string>
#include <unordered_map>
#include <vector>

class EditClassMenu : public Window
{
public:
    virtual ~EditClassMenu() = default;

    void Draw() override;

    /**
     * @note @p bulkCount is ignored if @p bulkEdit is false
     *
     * @param prevTimetable
     * @param newClass
     * @param classId
     * @param bulkEdit
     * @param bulkCount
     */
    void Open(Timetable* prevTimetable, bool newClass, int classId, bool bulkEdit, int bulkCount);

private:
    void Open() override {}

    bool newClass_ = false;
    int classId_ = 0;
    bool bulkEdit_ = false;
    int bulkCount_ = 1;
    int teacherIndex_ = 0;
    std::string teacherValues_;
    std::vector<int> teacherIds_;
    bool allLessons_ = true;
    std::unordered_map<std::string, bool> lessons_;
    std::unordered_map<std::string, int> lessonCounts_;
    std::unordered_map<int, bool> allLessonTeachers_;
    std::unordered_map<std::string, bool> lessonTeachers_;
    std::vector<bool> allAvailableLessonsVertical_;
    std::vector<bool> allAvailableLessonsHorizontal_;

    Timetable* prevTimetable_ = nullptr;
    Timetable timetable_;
};

extern std::shared_ptr<EditClassMenu> g_editClassMenu;
