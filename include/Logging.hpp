// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief printf syntax logging
 * @version 1.1.1
 */

#pragma once

#include <cstdarg>
#include <stdexcept> // IWYU pragma: export

#if defined(__clang__)
#define PRINTF_CHECK __attribute__((format(printf, 1, 2)))
#elif defined(__GNUC__)
#define PRINTF_CHECK __attribute__((format(gnu_printf, 1, 2)))
#else
#define PRINTF_CHECK
#endif

#if defined(__clang__)
#define PRINTF_CHECK_LINE __attribute__((format(printf, 3, 4)))
#elif defined(__GNUC__)
#define PRINTF_CHECK_LINE __attribute__((format(gnu_printf, 3, 4)))
#else
#define PRINTF_CHECK_LINE
#endif

void LogInfo(const char* format, ...) PRINTF_CHECK;
void LogWarning(const char* format, ...) PRINTF_CHECK;
void LogError(const char* format, ...) PRINTF_CHECK;

void VLogInfo(const char* format, va_list args);
void VLogWarning(const char* format, va_list args);
void VLogError(const char* format, va_list args);

void LogInfoLine(const char* file, int line, const char* format, ...) PRINTF_CHECK_LINE;
void LogWarningLine(const char* file, int line, const char* format, ...) PRINTF_CHECK_LINE;
void LogErrorLine(const char* file, int line, const char* format, ...) PRINTF_CHECK_LINE;

void VLogInfoLine(const char* file, int line, const char* format, va_list args);
void VLogWarningLine(const char* file, int line, const char* format, va_list args);
void VLogErrorLine(const char* file, int line, const char* format, va_list args);

#define LOG_INFO(format, ...) LogInfoLine(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) LogWarningLine(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LogErrorLine(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define THROW_ERROR(format, ...)                                                                   \
    do                                                                                             \
    {                                                                                              \
        LogErrorLine(__FILE__, __LINE__, format, ##__VA_ARGS__);                                   \
        throw std::runtime_error(format);                                                          \
    } while (0)
