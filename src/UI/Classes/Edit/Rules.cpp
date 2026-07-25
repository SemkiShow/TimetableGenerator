// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes/Edit/Rules.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI/Classes/Utils.hpp"
#include "Widgets/Window.hpp"
#include <algorithm>
#include <climits>
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <string>
#include <unordered_map>

std::shared_ptr<RulesMenu> g_rulesMenu;

void RulesMenu::Open(Timetable* prevTimetable, bool newRule, int ruleId, int classId,
                     const std::unordered_map<std::string, bool>& lessons,
                     const std::unordered_map<std::string, int>& lessonCounts,
                     const std::unordered_map<std::string, bool>& lessonTeachers)
{
    this->prevTimetable_ = prevTimetable;
    this->newRule_ = newRule;
    this->ruleId_ = ruleId;
    this->classId_ = classId;
    this->lessons_ = lessons;
    this->lessonCounts_ = lessonCounts;
    this->lessonTeachers_ = lessonTeachers;

    timetable_ = *prevTimetable;
    if (newRule) timetable_.classes[classId].timetableLessonRules.emplace_back();

    LoadTimetableLessonsFromSelection(timetable_, classId, this->lessons_, this->lessonCounts_,
                                      this->lessonTeachers_);

    timetableLessonValues_ = "";
    for (auto& lesson: timetable_.classes[classId].timetableLessons)
    {
        for (size_t i = 0; i < lesson.second.lessonTeacherPairs.size(); i++)
        {
            LessonTeacherPair& lessonTeacherPair = lesson.second.lessonTeacherPairs[i];
            timetableLessonValues_ +=
                timetable_.lessons[lessonTeacherPair.lessonId].name + " (" +
                g_currentTimetable.teachers[lessonTeacherPair.teacherId].name + ")";
            if (i < lesson.second.lessonTeacherPairs.size() - 1) timetableLessonValues_ += ", ";
        }
        timetableLessonValues_ += '\0';
    }

    minTimetableLessonCount_ = INT_MAX;

    Window::Open();
}

void RulesMenu::Draw()
{
    if (!ImGui::Begin((newRule_ ? gettext("New rule") : gettext("Edit rule")), &visible_))
    {
        ImGui::End();
        return;
    }

    TimetableLessonRule& rule = timetable_.classes[classId_].timetableLessonRules[ruleId_];

    ImGui::Checkbox(gettext("preserve order"), &rule.preserveOrder);
    ImGui::InputInt(gettext("count"), &rule.count);
    rule.count = std::max(rule.count, 1);
    rule.count = std::min(rule.count, minTimetableLessonCount_);

    if (ImGui::Button(gettext("+")))
    {
        rule.timetableLessonIds.push_back(-1);
    }

    for (size_t i = 0; i < rule.timetableLessonIds.size(); i++)
    {
        ImGui::PushID((int)i);
        if (ImGui::Button(gettext("-")))
        {
            rule.timetableLessonIds.erase(rule.timetableLessonIds.begin() + (int)i);
            i--;
            ImGui::PopID();
            continue;
        }
        ImGui::SameLine();
        if (ImGui::Combo("", &rule.timetableLessonIds[i], timetableLessonValues_.c_str()))
        {
            minTimetableLessonCount_ = INT_MAX;
            for (size_t i = 0; i < rule.timetableLessonIds.size(); i++)
            {
                int timetableLessonId = rule.timetableLessonIds[i];
                if (timetableLessonId < 0) continue;
                auto& timetableLesson =
                    timetable_.classes[classId_].timetableLessons[timetableLessonId];
                minTimetableLessonCount_ =
                    std::min(minTimetableLessonCount_, timetableLesson.count);
            }
        }
        ImGui::PopID();
    }

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed the Ok button in rules of class with id %d", classId_);
        auto& rule = timetable_.classes[classId_].timetableLessonRules[ruleId_];
        if (newRule_)
        {
            prevTimetable_->classes[classId_].timetableLessonRules.push_back(rule);
        }
        else
        {
            prevTimetable_->classes[classId_].timetableLessonRules[ruleId_] = rule;
        }
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
