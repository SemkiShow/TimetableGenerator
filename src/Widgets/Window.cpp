// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Widgets/Window.hpp"
#include "Logging.hpp"
#include <typeinfo>

void Window::Open()
{
    LogInfo("Opening %s", typeid(*this).name());
    visible = true;
}
