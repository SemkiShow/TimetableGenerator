// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The application class
 */

#pragma once

#include "Widgets/Window.hpp"
#include <memory>
#include <vector>

class Application
{
  public:
    virtual ~Application() = default;

    virtual void Update();
    virtual void Draw();

    void AddWindow(std::shared_ptr<Window> window) { windows.push_back(window); }

  private:
    std::vector<std::shared_ptr<Window>> windows;
};
