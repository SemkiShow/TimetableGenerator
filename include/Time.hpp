// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief Time related functions
 */

#pragma once

#include <ctime>
#include <string>

struct Time
{
    static constexpr int FIRST_YEAR = 1900;

    enum class Format
    {
        Iso,         // YYYY-MM-DD hh:mm:ss
        IsoDate,     // YYYY-MM-DD
        IsoDateTime, // YYYY-MM-DDThh:mm:ssZ
        Path,        // YYYY.MM.DD hh-mm-ss
    };

    tm ToTm() const;
    static Time FromTm(const tm& tm);

    std::string ToString(Format format = Format::Iso) const;
    static Time FromString(const std::string& str, Format format = Format::Iso);

    int year = FIRST_YEAR;
    int month = 1;
    int day = 1;
    int hour = 0;
    int minute = 0;
    int second = 0;
};

Time GetCurrentTime();
