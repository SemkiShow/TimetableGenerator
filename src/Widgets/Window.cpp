// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Widgets/Window.hpp"
#include "Logging.hpp"
#include <string>
#include <typeinfo>

void Window::Open()
{
    LogInfo(std::string("Opening ") + typeid(*this).name());
    visible = true;
}
