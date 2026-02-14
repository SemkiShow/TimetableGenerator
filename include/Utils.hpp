// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief Utils
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

std::vector<std::string> Split(std::string input, char delimiter = ' ');
std::vector<std::string> ListFiles(const std::filesystem::path& path);
std::string TrimJunk(const std::string& input);
std::string GetNthUtf8Character(const std::string& utf8String, int index);
