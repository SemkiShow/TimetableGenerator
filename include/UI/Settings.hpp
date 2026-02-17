// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The settings menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>
#include <string>

class SettingsMenu : public Window
{
  public:
    virtual ~SettingsMenu() = default;

    void Draw() override;

    void ReloadLabels();

  private:
    std::string styleValues = "";
};

extern std::shared_ptr<SettingsMenu> settingsMenu;
