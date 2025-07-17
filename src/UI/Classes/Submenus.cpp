#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "UI.hpp"
#include "UI/Classes.hpp"
#include <climits>
#include <imgui.h>
#include <vector>

int currentLessonID = 0;
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
            tmpTmpTimetable.classes[currentClassID].timetableLessons[currentLessonID];
        for (size_t j = 0; j < currentLesson.lessonTeacherPairs.size(); j++)
        {
            if (!classLessons[std::to_string(currentLesson.lessonTeacherPairs[j].lessonID) + "0"])
                continue;
            if (!classLessonTeachers
                    [std::to_string(currentLesson.lessonTeacherPairs[j].lessonID) +
                     currentTimetable.teachers[currentLesson.lessonTeacherPairs[j].teacherID].name +
                     "0"])
                continue;
            combinedLessonLessons[currentLesson.lessonTeacherPairs[j].lessonID] = true;
            combinedLessonTeachers
                [std::to_string(currentLesson.lessonTeacherPairs[j].lessonID) +
                 currentTimetable.teachers[currentLesson.lessonTeacherPairs[j].teacherID].name] =
                    true;
        }
    }
}

void ShowCombineLessons(bool* isOpen)
{
    if (!ImGui::Begin(labels["Combine lessons"].c_str(), isOpen))
    {
        ImGui::End();
        return;
    }

    // Lessons
    ImGui::Columns(2);
    int pushID = 0;
    for (auto& lesson: tmpLessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        ImGui::PushID(pushID);
        ImGui::Checkbox(lesson.second.name.c_str(), &combinedLessonLessons[lesson.first]);
        ImGui::NextColumn();
        ImGui::PopID();
        pushID++;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            ImGui::PushID(pushID);
            ImGui::Checkbox(
                teacher.second.name.c_str(),
                &combinedLessonTeachers[std::to_string(lesson.first) + teacher.second.name]);
            ImGui::PopID();
            pushID++;
        }
        ImGui::NextColumn();
        ImGui::Separator();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Pressed the Ok button in combine lessons of class with ID " +
                std::to_string(currentClassID));
        tmpTmpTimetable.classes[currentClassID]
            .timetableLessons[currentLessonID]
            .lessonTeacherPairs.clear();
        int counter = 0;
        for (auto& lesson: tmpLessons)
        {
            if (!combinedLessonLessons[lesson.first]) continue;
            for (auto& teacher: currentTimetable.teachers)
            {
                if (!combinedLessonTeachers[std::to_string(lesson.first) + teacher.second.name])
                    continue;
                tmpTmpTimetable.classes[currentClassID]
                    .timetableLessons[currentLessonID]
                    .lessonTeacherPairs.push_back(LessonTeacherPair());
                tmpTmpTimetable.classes[currentClassID]
                    .timetableLessons[currentLessonID]
                    .lessonTeacherPairs[counter]
                    .lessonID = lesson.first;
                tmpTmpTimetable.classes[currentClassID]
                    .timetableLessons[currentLessonID]
                    .lessonTeacherPairs[counter]
                    .teacherID = teacher.first;
                counter++;
            }
        }
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str()))
    {
        if (newCombinedLesson)
            tmpTmpTimetable.classes[currentClassID].timetableLessons.erase(currentLessonID);
        *isOpen = false;
    }
    ImGui::End();
}

std::string timetableLessonValues = "";
size_t currentRuleID = -1;
bool newRule = false;
int minTimetableLessonAmount = INT_MAX;
TimetableLessonRule tmpRule;

void ResetRulesVariables()
{
    LoadTimetableLessonsFromSelection();

    timetableLessonValues = "";
    for (auto& lesson: tmpTmpTimetable.classes[currentClassID].timetableLessons)
    {
        for (size_t i = 0; i < lesson.second.lessonTeacherPairs.size(); i++)
        {
            LessonTeacherPair& lessonTeacherPair = lesson.second.lessonTeacherPairs[i];
            timetableLessonValues += tmpLessons[lessonTeacherPair.lessonID].name + " (" +
                                     currentTimetable.teachers[lessonTeacherPair.teacherID].name +
                                     ")";
            if (i < lesson.second.lessonTeacherPairs.size() - 1) timetableLessonValues += ", ";
        }
        timetableLessonValues += '\0';
    }

    minTimetableLessonAmount = INT_MAX;
    tmpRule = tmpTmpTimetable.classes[currentClassID].timetableLessonRules[currentRuleID];
}

bool isRules = false;
void ShowRules(bool* isOpen)
{
    if (!ImGui::Begin((newRule ? labels["New rule"] : labels["Edit rule"]).c_str(), isOpen))
    {
        ImGui::End();
        return;
    }

    TimetableLessonRule& timetableLessonRule =
        tmpTmpTimetable.classes[currentClassID].timetableLessonRules[currentRuleID];

    ImGui::Checkbox(labels["preserve order"].c_str(), &timetableLessonRule.preserveOrder);
    ImGui::InputInt(labels["amount"].c_str(), &timetableLessonRule.amount);
    if (timetableLessonRule.amount < 1) timetableLessonRule.amount = 1;
    if (timetableLessonRule.amount > minTimetableLessonAmount)
        timetableLessonRule.amount = minTimetableLessonAmount;

    if (ImGui::Button(labels["+"].c_str()))
    {
        timetableLessonRule.timetableLessonIDs.push_back(-1);
    }

    for (size_t i = 0; i < timetableLessonRule.timetableLessonIDs.size(); i++)
    {
        ImGui::PushID(i);
        if (ImGui::Button(labels["-"].c_str()))
        {
            timetableLessonRule.timetableLessonIDs.erase(
                timetableLessonRule.timetableLessonIDs.begin() + i);
            i--;
            ImGui::PopID();
            continue;
        }
        ImGui::SameLine();
        if (ImGui::Combo("", &timetableLessonRule.timetableLessonIDs[i],
                         timetableLessonValues.c_str()))
        {
            minTimetableLessonAmount = INT_MAX;
            for (size_t i = 0; i < timetableLessonRule.timetableLessonIDs.size(); i++)
            {
                int timetableLessonID = timetableLessonRule.timetableLessonIDs[i];
                if (timetableLessonID < 0) continue;
                if (tmpTmpTimetable.classes[currentClassID]
                        .timetableLessons[timetableLessonID]
                        .amount < minTimetableLessonAmount)
                {
                    minTimetableLessonAmount = tmpTmpTimetable.classes[currentClassID]
                                                   .timetableLessons[timetableLessonID]
                                                   .amount;
                }
            }
        }
        ImGui::PopID();
    }

    // Ok and Cancel
    if (ImGui::Button(labels["Ok"].c_str()))
    {
        LogInfo("Pressed the Ok button in rules of class with ID " +
                std::to_string(currentClassID));
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(labels["Cancel"].c_str()))
    {
        if (newRule)
        {
            tmpTmpTimetable.classes[currentClassID].timetableLessonRules.erase(
                tmpTmpTimetable.classes[currentClassID].timetableLessonRules.begin() +
                currentRuleID);
        }
        else
        {
            tmpTmpTimetable.classes[currentClassID].timetableLessonRules[currentRuleID] = tmpRule;
        }
        *isOpen = false;
    }
    ImGui::End();
}
