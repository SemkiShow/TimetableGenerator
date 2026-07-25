// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "UI/Classes/Edit.hpp"
#include "UI/Classes/Utils.hpp"
#include <algorithm>
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <string>

std::shared_ptr<ClassesMenu> g_classesMenu;

void ClassesMenu::Draw()
{
    if (!ImGui::Begin(gettext("Classes"), &visible_))
    {
        ImGui::End();
        return;
    }

    ImGui::TextColored(
        COLOR_WARNING, "%s",
        gettext(
            "Warning: changing the current year can be quite destructive.\nIf something went wrong, press the Cancel button to revert all changes"));
    if (ImGui::Button(gettext("Back"))) ShiftClasses(timetable_, -1);
    ImGui::SameLine();
    ImGui::Text("%d", timetable_.year);
    ImGui::SameLine();
    if (ImGui::Button(gettext("Next"))) ShiftClasses(timetable_, 1);
    ImGui::Separator();

    if (ImGui::Button(gettext("+")))
    {
        LogInfo("Adding a new class with id %d", timetable_.maxClassId + 1);
        g_editClassMenu->Open(&timetable_, true, timetable_.maxClassId + 1, true, 1);
    }
    ImGui::Separator();

    ImGui::Columns(2);
    std::string lastClassNumber;
    int buttonId = 0;
    for (size_t i = 0; i < timetable_.orderedClasses.size(); i++)
    {
        if (lastClassNumber != timetable_.classes[timetable_.orderedClasses[i]].number)
        {
            lastClassNumber = timetable_.classes[timetable_.orderedClasses[i]].number;
            ImGui::PushID(buttonId);

            if (ImGui::Button(gettext("-")))
            {
                LogInfo("Removed classes with number %s", lastClassNumber.c_str());
                ImGui::PopID();
                for (auto it = timetable_.classes.begin(); it != timetable_.classes.end();)
                {
                    if (it->second.number == lastClassNumber)
                    {
                        timetable_.orderedClasses.erase(std::find(timetable_.orderedClasses.begin(),
                                                                  timetable_.orderedClasses.end(),
                                                                  it->first));
                        it = timetable_.classes.erase(it);
                        continue;
                    }
                    ++it;
                }
                break;
            }
            ImGui::SameLine();

            if (ImGui::Button(gettext("Edit")))
            {
                LogInfo("Bulk editing classes with number %s", lastClassNumber.c_str());
                int bulkCount = 0;
                for (auto& classPair: timetable_.classes)
                {
                    if (classPair.second.number == lastClassNumber) bulkCount++;
                }
                g_editClassMenu->Open(&timetable_, false, timetable_.orderedClasses[i], true,
                                      bulkCount);
            }
            ImGui::SameLine();

            ImGui::Text("%s", lastClassNumber.c_str());
            ImGui::Indent();
            if (ImGui::Button(gettext("+")))
            {
                int classId = 0;
                for (size_t j = 0; j < timetable_.orderedClasses.size(); j++)
                {
                    if (timetable_.classes[timetable_.orderedClasses[j]].number == lastClassNumber)
                        classId = (int)j;
                }
                classId++;
                LogInfo("Adding a new class with number %s and id %d", lastClassNumber.c_str(),
                        classId);
                g_editClassMenu->Open(&timetable_, true, classId, false, 0);
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
            LogInfo("Removed a class with id %d", timetable_.orderedClasses[i]);
            timetable_.classes.erase(timetable_.orderedClasses[i]);
            timetable_.orderedClasses.erase(timetable_.orderedClasses.begin() + (int)i);
            i--;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing class with id %d", timetable_.orderedClasses[i]);
            g_editClassMenu->Open(&timetable_, false, timetable_.orderedClasses[i], false, 0);
        }
        ImGui::SameLine();

        ImGui::Text("%s%s", timetable_.classes[timetable_.orderedClasses[i]].number.c_str(),
                    timetable_.classes[timetable_.orderedClasses[i]].letter.c_str());
        ImGui::PopID();
        buttonId++;
        ImGui::Unindent();
        ImGui::NextColumn();

        if (prevTimetable_->teachers.find(
                timetable_.classes[timetable_.orderedClasses[i]].teacherId) !=
            prevTimetable_->teachers.end())
            ImGui::LabelText(
                "", "%s",
                prevTimetable_->teachers[timetable_.classes[timetable_.orderedClasses[i]].teacherId]
                    .name.c_str());
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok in the classes menu");
        prevTimetable_->classes = timetable_.classes;
        prevTimetable_->maxClassId = timetable_.maxClassId;
        prevTimetable_->orderedClasses = timetable_.orderedClasses;
        prevTimetable_->lessons = timetable_.lessons;
        prevTimetable_->year = timetable_.year;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
