// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Timetable/Generate.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <cfloat>
#include <cmath>
#include <imgui.h>
#include <memory>
#include <string>
#include <thread>

std::shared_ptr<GenerateTimetableMenu> g_generateTimetableMenu;

void GenerateTimetableMenu::Draw()
{
    if (!ImGui::Begin(gettext("Generate timetable"), &visible_))
    {
        ImGui::End();
        return;
    }
    if (status_ == gettext("Timetable generating done!"))
    {
        ImGui::TextColored(COLOR_SUCCESS, "%s", gettext("Timetable generating done!"));
    }
    else
    {
        ImGui::Text("%s", status_.c_str());
    }
    if (status_ == gettext("Allocating memory for the timetables..."))
    {
        ImGui::LabelText("##1", "\n\n\n\n\n\n\n");
    }
    else
    {
        ImGui::Text("%s %d", gettext("Iteration:"), g_iterationData.iteration);
        ImGui::Text("%s %d", gettext("The best score is"), g_iterationData.allTimeBestScore);
        ImGui::Text("%s %d %s", gettext("The best timetable has"),
                    g_iterationData.timetables[g_iterationData.bestTimetableIndex].errors,
                    gettext("errors"));
        ImGui::Text("%s %d %s", gettext("The best timetable has"),
                    g_iterationData.timetables[g_iterationData.bestTimetableIndex].bonusPoints,
                    gettext("bonus points"));
        ImGui::Text("%d %s", g_iterationData.iterationsPerChange,
                    gettext("iterations have passed since last score improvement"));
        float progressPercentage = 1;
        if (status_ == gettext("Generating a timetable that matches the requirements..."))
        {
            progressPercentage =
                1.0F - (float)g_iterationData.minErrors / (float)g_iterationData.maxErrors;
        }
        else if (status_ == gettext("Finding additional bonus points..."))
        {
            progressPercentage =
                float(g_iterationData.maxBonusPoints - g_iterationData.startBonusPoints) /
                (float)g_settings.additionalBonusPoints;
        }
        ImGui::ProgressBar(powf(progressPercentage, 2));
        constexpr int PLOT_HEIGHT = 100;
        ImGui::PlotLines(gettext("errors"), g_iterationData.errorValues,
                         IterationData::ERROR_VALUES_SIZE, 0, nullptr, FLT_MAX, FLT_MAX,
                         ImVec2(0, PLOT_HEIGHT));
    }
    ImGui::End();

    // Stop searching for a timetable if the Generate timetable window is closed
    if (!IsVisible())
    {
        std::thread stopSearchingThread(StopSearching);
        stopSearchingThread.detach();
    }
}
