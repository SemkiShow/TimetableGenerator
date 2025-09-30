// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <filesystem>
#include <string>
#include <vector>

std::string GetOS();
std::string GetCPU();
long GetRAMMegabytes();
std::vector<std::string> GetGPUs();
std::vector<std::string> GetAllMountPoints();
std::filesystem::space_info GetDiskInfo(const std::string path);
