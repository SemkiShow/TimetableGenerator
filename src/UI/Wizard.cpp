#include "UI.hpp"

#define WIZARD_STEPS 6

bool isWizard = false;
int wizardStep = 0;
const char* wizardTexts[WIZARD_STEPS] = {
    "Welcome to the TimetableGenerator setup wizard!\n\nThe first step is to setup classrooms.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add classes.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add lessons.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to add teachers.\nAfter you are done, press Ok and continue to the next step.",
    "The next step is to assign lessons to classes.\nAfter you are done, press Ok and continue to the next step.",
    "You are done! Now press the Generate timetable button to begin the timetable finding process!"
};
bool* wizardMenus[WIZARD_STEPS] = {&isClassrooms, &isClasses, &isLessons, &isTeachers, &isClasses};

void ShowWizard(bool* isOpen)
{
    if (!ImGui::Begin("Setup wizard", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::ProgressBar(wizardStep * 1.0 / (WIZARD_STEPS-1));
    ImGui::Text(("Step " + std::to_string(wizardStep + 1)).c_str());
    ImGui::Text(wizardTexts[wizardStep]);
    if (wizardStep > 0 && ImGui::Button("Back"))
    {
        wizardStep--;
    }
    if (wizardStep > 0) ImGui::SameLine();
    if (ImGui::Button("Next"))
    {
        if (wizardStep < WIZARD_STEPS-1) *wizardMenus[wizardStep] = true;
        wizardStep++;
        if (wizardStep >= WIZARD_STEPS) *isOpen = false;
    }
    ImGui::End();
}
