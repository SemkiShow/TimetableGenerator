// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/Classes/Edit.hpp"
#include "UI/Classes/Utils.hpp"
#include <algorithm>
#include <imgui.h>

std::shared_ptr<ClassesMenu> classesMenu;

void ClassesMenu::Draw()
{
    if (!ImGui::Begin(gettext("Classes"), &visible))
    {
        ImGui::End();
        return;
    }

    ImGui::TextColored(
        ImVec4(255, 255, 0, 255), "%s",
        gettext(
            "Warning: changing the current year can be quite destructive.\nIf something went wrong, press the Cancel button to revert all changes"));
    if (ImGui::Button(gettext("Back"))) ShiftClasses(timetable, -1);
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(timetable.year).c_str());
    ImGui::SameLine();
    if (ImGui::Button(gettext("Next"))) ShiftClasses(timetable, 1);
    ImGui::Separator();

    if (ImGui::Button(gettext("+")))
    {
        LogInfo("Adding a new class with id " + std::to_string(timetable.maxClassId + 1));
        editClassMenu->Open(&timetable, true, timetable.maxClassId + 1, true, 1);
    }
    ImGui::Separator();

    ImGui::Columns(2);
    std::string lastClassNumber = "";
    int buttonId = 0;
    for (size_t i = 0; i < timetable.orderedClasses.size(); i++)
    {
        if (lastClassNumber != timetable.classes[timetable.orderedClasses[i]].number)
        {
            lastClassNumber = timetable.classes[timetable.orderedClasses[i]].number;
            ImGui::PushID(buttonId);

            if (ImGui::Button(gettext("-")))
            {
                LogInfo("Removed classes with number " + lastClassNumber);
                ImGui::PopID();
                for (auto it = timetable.classes.begin(); it != timetable.classes.end();)
                {
                    if (it->second.number == lastClassNumber)
                    {
                        timetable.orderedClasses.erase(std::find(timetable.orderedClasses.begin(),
                                                                 timetable.orderedClasses.end(),
                                                                 it->first));
                        it = timetable.classes.erase(it);
                        continue;
                    }
                    ++it;
                }
                break;
            }
            ImGui::SameLine();

            if (ImGui::Button(gettext("Edit")))
            {
                LogInfo("Bulk editing classes with number " + lastClassNumber);
                int bulkAmount = 0;
                for (auto& classPair: timetable.classes)
                {
                    if (classPair.second.number == lastClassNumber) bulkAmount++;
                }
                editClassMenu->Open(&timetable, false, timetable.orderedClasses[i], true,
                                    bulkAmount);
            }
            ImGui::SameLine();

            ImGui::Text("%s", lastClassNumber.c_str());
            ImGui::Indent();
            if (ImGui::Button(gettext("+")))
            {
                int classId = 0;
                for (size_t j = 0; j < timetable.orderedClasses.size(); j++)
                {
                    if (timetable.classes[timetable.orderedClasses[j]].number == lastClassNumber)
                        classId = j;
                }
                classId++;
                LogInfo("Adding a new class with number " + lastClassNumber + " and Id " +
                        std::to_string(classId));
                editClassMenu->Open(&timetable, true, classId, false, 0);
            }
            ImGui::Unindent();
            ImGui::NextColumn();
            ImGui::LabelText("", "%s", "");
            ImGui::NextColumn();
            ImGui::PopID();
            buttonId++;
        }
        ImGui::Indent();
        ImGui::PushID(buttonId);

        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a class with id " + std::to_string(timetable.orderedClasses[i]));
            timetable.classes.erase(timetable.orderedClasses[i]);
            timetable.orderedClasses.erase(timetable.orderedClasses.begin() + i);
            i--;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing class with id " + std::to_string(timetable.orderedClasses[i]));
            editClassMenu->Open(&timetable, false, timetable.orderedClasses[i], false, 0);
        }
        ImGui::SameLine();

        ImGui::Text("%s", (timetable.classes[timetable.orderedClasses[i]].number +
                           timetable.classes[timetable.orderedClasses[i]].letter)
                              .c_str());
        ImGui::PopID();
        buttonId++;
        ImGui::Unindent();
        ImGui::NextColumn();

        if (prevTimetable->teachers.find(
                timetable.classes[timetable.orderedClasses[i]].teacherId) !=
            prevTimetable->teachers.end())
            ImGui::LabelText(
                "", "%s",
                prevTimetable->teachers[timetable.classes[timetable.orderedClasses[i]].teacherId]
                    .name.c_str());
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok in the classes menu");
        prevTimetable->classes = timetable.classes;
        prevTimetable->maxClassId = timetable.maxClassId;
        prevTimetable->orderedClasses = timetable.orderedClasses;
        prevTimetable->lessons = timetable.lessons;
        prevTimetable->year = timetable.year;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
