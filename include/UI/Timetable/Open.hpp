// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The open timetable menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>

class OpenTimetableMenu : public Window
{
  public:
    virtual ~OpenTimetableMenu() = default;

    void Draw() override;

    void Open() override;
};

extern std::shared_ptr<OpenTimetableMenu> openTimetableMenu;
