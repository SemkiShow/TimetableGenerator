// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The setup wizard menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>

class WizardMenu : public Window
{
  public:
    virtual ~WizardMenu() = default;

    void Draw() override;
};

extern std::shared_ptr<WizardMenu> wizardMenu;
