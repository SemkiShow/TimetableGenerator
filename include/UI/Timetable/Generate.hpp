// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The generate timetable menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>
#include <string>

class GenerateTimetableMenu : public Window
{
  public:
    virtual ~GenerateTimetableMenu() = default;

    void Draw() override;
    void PostDraw() override;

    void SetStatus(const std::string& val) { status = val; }

  private:
    std::string status;
};

extern std::shared_ptr<GenerateTimetableMenu> generateTimetableMenu;
