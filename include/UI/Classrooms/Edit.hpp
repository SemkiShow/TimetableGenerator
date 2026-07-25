// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The edit classroom menu
 */

#pragma once

#include "Timetable.hpp"
#include "Widgets/Window.hpp"
#include <memory>

class EditClassroomMenu : public Window
{
public:
    virtual ~EditClassroomMenu() = default;

    void Draw() override;

    /**
     * @brief Open the edit classroom menu
     * @note @p startNumber and @p endNumber are ignored if @p newClassroom is false
     *
     * @param newClassroom
     * @param classroomId
     * @param startNumber
     * @param endNumber
     */
    void Open(Timetable* prevTimetable, bool newClassroom, int classroomId, int startNumber,
              int endNumber);

private:
    void Open() override {}

    bool newClassroom_ = false;
    int classroomId_ = 0;
    int startNumber_ = 0, endNumber_ = 0;

    Timetable* prevTimetable_ = nullptr;
    Timetable timetable_;
};

extern std::shared_ptr<EditClassroomMenu> g_editClassroomMenu;
