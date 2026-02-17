// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Widgets/Application.hpp"
#include <string>

struct Vector2;

enum class Style
{
    Dark,
    Light,
    Classic
};

#define DEFAULT_FONT_SIZE 16

extern Vector2 windowSize;
extern std::string weekDays[7];

void LoadResources();
void LoadFonts();
void LoadStyle();
void InitUI();
void DrawFrame();
void FreeResources();

extern std::shared_ptr<Application> app;
