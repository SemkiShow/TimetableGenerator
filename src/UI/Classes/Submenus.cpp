// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "UI/Classes.hpp"
#include <climits>
#include <imgui.h>
#include <vector>

int currentLessonId = 0;
std::unordered_map<int, bool> combinedLessonLessons;
std::unordered_map<std::string, bool> combinedLessonTeachers;
bool newCombinedLesson = false;
bool isCombineLessons = false;

void ResetCombineLessonsVariables()
{
    for (auto& lesson: tmpLessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        combinedLessonLessons[lesson.first] = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            combinedLessonTeachers[std::to_string(lesson.first) + teacher.second.name] = false;
        }
    }
    if (!newCombinedLesson)
    {
        TimetableLesson& currentLesson =
            tmpTmpTimetable.classes[currentClassId].timetableLessons[currentLessonId];
        for (size_t j = 0; j < currentLesson.lessonTeacherPairs.size(); j++)
        {
            if (!classLessons[std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) + "0"])
                continue;
            if (!classLessonTeachers
                    [std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) +
                     currentTimetable.teachers[currentLesson.lessonTeacherPairs[j].teacherId].name +
                     "0"])
                continue;
            combinedLessonLessons[currentLesson.lessonTeacherPairs[j].lessonId] = true;
            combinedLessonTeachers
                [std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) +
                 currentTimetable.teachers[currentLesson.lessonTeacherPairs[j].teacherId].name] =
                    true;
        }
    }
}

void ShowCombineLessons(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Combine lessons"), isOpen))
    {
        ImGui::End();
        return;
    }

    // Lessons
    ImGui::Columns(2);
    int pushId = 0;
    for (auto& lesson: tmpLessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        ImGui::PushID(pushId);
        ImGui::Checkbox(lesson.second.name.c_str(), &combinedLessonLessons[lesson.first]);
        ImGui::NextColumn();
        ImGui::PopID();
        pushId++;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            ImGui::PushID(pushId);
            ImGui::Checkbox(
                teacher.second.name.c_str(),
                &combinedLessonTeachers[std::to_string(lesson.first) + teacher.second.name]);
            ImGui::PopID();
            pushId++;
        }
        ImGui::NextColumn();
        ImGui::Separator();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed the Ok button in combine lessons of class with Id " +
                std::to_string(currentClassId));
        tmpTmpTimetable.classes[currentClassId]
            .timetableLessons[currentLessonId]
            .lessonTeacherPairs.clear();
        int counter = 0;
        for (auto& lesson: tmpLessons)
        {
            if (!combinedLessonLessons[lesson.first]) continue;
            for (auto& teacher: currentTimetable.teachers)
            {
                if (!combinedLessonTeachers[std::to_string(lesson.first) + teacher.second.name])
                    continue;
                tmpTmpTimetable.classes[currentClassId]
                    .timetableLessons[currentLessonId]
                    .lessonTeacherPairs.push_back(LessonTeacherPair());
                tmpTmpTimetable.classes[currentClassId]
                    .timetableLessons[currentLessonId]
                    .lessonTeacherPairs[counter]
                    .lessonId = lesson.first;
                tmpTmpTimetable.classes[currentClassId]
                    .timetableLessons[currentLessonId]
                    .lessonTeacherPairs[counter]
                    .teacherId = teacher.first;
                counter++;
            }
        }
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel")))
    {
        if (newCombinedLesson)
            tmpTmpTimetable.classes[currentClassId].timetableLessons.erase(currentLessonId);
        *isOpen = false;
    }
    ImGui::End();
}

std::string timetableLessonValues = "";
size_t currentRuleId = -1;
bool newRule = false;
int minTimetableLessonAmount = INT_MAX;
TimetableLessonRule tmpRule;

void ResetRulesVariables()
{
    LoadTimetableLessonsFromSelection();

    timetableLessonValues = "";
    for (auto& lesson: tmpTmpTimetable.classes[currentClassId].timetableLessons)
    {
        for (size_t i = 0; i < lesson.second.lessonTeacherPairs.size(); i++)
        {
            LessonTeacherPair& lessonTeacherPair = lesson.second.lessonTeacherPairs[i];
            timetableLessonValues += tmpLessons[lessonTeacherPair.lessonId].name + " (" +
                                     currentTimetable.teachers[lessonTeacherPair.teacherId].name +
                                     ")";
            if (i < lesson.second.lessonTeacherPairs.size() - 1) timetableLessonValues += ", ";
        }
        timetableLessonValues += '\0';
    }

    minTimetableLessonAmount = INT_MAX;
    tmpRule = tmpTmpTimetable.classes[currentClassId].timetableLessonRules[currentRuleId];
}

bool isRules = false;
void ShowRules(bool* isOpen)
{
    if (!ImGui::Begin((newRule ? gettext("New rule") : gettext("Edit rule")), isOpen))
    {
        ImGui::End();
        return;
    }

    TimetableLessonRule& timetableLessonRule =
        tmpTmpTimetable.classes[currentClassId].timetableLessonRules[currentRuleId];

    ImGui::Checkbox(gettext("preserve order"), &timetableLessonRule.preserveOrder);
    ImGui::InputInt(gettext("amount"), &timetableLessonRule.amount);
    if (timetableLessonRule.amount < 1) timetableLessonRule.amount = 1;
    if (timetableLessonRule.amount > minTimetableLessonAmount)
        timetableLessonRule.amount = minTimetableLessonAmount;

    if (ImGui::Button(gettext("+")))
    {
        timetableLessonRule.timetableLessonIds.push_back(-1);
    }

    for (size_t i = 0; i < timetableLessonRule.timetableLessonIds.size(); i++)
    {
        ImGui::PushID(i);
        if (ImGui::Button(gettext("-")))
        {
            timetableLessonRule.timetableLessonIds.erase(
                timetableLessonRule.timetableLessonIds.begin() + i);
            i--;
            ImGui::PopID();
            continue;
        }
        ImGui::SameLine();
        if (ImGui::Combo("", &timetableLessonRule.timetableLessonIds[i],
                         timetableLessonValues.c_str()))
        {
            minTimetableLessonAmount = INT_MAX;
            for (size_t i = 0; i < timetableLessonRule.timetableLessonIds.size(); i++)
            {
                int timetableLessonId = timetableLessonRule.timetableLessonIds[i];
                if (timetableLessonId < 0) continue;
                if (tmpTmpTimetable.classes[currentClassId]
                        .timetableLessons[timetableLessonId]
                        .amount < minTimetableLessonAmount)
                {
                    minTimetableLessonAmount = tmpTmpTimetable.classes[currentClassId]
                                                   .timetableLessons[timetableLessonId]
                                                   .amount;
                }
            }
        }
        ImGui::PopID();
    }

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Pressed the Ok button in rules of class with Id " +
                std::to_string(currentClassId));
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel")))
    {
        if (newRule)
        {
            tmpTmpTimetable.classes[currentClassId].timetableLessonRules.erase(
                tmpTmpTimetable.classes[currentClassId].timetableLessonRules.begin() +
                currentRuleId);
        }
        else
        {
            tmpTmpTimetable.classes[currentClassId].timetableLessonRules[currentRuleId] = tmpRule;
        }
        *isOpen = false;
    }
    ImGui::End();
}
