// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Timetable/Generate.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include <cmath>
#include <imgui.h>
#include <thread>

std::shared_ptr<GenerateTimetableMenu> generateTimetableMenu;

void GenerateTimetableMenu::Draw()
{
    if (!ImGui::Begin(gettext("Generate timetable"), &visible))
    {
        ImGui::End();
        return;
    }
    if (status == gettext("Timetable generating done!"))
    {
        ImGui::TextColored(ImVec4(0, 255, 0, 255), "%s", gettext("Timetable generating done!"));
    }
    else
    {
        ImGui::Text("%s", status.c_str());
    }
    if (status == gettext("Allocating memory for the timetables..."))
    {
        ImGui::LabelText("##1", "\n\n\n\n\n\n\n");
    }
    else
    {
        ImGui::Text(
            "%s", (GetText("Iteration:") + " " + std::to_string(iterationData.iteration)).c_str());
        ImGui::Text("%s", (GetText("The best score is") + " " +
                           std::to_string(iterationData.allTimeBestScore))
                              .c_str());
        ImGui::Text("%s", (GetText("The best timetable has") + " " +
                           std::to_string(
                               iterationData.timetables[iterationData.bestTimetableIndex].errors) +
                           " " + gettext("errors"))
                              .c_str());
        ImGui::Text("%s",
                    (GetText("The best timetable has") + " " +
                     std::to_string(
                         iterationData.timetables[iterationData.bestTimetableIndex].bonusPoints) +
                     " " + gettext("bonus points"))
                        .c_str());
        ImGui::Text("%s", (std::to_string(iterationData.iterationsPerChange) + " " +
                           gettext("iterations have passed since last score improvement"))
                              .c_str());
        float progressPercentage = 1;
        if (status == gettext("Generating a timetable that matches the requirements..."))
        {
            progressPercentage =
                (-(iterationData.maxErrors * 1.0f / 100) * iterationData.minErrors + 100) / 100;
        }
        else if (status == gettext("Finding additional bonus points..."))
        {
            progressPercentage = (iterationData.maxBonusPoints - iterationData.startBonusPoints) *
                                 1.0f / additionalBonusPoints;
        }
        ImGui::ProgressBar(pow(progressPercentage, 2));
        ImGui::PlotLines(gettext("errors"), iterationData.errorValues,
                         iterationData.errorValuesPoints, 0, NULL, FLT_MAX, FLT_MAX,
                         ImVec2(0, 100));
    }
    ImGui::End();
}

void GenerateTimetableMenu::PostDraw()
{
    // Stop searching for a timetable if the Generate timetable window is closed
    if (!IsVisible())
    {
        iterationData.isDone = true;
        while (iterationData.threadLock) COMPILER_BARRIER();
        std::thread stopSearchingThread(StopSearching);
        stopSearchingThread.detach();
    }
}
