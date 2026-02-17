// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The FAQ menu
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>
#include <vector>

struct Texture;

class FaqMenu : public Window
{
  public:
    virtual ~FaqMenu() = default;

    void Draw() override;
};

extern std::shared_ptr<FaqMenu> faqMenu;
extern std::vector<Texture> faqScreenshots;
