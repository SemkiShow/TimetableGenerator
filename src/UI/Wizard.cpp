// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Searching.hpp"
#include "Settings.hpp"
#include "UI.hpp"
#include <imgui.h>
#include <string>
#include <thread>

int wizardStep = 0;
std::string wizardTexts[WIZARD_STEPS] = {
    "Welcome to the TimetableGenerator setup wizard!\n\nThe first step is to setup classrooms.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add classes.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add lessons.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add teachers.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to assign lessons to classes.\nAfter you are done, press Ok and continue to the next step.",
    "You are done! Now press the Generate timetable\nbutton to begin the timetable finding process!"};
void (*wizardMenus[])() = {OpenClassrooms, OpenClasses, OpenLessons, OpenTeachers, OpenClasses};
bool* wizardToggles[] = {&isClassrooms, &isClasses, &isLessons, &isTeachers, &isClasses};
bool openWizard = false;

bool isWizard = false;
void ShowWizard(bool* isOpen)
{
    if (wizardStep > 0 && wizardStep < WIZARD_STEPS && !*wizardToggles[wizardStep - 1] &&
        openWizard)
    {
        openWizard = false;
        *isOpen = true;
    }
    if (!*isOpen) return;

    if (!ImGui::Begin(labels["Setup wizard"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::ProgressBar(wizardStep * 1.0 / (WIZARD_STEPS - 1));
    ImGui::Text("Step %d", wizardStep + 1);
    ImGui::Text("%s", wizardTexts[wizardStep].c_str());
    if (wizardStep > 0 && ImGui::Button(labels["Back"].c_str())) wizardStep--;
    if (wizardStep > 0) ImGui::SameLine();
    if (wizardStep == WIZARD_STEPS - 1)
    {
        if (ImGui::Button(labels["Generate timetable"].c_str()))
        {
            *isOpen = false;
            std::thread beginSearchingThread(BeginSearching, currentTimetable);
            beginSearchingThread.detach();
        }
    }
    else
    {
        if (ImGui::Button(labels["Next"].c_str()))
        {
            LogInfo("Clicked Next in the wizard menu while on step " + std::to_string(wizardStep));
            if (wizardStep < WIZARD_STEPS - 1)
            {
                wizardMenus[wizardStep]();
                openWizard = true;
                *isOpen = false;
            }
            wizardStep++;
            if (wizardStep >= WIZARD_STEPS) *isOpen = false;
        }
    }
    ImGui::End();
}
