// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "System.hpp"
#include <cstdlib>
#include <filesystem>
#include <string>

void OpenInFileManager(const std::filesystem::path& path)
{
#ifdef _WIN32
    std::string command = "explorer \"" + path.string() + "\"";
    (void)system(command.c_str());
#elif __APPLE__
    std::string command = "open \"" + path.string() + "\"";
    (void)system(command.c_str());
#elif __linux__
    std::string command = "xdg-open \"" + path.string() + "\"";
    (void)system(command.c_str());
#else
#error Platform not supported
#endif
}
