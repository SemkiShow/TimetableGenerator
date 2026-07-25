// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Wizard.hpp"
#include "Logging.hpp"
#include "Searching.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/Classes.hpp"
#include "UI/Classrooms.hpp"
#include "UI/Lessons.hpp"
#include "UI/Teachers.hpp"
#include "Widgets/Window.hpp"
#include <functional>
#include <imgui.h>
#include <memory>
#include <thread>
#include <vector>

std::shared_ptr<WizardMenu> g_wizardMenu;

static const char* g_wizardTexts[] = {
    _("Welcome to the TimetableGenerator setup wizard!\n\nThe first step is to add classrooms.\nAfter you are done, press Ok and continue to the next step."),
    _("The next step is to add classes.\nAfter you are done, press Ok and continue to the next step."),
    _("The next step is to add lessons.\nAfter you are done, press Ok and continue to the next step."),
    _("The next step is to add teachers.\nAfter you are done, press Ok and continue to the next step."),
    _("The next step is to assign lessons to classes.\nAfter you are done, press Ok and continue to the next step."),
    _("You are done! Now press the Generate timetable\nbutton to begin the timetable finding process!")};
static std::vector<std::function<Window*()>> g_wizardMenus{
    [] { return g_classroomsMenu.get(); }, [] { return g_classesMenu.get(); },
    [] { return g_lessonsMenu.get(); }, [] { return g_teachersMenu.get(); },
    [] { return g_classesMenu.get(); }};

void WizardMenu::Draw()
{
    if (step_ > 0 && step_ < WIZARD_STEPS && !g_wizardMenus[step_ - 1]()->IsVisible() &&
        openWizard_)
    {
        openWizard_ = false;
        Open();
    }

    if (!ImGui::Begin(gettext("Setup wizard"), &visible_))
    {
        ImGui::End();
        return;
    }

    ImGui::ProgressBar((float)step_ / (WIZARD_STEPS - 1));
    ImGui::Text("Step %d", step_ + 1);
    ImGui::Text("%s", gettext(g_wizardTexts[step_]));
    if (step_ > 0 && ImGui::Button(gettext("Back"))) step_--;
    if (step_ > 0) ImGui::SameLine();
    if (step_ == WIZARD_STEPS - 1)
    {
        if (ImGui::Button(gettext("Generate timetable")))
        {
            Close();
            std::thread beginSearchingThread(BeginSearching, g_currentTimetable);
            beginSearchingThread.detach();
        }
    }
    else
    {
        if (ImGui::Button(gettext("Next")))
        {
            LogInfo("Clicked Next in the wizard menu while on step %d", step_);
            if (step_ < WIZARD_STEPS - 1)
            {
                g_wizardMenus[step_]()->Open();
                openWizard_ = true;
                Close();
            }
            step_++;
            if (step_ >= WIZARD_STEPS) Close();
        }
    }
    ImGui::End();
}
