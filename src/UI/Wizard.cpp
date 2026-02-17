// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Wizard.hpp"
#include "Logging.hpp"
#include "Searching.hpp"
#include "Translations.hpp"
#include "UI/Classes.hpp"
#include "UI/Classrooms.hpp"
#include "UI/Lessons.hpp"
#include "UI/Teachers.hpp"
#include <functional>
#include <imgui.h>
#include <string>
#include <thread>

const int wizardSteps = 6;

std::shared_ptr<WizardMenu> wizardMenu;

int wizardStep = 0;
const char* wizardTexts[] = {
    _("Welcome to the TimetableGenerator setup wizard!\n\nThe first step is to add classrooms.\nAfter you are done, press Ok and continue to the next step."),
    _("The next step is to add classes.\nAfter you are done, press Ok and continue to the next step."),
    _("The next step is to add lessons.\nAfter you are done, press Ok and continue to the next step."),
    _("The next step is to add teachers.\nAfter you are done, press Ok and continue to the next step."),
    _("The next step is to assign lessons to classes.\nAfter you are done, press Ok and continue to the next step."),
    _("You are done! Now press the Generate timetable\nbutton to begin the timetable finding process!")};
std::vector<std::function<Window*()>> wizardMenus{
    [] { return classroomsMenu.get(); }, [] { return classesMenu.get(); },
    [] { return lessonsMenu.get(); }, [] { return teachersMenu.get(); },
    [] { return classesMenu.get(); }};
bool openWizard = false;

void WizardMenu::Draw()
{
    if (wizardStep > 0 && wizardStep < wizardSteps && !wizardMenus[wizardStep - 1]()->IsVisible() &&
        openWizard)
    {
        openWizard = false;
        Open();
    }

    if (!ImGui::Begin(gettext("Setup wizard"), &visible))
    {
        ImGui::End();
        return;
    }

    ImGui::ProgressBar(wizardStep * 1.0 / (wizardSteps - 1));
    ImGui::Text("Step %d", wizardStep + 1);
    ImGui::Text("%s", GetText(wizardTexts[wizardStep]).c_str());
    if (wizardStep > 0 && ImGui::Button(gettext("Back"))) wizardStep--;
    if (wizardStep > 0) ImGui::SameLine();
    if (wizardStep == wizardSteps - 1)
    {
        if (ImGui::Button(gettext("Generate timetable")))
        {
            Close();
            std::thread beginSearchingThread(BeginSearching, currentTimetable);
            beginSearchingThread.detach();
        }
    }
    else
    {
        if (ImGui::Button(gettext("Next")))
        {
            LogInfo("Clicked Next in the wizard menu while on step " + std::to_string(wizardStep));
            if (wizardStep < wizardSteps - 1)
            {
                wizardMenus[wizardStep]()->Open();
                openWizard = true;
                Close();
            }
            wizardStep++;
            if (wizardStep >= wizardSteps) Close();
        }
    }
    ImGui::End();
}
