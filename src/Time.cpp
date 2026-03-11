// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Time.hpp"
#include <chrono>
#include <iomanip>

Time GetCurrentTime()
{
    auto now = std::chrono::system_clock::now();
    auto timeTNow = std::chrono::system_clock::to_time_t(now);
    tm tm;
#ifdef _WIN32
    localtime_s(&tm, &timeTNow);
#else
    localtime_r(&timeTNow, &tm);
#endif
    return tm;
}

std::string GetTimeString(const Time& time)
{
    tm tm = time.ToTm();
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

static bool IsTimeCorrect(const std::string& timeString)
{
    const auto& s = timeString;

    // Check size
    if (s.size() < 19) return false;

    // Check -
    if (s[4] != '-' || s[7] != '-') return false;

    // Check :
    if (s[13] != ':' || s[16] != ':') return false;

    // Check year digits
    if ((isdigit(s[0]) == 0) || (isdigit(s[1]) == 0) || (isdigit(s[2]) == 0) ||
        (isdigit(s[3]) == 0))
        return false;

    // Check month digits
    if ((isdigit(s[5]) == 0) || (isdigit(s[6]) == 0)) return false;

    // Check day digits
    if ((isdigit(s[8]) == 0) || (isdigit(s[9]) == 0)) return false;

    // Check hour digits
    if ((isdigit(s[11]) == 0) || (isdigit(s[12]) == 0)) return false;

    // Check minute digits
    if ((isdigit(s[14]) == 0) || (isdigit(s[15]) == 0)) return false;

    // Check second digits
    if ((isdigit(s[17]) == 0) || (isdigit(s[18]) == 0)) return false;

    // Success!
    return true;
}

Time ExtractTime(const std::string& timeString)
{
    const auto& s = timeString;
    Time time;

    if (!IsTimeCorrect(s)) return time;

    time.year = (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0');
    time.month = (s[5] - '0') * 10 + (s[6] - '0');
    time.day = (s[8] - '0') * 10 + (s[9] - '0');
    time.hour = (s[11] - '0') * 10 + (s[12] - '0');
    time.minute = (s[14] - '0') * 10 + (s[15] - '0');
    time.second = (s[17] - '0') * 10 + (s[18] - '0');

    return time;
}
