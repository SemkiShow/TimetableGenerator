// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Utils.hpp"
#include <algorithm>
#include <filesystem>
#include <utf8/checked.h>

std::vector<std::string> Split(std::string input, char delimiter)
{
    std::vector<std::string> output;
    output.push_back("");
    int index = 0;
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] == delimiter)
        {
            index++;
            output.push_back("");
            continue;
        }
        output[index] += input[i];
    }
    return output;
}

std::vector<std::string> ListFiles(const std::filesystem::path& path)
{
    std::vector<std::string> files;
    for (const auto& entry: std::filesystem::directory_iterator(path))
    {
        if (std::filesystem::is_regular_file(entry.path()))
        {
            files.push_back(entry.path().string());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

std::string TrimJunk(const std::string& input)
{
    auto first = input.find_first_not_of("\t\n\r\f\v");
    auto last = input.find_last_not_of("\t\n\r\f\v");
    return (first == input.npos) ? "" : input.substr(first, last - first + 1);
}

std::string GetNthUtf8Character(const std::string& utf8String, int index)
{
    auto it = utf8String.begin();
    auto end = utf8String.end();

    for (int i = 0; i < index && it != end; ++i) utf8::next(it, end);

    if (it == end) return "";

    auto charStart = it;
    utf8::next(it, end);
    return std::string(charStart, it);
}
