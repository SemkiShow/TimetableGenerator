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

    void SetStatus(const std::string& val) { status_ = val; }

private:
    std::string status_;
};

extern std::shared_ptr<GenerateTimetableMenu> g_generateTimetableMenu;
