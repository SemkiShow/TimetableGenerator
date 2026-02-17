// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The about menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>

class AboutMenu : public Window
{
  public:
    virtual ~AboutMenu() = default;

    void Draw() override;
};

extern std::shared_ptr<AboutMenu> aboutMenu;
