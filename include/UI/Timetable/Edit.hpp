// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The new timetable menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>
#include <string>

class NewTimetableMenu : public Window
{
  public:
    virtual ~NewTimetableMenu() = default;

    void Draw() override;

    void Open(bool newTimetable, const std::string& timetableName)
    {
        this->newTimetable = newTimetable;
        this->timetableName = timetableName;
        Window::Open();
    }

  private:
    void Open() override {}

    bool newTimetable = false;
    std::string timetableName;
};

extern std::shared_ptr<NewTimetableMenu> newTimetableMenu;
