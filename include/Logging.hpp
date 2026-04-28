// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief printf syntax logging
 */

#pragma once

#if defined(__clang__)
#define PRINTF_CHECK __attribute__((format(printf, 1, 2)))
#elif defined(__GNUC__)
#define PRINTF_CHECK __attribute__((format(gnu_printf, 1, 2)))
#else
#define PRINTF_CHECK
#endif

void LogInfo(const char* format, ...) PRINTF_CHECK;
void LogError(const char* format, ...) PRINTF_CHECK;
