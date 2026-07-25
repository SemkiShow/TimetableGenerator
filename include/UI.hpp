// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Widgets/Application.hpp"
#include <memory>
#include <vector>

struct Vector2;

#define COLOR_ERROR   ImVec4(255, 0, 0, 255)
#define COLOR_WARNING ImVec4(255, 255, 0, 255)
#define COLOR_SUCCESS ImVec4(0, 255, 0, 255)

enum class Style
{
    Dark,
    Light,
    Classic
};

extern Vector2 g_windowSize;
extern std::vector<const char*> g_weekDays;

void LoadResources();
void LoadFonts();
void LoadStyle();
void InitUI();
void DrawFrame();

extern std::shared_ptr<Application> g_app;
