// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Settings.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <algorithm>
#include <imgui.h>
#include <memory>

std::shared_ptr<SettingsMenu> g_settingsMenu;

void SettingsMenu::ReloadLabels()
{
    styleValues_ = "";
    styleValues_ += GetText("dark");
    styleValues_ += '\0';
    styleValues_ += GetText("light");
    styleValues_ += '\0';
    styleValues_ += GetText("classic");
    styleValues_ += '\0';
    styleValues_ += '\0';
}

void SettingsMenu::Draw()
{
    if (!ImGui::Begin(gettext("Settings"), &visible_))
    {
        ImGui::End();
        return;
    }
    ImGui::InputScalar(gettext("days per week"), ImGuiDataType_U32, &g_settings.daysPerWeek);
    g_settings.daysPerWeek = std::max(1, g_settings.daysPerWeek);
    ImGui::InputScalar(gettext("lessons per day"), ImGuiDataType_U32, &g_settings.lessonsPerDay);
    g_settings.lessonsPerDay = std::max(1, g_settings.lessonsPerDay);
    int styleInt = static_cast<int>(g_settings.style);
    if (ImGui::Combo(gettext("style"), &styleInt, styleValues_.c_str()))
    {
        g_settings.style = static_cast<Style>(styleInt);
        LoadStyle();
    }
    if (ImGui::Combo(gettext("language"), &g_languageId, g_languageValues.c_str()))
    {
        g_settings.language = g_availableLanguages[g_languageId];
        ::ReloadLabels();
    }
    if (ImGui::TreeNode(gettext("Developer options")))
    {
        constexpr int MIN_AUTOSAVE_INTERVAL = 0;
        constexpr int MAX_AUTOSAVE_INTERVAL = 600;
        constexpr int MIN_FONT_SIZE = 5;
        constexpr float MIN_ERROR_BONUS_RATIO = 0.1F;
        constexpr float MAX_ERROR_BONUS_RATIO = 100;
        constexpr int MIN_TIMETABLES_PER_GENERATION_STEP = 1;
        constexpr int MAX_TIMETABLES_PER_GENERATION_STEP = 100;
        constexpr int MIN_MIN_TIMETABLES_PER_GENERATION = 10;
        constexpr int MAX_MIN_TIMETABLES_PER_GENERATION = 10000;
        constexpr int MIN_MAX_TIMETABLES_PER_GENERATION = 10;
        constexpr int MAX_MAX_TIMETABLES_PER_GENERATION = 10000;
        constexpr int MIN_MAX_ITERATIONS = -1;
        constexpr int MAX_MAX_ITERATIONS = 10000;
        constexpr int MIN_ADDITIONAL_BONUS_POINTS = 0;
        constexpr int MAX_ADDITIONAL_BONUS_POINTS = 100;

        ImGui::Checkbox(gettext("vsync"), &g_settings.vsync);
        ImGui::Checkbox(gettext("merged-font"), &g_settings.mergedFont);
        ImGui::SliderInt(gettext("timetable-autosave-interval"), &g_settings.autosaveInterval,
                         MIN_AUTOSAVE_INTERVAL, MAX_AUTOSAVE_INTERVAL);
        ImGui::InputInt(gettext("font-size"), &g_settings.fontSize);
        g_settings.fontSize = std::max(MIN_FONT_SIZE, g_settings.fontSize);
        ImGui::SliderFloat(gettext("error-bonus-ratio"), &g_settings.errorBonusRatio,
                           MIN_ERROR_BONUS_RATIO, MAX_ERROR_BONUS_RATIO);
        ImGui::SliderInt(gettext("timetables-per-generation-step"),
                         &g_settings.timetablesPerGenerationStep,
                         MIN_TIMETABLES_PER_GENERATION_STEP, MAX_TIMETABLES_PER_GENERATION_STEP);
        ImGui::SliderInt(gettext("min-timetables-per-generation"),
                         &g_settings.minTimetablesPerGeneration, MIN_MIN_TIMETABLES_PER_GENERATION,
                         MAX_MIN_TIMETABLES_PER_GENERATION);
        ImGui::SliderInt(gettext("max-timetables-per-generation"),
                         &g_settings.maxTimetablesPerGeneration, MIN_MAX_TIMETABLES_PER_GENERATION,
                         MAX_MAX_TIMETABLES_PER_GENERATION);
        g_settings.maxTimetablesPerGeneration =
            std::max(g_settings.maxTimetablesPerGeneration, g_settings.minTimetablesPerGeneration);
        ImGui::SliderInt(gettext("max-iterations"), &g_settings.maxIterations, MIN_MAX_ITERATIONS,
                         MAX_MAX_ITERATIONS);
        ImGui::SliderInt(gettext("additional-bonus-points"), &g_settings.additionalBonusPoints,
                         MIN_ADDITIONAL_BONUS_POINTS, MAX_ADDITIONAL_BONUS_POINTS);
        if (ImGui::Checkbox(gettext("verbose-logging"), &g_settings.verboseLogging))
        {
            ToggleVerboseLoggingThreads();
        }
        ImGui::Checkbox(gettext("use-prereleases"), &g_settings.usePrereleases);
        ImGui::TreePop();
    }
    ImGui::End();
}
