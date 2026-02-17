// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Settings.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <imgui.h>
#include <thread>

std::shared_ptr<SettingsMenu> settingsMenu;

void SettingsMenu::ReloadLabels()
{
    styleValues = "";
    styleValues += GetText("dark");
    styleValues += '\0';
    styleValues += GetText("light");
    styleValues += '\0';
    styleValues += GetText("classic");
    styleValues += '\0';
    styleValues += '\0';
}

void SettingsMenu::Draw()
{
    if (!ImGui::Begin(gettext("Settings"), &visible))
    {
        ImGui::End();
        return;
    }
    ImGui::InputScalar(gettext("days per week"), ImGuiDataType_U32, &daysPerWeek);
    daysPerWeek = std::max(1u, daysPerWeek);
    ImGui::InputScalar(gettext("lessons per day"), ImGuiDataType_U32, &lessonsPerDay);
    lessonsPerDay = std::max(1u, lessonsPerDay);
    int styleInt = static_cast<int>(style);
    if (ImGui::Combo(gettext("style"), &styleInt, styleValues.c_str()))
    {
        style = static_cast<Style>(styleInt);
        LoadStyle();
    }
    if (ImGui::Combo(gettext("language"), &languageId, languageValues.c_str()))
    {
        language = availableLanguages[languageId];
        ::ReloadLabels();
    }
    if (ImGui::TreeNode(gettext("Developer options")))
    {
        ImGui::Checkbox(gettext("vsync"), &vsync);
        ImGui::Checkbox(gettext("merged-font"), &mergedFont);
        ImGui::SliderInt(gettext("timetable-autosave-interval"), &timetableAutosaveInterval, 0,
                         600);
        ImGui::InputInt(gettext("font-size"), &fontSize);
        fontSize = std::max(5, fontSize);
        ImGui::SliderFloat(gettext("error-bonus-ratio"), &errorBonusRatio, 0.1f, 100.0f);
        ImGui::SliderInt(gettext("timetables-per-generation-step"), &timetablesPerGenerationStep, 1,
                         100);
        ImGui::SliderInt(gettext("min-timetables-per-generation"), &minTimetablesPerGeneration, 10,
                         10000);
        ImGui::SliderInt(gettext("max-timetables-per-generation"), &maxTimetablesPerGeneration, 10,
                         10000);
        if (maxTimetablesPerGeneration < minTimetablesPerGeneration)
            maxTimetablesPerGeneration = minTimetablesPerGeneration;
        ImGui::SliderInt(gettext("max-iterations"), &maxIterations, -1, 10000);
        ImGui::SliderInt(gettext("additional-bonus-points"), &additionalBonusPoints, 0, 100);
        if (ImGui::Checkbox(gettext("verbose-logging"), &verboseLogging))
        {
            while (iterationData.threadLock) COMPILER_BARRIER();
            iterationData.threadLock = true;
            if (verboseLogging)
                threadsNumber = 1;
            else
                threadsNumber = std::max(1u, std::thread::hardware_concurrency());
            iterationData.threadLock = false;
        }
        ImGui::Checkbox(gettext("use-prereleases"), &usePrereleases);
        ImGui::TreePop();
    }
    ImGui::End();
}
