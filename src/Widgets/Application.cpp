// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Widgets/Application.hpp"

void Application::Update()
{
    for (auto& window: windows)
    {
        if (window->IsVisible()) window->Update();
    }
}

void Application::Draw()
{
    for (auto& window: windows)
    {
        if (window->IsVisible())
        {
            window->Draw();
            window->PostDraw();
        }
    }
}
