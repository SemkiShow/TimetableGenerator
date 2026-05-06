// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Time.hpp"
#include "Logging.hpp"
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>

tm Time::ToTm() const
{
    tm tm;
    tm.tm_year = year - FIRST_YEAR;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    return tm;
}

Time Time::FromTm(const tm& tm)
{
    Time time;
    time.year = tm.tm_year + FIRST_YEAR;
    time.month = tm.tm_mon + 1;
    time.day = tm.tm_mday;
    time.hour = tm.tm_hour;
    time.minute = tm.tm_min;
    time.second = tm.tm_sec;
    return time;
}

Time operator+(Time a, Time b)
{
    tm cTm = a.ToTm();
    cTm.tm_year += b.year;
    cTm.tm_mon += b.month;
    cTm.tm_mday += b.day;
    cTm.tm_hour += b.hour;
    cTm.tm_min += b.minute;
    cTm.tm_sec += b.second;
    cTm.tm_isdst = -1;

    if (mktime(&cTm) == -1)
    {
        LogError("Failed to perform %s + %s: %s", a.ToString().c_str(), b.ToString().c_str(),
                 strerror(errno));
        return {};
    }

    return Time::FromTm(cTm);
}

Time& operator+=(Time& a, Time b) { return a = a + b; }

bool operator==(Time a, Time b)
{
    return a.year == b.year && a.month == b.month && a.day == b.day && a.hour == b.hour &&
           a.minute == b.minute && a.second == b.second;
}

bool operator!=(Time a, Time b) { return !(a == b); }

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
    return Time::FromTm(tm);
}

std::string Time::ToString(Format format) const
{
    constexpr int BUF_LEN = 32;
    char buf[BUF_LEN];
    int res = 0;

    switch (format)
    {
    case Format::Iso:
        res = snprintf(buf, BUF_LEN, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour,
                       minute, second);
        break;
    case Format::IsoDate:
        res = snprintf(buf, BUF_LEN, "%04d-%02d-%02d", year, month, day);
        break;
    case Format::IsoDateTime:
        res = snprintf(buf, BUF_LEN, "%04d-%02d-%02dT%02d:%02d:%02dZ", year, month, day, hour,
                       minute, second);
        break;
    case Format::Path:
        res = snprintf(buf, BUF_LEN, "%04d.%02d.%02d %02d-%02d-%02d", year, month, day, hour,
                       minute, second);
        break;
    default:
        LogError("Invalid Time format!");
        break;
    }
    if (res < 0 || res >= BUF_LEN) return "";
    return {buf};
}

Time Time::FromString(const std::string& str, Format format)
{
    Time t;
    int count = 0;
    constexpr int EXPECTED_VALUES[] = {6, 3, 6, 6};
    Time delta = TIME_ZERO;
    char tzSign[2] = {0};

    switch (format)
    {
    case Format::Iso:
        count = sscanf(str.c_str(), "%d-%d-%d %d:%d:%d", &t.year, &t.month, &t.day, &t.hour,
                       &t.minute, &t.second);
        break;
    case Format::IsoDate:
        count = sscanf(str.c_str(), "%d-%d-%d", &t.year, &t.month, &t.day);
        break;
    case Format::IsoDateTime:
        count = sscanf(str.c_str(), "%d-%d-%dT%d:%d:%d%1[+-Z]%d:%d", &t.year, &t.month, &t.day,
                       &t.hour, &t.minute, &t.second, tzSign, &delta.hour, &delta.minute);

        // No terminating character
        if (count == EXPECTED_VALUES[static_cast<int>(Format::IsoDateTime)] + 1)
        {
            count = EXPECTED_VALUES[static_cast<int>(Format::IsoDateTime)];
        }

        // Timezone present
        if (count == EXPECTED_VALUES[static_cast<int>(Format::IsoDateTime)] + 3)
        {
            count = EXPECTED_VALUES[static_cast<int>(Format::IsoDateTime)];
            if (tzSign[0] == '+')
            {
                delta.hour *= -1;
                delta.minute *= -1;
            }
            t += delta;
        }
        break;
    case Format::Path:
        count = sscanf(str.c_str(), "%d.%d.%d %d-%d-%d", &t.year, &t.month, &t.day, &t.hour,
                       &t.minute, &t.second);
        break;
    default:
        LogError("Invalid Time format!");
        return {};
    }

    if (count != EXPECTED_VALUES[static_cast<int>(format)])
    {
        LogError("Invalid time string: %s", str.c_str());
        return {};
    }
    return t;
}
