// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief Translations
 */

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#define _(s) (s)

/**
 * @brief An std::string wrapper for libintl-lite's gettext
 *
 * @param id The id for the target string (usually in English)
 * @return std::string
 */
std::string GetText(const std::string& id);

/**
 * @brief A functon from libintl-lite
 *
 * @param id
 * @return const char*
 */
const char* gettext(const char* id);

/**
 * @brief Set the language
 *
 * @param domain The name of the .mo file
 * @param localePath A path for locales
 * @param language The target language
 * @note This funcitons requires a specific file layout:
 * @p localePath / @p language / LC_MESSAGES / @p domain .mo
 * @see GetText
 */
void SetLanguage(const std::string& domain, const std::filesystem::path& localePath,
                 const std::string& language);

/**
 * @brief Read all folders in resources/locales to get all available languages
 *
 */
void GetAllLanguages();

/**
 * @brief Change some strings after the language has been changed
 *
 */
void ReloadLabels();

extern std::vector<std::string> availableLanguages;
extern std::string languageValues;
extern int languageId;
