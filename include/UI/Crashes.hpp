// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The crashes menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>

class CrashesMenu : public Window
{
  public:
    virtual ~CrashesMenu() = default;

    void Draw() override;

    void Open() override
    {
        sendLogs = sendTimetables = sendSettings = sendSystemInfo = true;
        Window::Open();
    }

  private:
    int CreateCrashReport();

    bool sendLogs = true;
    bool sendTimetables = true;
    bool sendSettings = true;
    bool sendSystemInfo = true;
};

extern std::shared_ptr<CrashesMenu> crashesMenu;
