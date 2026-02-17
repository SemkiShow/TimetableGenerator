// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes/Edit/Rules.hpp"
#include "Logging.hpp"
#include "Translations.hpp"
#include "UI/Classes/Utils.hpp"
#include <imgui.h>

std::shared_ptr<RulesMenu> rulesMenu;

void RulesMenu::Open(Timetable* prevTimetable, bool newRule, int ruleId, int classId,
                     const std::unordered_map<std::string, bool>& lessons,
                     const std::unordered_map<std::string, int>& lessonAmounts,
                     const std::unordered_map<std::string, bool>& lessonTeachers)
{
    this->prevTimetable = prevTimetable;
    this->newRule = newRule;
    this->ruleId = ruleId;
    this->classId = classId;
    this->lessons = lessons;
    this->lessonAmounts = lessonAmounts;
    this->lessonTeachers = lessonTeachers;

    timetable = *prevTimetable;
    if (newRule) timetable.classes[classId].timetableLessonRules.emplace_back();

    LoadTimetableLessonsFromSelection(timetable, classId, this->lessons, this->lessonAmounts,
                                      this->lessonTeachers);

    timetableLessonValues = "";
    for (auto& lesson: timetable.classes[classId].timetableLessons)
    {
        for (size_t i = 0; i < lesson.second.lessonTeacherPairs.size(); i++)
        {
            LessonTeacherPair& lessonTeacherPair = lesson.second.lessonTeacherPairs[i];
            timetableLessonValues += timetable.lessons[lessonTeacherPair.lessonId].name + " (" +
                                     currentTimetable.teachers[lessonTeacherPair.teacherId].name +
                                     ")";
            if (i < lesson.second.lessonTeacherPairs.size() - 1) timetableLessonValues += ", ";
        }
        timetableLessonValues += '\0';
    }

    minTimetableLessonAmount = INT_MAX;

    Window::Open();
}

void RulesMenu::Draw()
{
    if (!ImGui::Begin((newRule ? gettext("New rule") : gettext("Edit rule")), &visible))
    {
        ImGui::End();
        return;
    }

    TimetableLessonRule& rule = timetable.classes[classId].timetableLessonRules[ruleId];

    ImGui::Checkbox(gettext("preserve order"), &rule.preserveOrder);
    ImGui::InputInt(gettext("amount"), &rule.amount);
    if (rule.amount < 1) rule.amount = 1;
    if (rule.amount > minTimetableLessonAmount) rule.amount = minTimetableLessonAmount;

    if (ImGui::Button(gettext("+")))
    {
        rule.timetableLessonIds.push_back(-1);
    }

    for (size_t i = 0; i < rule.timetableLessonIds.size(); i++)
    {
        ImGui::PushID(i);
        if (ImGui::Button(gettext("-")))
        {
            rule.timetableLessonIds.erase(rule.timetableLessonIds.begin() + i);
            i--;
            ImGui::PopID();
            continue;
        }
        ImGui::SameLine();
        if (ImGui::Combo("", &rule.timetableLessonIds[i], timetableLessonValues.c_str()))
        {
            minTimetableLessonAmount = INT_MAX;
            for (size_t i = 0; i < rule.timetableLessonIds.size(); i++)
            {
                int timetableLessonId = rule.timetableLessonIds[i];
                if (timetableLessonId < 0) continue;
                auto& timetableLesson =
                    timetable.classes[classId].timetableLessons[timetableLessonId];
                minTimetableLessonAmount =
                    std::min(minTimetableLessonAmount, timetableLesson.amount);
            }
        }
        ImGui::PopID();
    }

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed the Ok button in rules of class with id " + std::to_string(classId));
        auto& rule = timetable.classes[classId].timetableLessonRules[ruleId];
        if (newRule)
        {
            prevTimetable->classes[classId].timetableLessonRules.push_back(rule);
        }
        else
        {
            prevTimetable->classes[classId].timetableLessonRules[ruleId] = rule;
        }
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
