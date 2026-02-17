// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The new version menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>

class NewVersionMenu : public Window
{
  public:
    virtual ~NewVersionMenu() = default;

    void Draw() override;
};

extern std::shared_ptr<NewVersionMenu> newVersionMenu;
