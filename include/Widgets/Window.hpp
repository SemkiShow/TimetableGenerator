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

    virtual void Open();
    virtual void Close() { visible_ = false; }

    bool IsVisible() const { return visible_; }

protected:
    bool visible_ = true;
};
