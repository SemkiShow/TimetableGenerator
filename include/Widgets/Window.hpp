// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief The window class
 */

#pragma once

class Window
{
  public:
    virtual ~Window() = default;

    virtual void Update() {}
    virtual void Draw() {}
    virtual void PostDraw() {}

    virtual void Open();
    virtual void Close() { visible = false; }

    bool IsVisible() { return visible; }

  protected:
    bool visible = true;
};
