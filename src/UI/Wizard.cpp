#include "Searching.hpp"
#include "Timetable.hpp"
#include "UI.hpp"
#include <imgui.h>

#define WIZARD_STEPS 6

bool isWizard = false;
int wizardStep = 0;
const char* wizardTexts[WIZARD_STEPS] = {
    "Welcome to the TimetableGenerator setup wizard!\n\nThe first step is to setup classrooms.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add classes.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add lessons.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add teachers.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to assign lessons to classes.\nAfter you are done, press Ok and continue to the next step.",
    "You are done! Now press the Generate timetable\nbutton to begin the timetable finding process!"
};
void (*wizardMenus[])() = {OpenClassrooms, OpenClasses, OpenLessons, OpenTeachers, OpenClasses};
bool* wizardToggles[] = {&isClassrooms, &isClasses, &isLessons, &isTeachers, &isClasses};
bool openWizard = false;

void ShowWizard(bool* isOpen)
{
    if (wizardStep > 0 && wizardStep < WIZARD_STEPS && !*wizardToggles[wizardStep-1] && openWizard)
    {
        openWizard = false;
        *isOpen = true;
    }
    if (!*isOpen) return;
    if (!ImGui::Begin("Setup wizard", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::ProgressBar(wizardStep * 1.0 / (WIZARD_STEPS-1));
    ImGui::Text("Step %d", wizardStep + 1);
    ImGui::Text("%s", wizardTexts[wizardStep]);
    if (wizardStep > 0 && ImGui::Button("Back")) wizardStep--;
    if (wizardStep > 0) ImGui::SameLine();
    if (wizardStep == WIZARD_STEPS - 1)
    {
        if (ImGui::Button("Generate timetable"))
        {
            *isOpen = false;
            BeginSearching(&currentTimetable);
        }
    }
    else
    {
        if (ImGui::Button("Next"))
        {
            if (wizardStep < WIZARD_STEPS-1)
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
