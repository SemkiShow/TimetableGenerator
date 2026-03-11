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

// NOLINTBEGIN
struct Time
{
    Time() = default;
    Time(int year, int month, int day, int hour, int minute, int second)
        : year(year), month(month), day(day), hour(hour), minute(minute), second(second)
    {
    }
    Time(const tm& tm)
        : year(tm.tm_year + 1900), month(tm.tm_mon + 1), day(tm.tm_mday), hour(tm.tm_hour),
          minute(tm.tm_min), second(tm.tm_sec)
    {
    }

    tm ToTm() const
    {
        tm tm;
        tm.tm_year = year - 1900;
        tm.tm_mon = month - 1;
        tm.tm_mday = day;
        tm.tm_hour = hour;
        tm.tm_min = minute;
        tm.tm_sec = second;
        return tm;
    }

    void FromTm(const tm& tm) { *this = tm; }

    int year = 1900;
    int month = 1;
    int day = 1;
    int hour = 0;
    int minute = 0;
    int second = 0;
};
// NOLINTEND

Time GetCurrentTime();

/**
 * @brief Print Time as an ISO 8601 time string
 *
 * @param time
 * @return std::string
 */
std::string GetTimeString(const Time& time);

/**
 * @brief Extract time from a time string in the ISO 8601 format
 * @warning This function returns Time{} if the input string is incorrect
 * @param timeString
 * @return Time
 */
Time ExtractTime(const std::string& timeString);
