// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <unordered_map>

int currentClassId = 0;
bool newClass = false;
bool bulkEditClass = false;
int bulkClassesAmount = 1;
int classTeacherIndex = 0;
std::string classTeacherValues = "";
std::vector<int> classTeacherIds;
bool allClassLessons = true;
std::unordered_map<std::string, bool> classLessons;
std::unordered_map<std::string, int> classLessonAmounts;
std::unordered_map<int, bool> allClassLessonTeachers;
std::unordered_map<std::string, bool> classLessonTeachers;
std::vector<bool> allAvailableClassLessonsVertical;
std::vector<bool> allAvailableClassLessonsHorizontal;
std::map<int, Lesson> tmpLessons;
std::map<int, Lesson> tmpTmpLessons;

static void ResetVariables()
{
    LogInfo("Resetting class variables");
    allAvailableClassLessonsVertical.clear();
    allAvailableClassLessonsVertical.resize(daysPerWeek, true);
    allAvailableClassLessonsHorizontal.clear();
    allAvailableClassLessonsHorizontal.resize(lessonsPerDay, true);

    tmpTmpTimetable.classes[currentClassId].days.resize(daysPerWeek);
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        if (tmpTmpTimetable.classes[currentClassId].days[i].lessons.size() < lessonsPerDay)
        {
            int iterations =
                lessonsPerDay - tmpTmpTimetable.classes[currentClassId].days[i].lessons.size();
            for (int j = 0; j < iterations; j++)
                tmpTmpTimetable.classes[currentClassId].days[i].lessons.push_back(newClass);
        }
    }

    allClassLessons = true;
    allClassLessonTeachers.clear();
    for (auto& lesson: tmpLessons) allClassLessonTeachers[lesson.first] = true;
    classLessons.clear();
    classLessonTeachers.clear();

    for (auto& lesson: tmpLessons)
    {
        classLessons[std::to_string(lesson.first) + "0"] = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = false;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] =
                newClass;
            classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] = 1;
        }
    }

    for (auto& lesson: tmpLessons)
    {
        bool classIdFound = false;
        for (size_t i = 0; i < lesson.second.classIds.size(); i++)
        {
            if (currentClassId == lesson.second.classIds[i] ||
                (tmpTmpTimetable.classes[currentClassId].number ==
                     tmpTmpTimetable.classes[lesson.second.classIds[i]].number &&
                 bulkEditClass))
            {
                classIdFound = true;
                break;
            }
        }
        if (!classIdFound) continue;
        classLessons[std::to_string(lesson.first) + "0"] = true;
        for (auto& teacher: currentTimetable.teachers)
        {
            bool lessonIdFound = false;
            for (size_t i = 0; i < teacher.second.lessonIds.size(); i++)
            {
                if (teacher.second.lessonIds[i] == lesson.first)
                {
                    lessonIdFound = true;
                    break;
                }
            }
            if (!lessonIdFound) continue;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = true;
        }
    }

    for (auto& lesson: tmpLessons)
    {
        for (auto& teacher: currentTimetable.teachers)
        {
            bool lessonTeacherPairFound = false;
            int lessonTeacherPairId = -1;
            for (auto& timetableLesson: tmpTmpTimetable.classes[currentClassId].timetableLessons)
            {
                if (timetableLesson.second.lessonTeacherPairs.size() != 1) continue;
                if (lesson.first == timetableLesson.second.lessonTeacherPairs[0].lessonId &&
                    teacher.first == timetableLesson.second.lessonTeacherPairs[0].teacherId)
                {
                    lessonTeacherPairFound = true;
                    lessonTeacherPairId = timetableLesson.first;
                    break;
                }
            }
            if (!lessonTeacherPairFound) continue;
            classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] =
                tmpTmpTimetable.classes[currentClassId]
                    .timetableLessons[lessonTeacherPairId]
                    .amount;
            classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = true;
        }
    }

    tmpTmpLessons = tmpLessons;
    ResetClassTeacherValues();
}

bool isEditClass = false;
void ShowEditClass(bool* isOpen)
{
    if (!ImGui::Begin((newClass ? gettext("New class") : gettext("Edit class")), isOpen))
    {
        ImGui::End();
        return;
    }

    // Bulk editing warning
    if (bulkEditClass && !newClass)
    {
        ImGui::TextColored(
            ImVec4(255, 255, 0, 255), "%s",
            gettext(
                "Warning: you are bulk editing classes!\nAfter pressing Ok ALL classes with the number below\nwill be OVERWRITTEN with the data you enter.\nIf you don't want that to happen, press the Cancel button."));
    }

    // Class number
    if (ImGui::InputText(gettext("number"), &tmpTmpTimetable.classes[currentClassId].number))
    {
        tmpTmpTimetable.classes[currentClassId].timetableLessons.clear();
        tmpTmpTimetable.classes[currentClassId].maxTimetableLessonId = 0;
        ResetVariables();
        FetchClassLessonsFromSimularClasses(tmpTmpTimetable, currentClassId);
    }

    // Classes amount
    if (bulkEditClass)
    {
        ImGui::InputInt(gettext("amount"), &bulkClassesAmount);
        if (bulkClassesAmount < 1) bulkClassesAmount = 1;
        if ((size_t)bulkClassesAmount >= strlen(gettext("abcdefghijklmnopqrstuvwxyz")))
            bulkClassesAmount = strlen(gettext("abcdefghijklmnopqrstuvwxyz")) - 1;
    }
    // Class letter and teacher
    else
    {
        ImGui::InputText(gettext("letter"), &tmpTmpTimetable.classes[currentClassId].letter);
        ImGui::Combo(gettext("teacher"), &classTeacherIndex, classTeacherValues.c_str());
    }
    ImGui::Separator();

    // Class available lessons
    ImGui::Text("%s", gettext("available lessons"));
    ImGui::Separator();
    ImGui::Columns(daysPerWeek + 1);
    ImGui::LabelText("##1", "%s", "");
    ImGui::LabelText("##2", "%s", "");
    int pushId = 3;
    allAvailableClassLessonsHorizontal.resize(lessonsPerDay, true);
    tmpTmpTimetable.classes[currentClassId].days.resize(daysPerWeek);
    for (size_t i = 0; i < lessonsPerDay; i++)
    {
        ImGui::PushID(pushId++);
        bool availableClassLessonsHorizontal = allAvailableClassLessonsHorizontal[i];
        if (ImGui::Checkbox(std::to_string(i).c_str(), &availableClassLessonsHorizontal))
        {
            LogInfo("Clicked allAvailableClassLessonsHorizontal number  " + std::to_string(i) +
                    " in class with Id " + std::to_string(currentClassId));
            allAvailableClassLessonsHorizontal[i] = availableClassLessonsHorizontal;
            for (size_t j = 0; j < daysPerWeek; j++)
            {
                tmpTmpTimetable.classes[currentClassId].days[j].lessons.resize(lessonsPerDay);
                tmpTmpTimetable.classes[currentClassId].days[j].lessons[i] =
                    allAvailableClassLessonsHorizontal[i];
            }
        }
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();
    allAvailableClassLessonsVertical.resize(daysPerWeek, false);
    tmpTmpTimetable.classes[currentClassId].days.resize(daysPerWeek);
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        tmpTmpTimetable.classes[currentClassId].days[i].lessons.resize(lessonsPerDay);
        int weekDay = i;
        while (weekDay >= 7) weekDay -= 7;
        ImGui::Text("%s", weekDays[weekDay].c_str());
        ImGui::PushID(pushId++);
        bool availableClassLessonsVertical = allAvailableClassLessonsVertical[i];
        if (ImGui::Checkbox((allAvailableClassLessonsVertical[i] ? gettext("Deselect all")
                                                                 : gettext("Select all")),
                            &availableClassLessonsVertical))
        {
            LogInfo("Clicked allAvailableClassLessonsVertical number  " + std::to_string(i) +
                    " in class with Id " + std::to_string(currentClassId));
            allAvailableClassLessonsVertical[i] = availableClassLessonsVertical;
            for (size_t j = 0; j < lessonsPerDay; j++)
                tmpTmpTimetable.classes[currentClassId].days[i].lessons[j] =
                    allAvailableClassLessonsVertical[i];
        }
        ImGui::PopID();
        for (size_t j = 0; j < lessonsPerDay; j++)
        {
            ImGui::PushID(pushId++);
            bool isLessonAvailable = tmpTmpTimetable.classes[currentClassId].days[i].lessons[j];
            if (ImGui::Checkbox("", &isLessonAvailable))
            {
                tmpTmpTimetable.classes[currentClassId].days[i].lessons[j] = isLessonAvailable;
                LogInfo("Clicked isLessonAvailable in day " + std::to_string(i) +
                        " in lesson number " + std::to_string(j) + " in class with Id " +
                        std::to_string(currentClassId));
            }
            ImGui::PopID();
        }
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Combine lessons
    ImGui::LabelText("", "%s", gettext("lessons"));
    ImGui::Separator();
    if (ImGui::Button(gettext("Combine lessons")))
    {
        LogInfo("Clicked the combine lessons button in class with Id " +
                std::to_string(currentClassId));
        newCombinedLesson = true;
        ResetCombineLessonsVariables();
        tmpTmpTimetable.classes[currentClassId].maxTimetableLessonId++;
        currentLessonId = tmpTmpTimetable.classes[currentClassId].maxTimetableLessonId;
        tmpTmpTimetable.classes[currentClassId].timetableLessons[currentLessonId] =
            TimetableLesson();
        isCombineLessons = true;
    }
    for (auto it = tmpTmpTimetable.classes[currentClassId].timetableLessons.begin();
         it != tmpTmpTimetable.classes[currentClassId].timetableLessons.end();)
    {
        if (it->second.lessonTeacherPairs.size() <= 1)
        {
            ++it;
            continue;
        }
        ImGui::PushID(pushId++);
        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a timetable lesson with Id " + std::to_string(it->first) +
                    " in a class with Id " + std::to_string(it->first));
            ImGui::PopID();
            it = tmpTmpTimetable.classes[currentClassId].timetableLessons.erase(it);
            break;
        }
        ImGui::SameLine();
        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a timetable lesson with Id " + std::to_string(it->first) +
                    " in a class with Id " + std::to_string(currentClassId));
            newCombinedLesson = false;
            currentLessonId = it->first;
            ResetCombineLessonsVariables();
            isCombineLessons = true;
        }
        ImGui::SameLine();
        std::string text = "";
        for (size_t j = 0; j < it->second.lessonTeacherPairs.size(); j++)
        {
            text += tmpLessons[it->second.lessonTeacherPairs[j].lessonId].name + " (";
            text +=
                currentTimetable.teachers[it->second.lessonTeacherPairs[j].teacherId].name + ")";
            if (j < it->second.lessonTeacherPairs.size() - 1) text += "\n";
        }
        ImGui::InputInt(text.c_str(), &it->second.amount);
        ImGui::PopID();
        ++it;
    }
    ImGui::Separator();
    ImGui::Columns(2);

    // Lessons
    for (auto& lesson: tmpLessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        bool anyTeacherSelected = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            if (classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"])
            {
                anyTeacherSelected = true;
                break;
            }
        }
        if (!anyTeacherSelected)
        {
            ImGui::PushID(pushId++);
            ImGui::TextColored(ImVec4(255, 255, 0, 255), "%s",
                               gettext("Warning: no teacher selected for this lesson"));
            ImGui::PopID();
        }
        ImGui::PushID(pushId++);
        ImGui::NextColumn();
        if (ImGui::Checkbox(
                (std::string(allClassLessonTeachers[lesson.first] ? gettext("Deselect all")
                                                                  : gettext("Select all")) +
                 "##1")
                    .c_str(),
                &allClassLessonTeachers[lesson.first]))
        {
            LogInfo("Clicked allClassLessonTeachers in class with Id " +
                    std::to_string(currentClassId));
            for (auto& teacher: currentTimetable.teachers)
                classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] =
                    allClassLessonTeachers[lesson.first];
        }
        ImGui::NextColumn();
        ImGui::PopID();
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            ImGui::PushID(pushId++);
            ImGui::BeginDisabled(
                !classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::InputInt(
                lesson.second.name.c_str(),
                &classLessonAmounts[std::to_string(lesson.first) + teacher.second.name]);
            if (classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] < 0)
                classLessonAmounts[std::to_string(lesson.first) + teacher.second.name] = 0;
            ImGui::EndDisabled();
            ImGui::NextColumn();
            ImGui::Checkbox(
                teacher.second.name.c_str(),
                &classLessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Separator();
    }
    ImGui::Columns(1);

    // Additional rules
    if (ImGui::Button(gettext("Add a lesson rule")))
    {
        newRule = true;
        tmpTmpTimetable.classes[currentClassId].timetableLessonRules.push_back(
            TimetableLessonRule());
        currentRuleId = tmpTmpTimetable.classes[currentClassId].timetableLessonRules.size() - 1;
        ResetRulesVariables();
        isRules = true;
    }
    ImGui::Columns(3);
    ImGui::Text("%s", gettext("Rules"));
    ImGui::NextColumn();
    ImGui::Text("%s", gettext("Preserve order"));
    ImGui::NextColumn();
    ImGui::Text("%s", gettext("Amount"));
    ImGui::NextColumn();
    ImGui::Separator();
    for (size_t i = 0; i < tmpTmpTimetable.classes[currentClassId].timetableLessonRules.size(); i++)
    {
        if (isRules && newRule && i == currentRuleId) continue;
        ImGui::PushID(pushId++);

        if (ImGui::Button(gettext("-")))
        {
            tmpTmpTimetable.classes[currentClassId].timetableLessonRules.erase(
                tmpTmpTimetable.classes[currentClassId].timetableLessonRules.begin() + i);
            i--;
            ImGui::PopID();
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            newRule = false;
            currentRuleId = i;
            ResetRulesVariables();
            isRules = true;
        }
        ImGui::SameLine();

        TimetableLessonRule& timetableLessonRule =
            tmpTmpTimetable.classes[currentClassId].timetableLessonRules[i];
        std::string timetableLessonRuleName = "";
        for (size_t j = 0; j < timetableLessonRule.timetableLessonIds.size(); j++)
        {
            int timetableLessonId = timetableLessonRule.timetableLessonIds[j];
            TimetableLesson& timetableLesson =
                tmpTmpTimetable.classes[currentClassId].timetableLessons[timetableLessonId];
            for (size_t k = 0; k < timetableLesson.lessonTeacherPairs.size(); k++)
            {
                LessonTeacherPair& lessonTeacherPair = timetableLesson.lessonTeacherPairs[k];
                timetableLessonRuleName +=
                    tmpLessons[lessonTeacherPair.lessonId].name + " (" +
                    currentTimetable.teachers[lessonTeacherPair.teacherId].name + ")";
                if (k < timetableLesson.lessonTeacherPairs.size() - 1)
                    timetableLessonRuleName += '\n';
            }
            if (j < timetableLessonRule.timetableLessonIds.size() - 1)
                timetableLessonRuleName += "\n\n";
        }
        ImGui::Text("%s", timetableLessonRuleName.c_str());
        ImGui::NextColumn();

        ImGui::Text("%s", (timetableLessonRule.preserveOrder ? gettext("Yes") : gettext("No")));
        ImGui::NextColumn();

        ImGui::Text("%d", timetableLessonRule.amount);
        ImGui::NextColumn();
        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked the Ok button while editing class with Id " +
                std::to_string(currentClassId));
        LoadTimetableLessonsFromSelection();
        if (bulkEditClass)
        {
            ChangeClassesAmount(tmpTmpTimetable, tmpTmpTimetable.classes[currentClassId].number,
                                bulkClassesAmount);
            for (auto& classPair: tmpTmpTimetable.classes)
            {
                if (classPair.first == currentClassId) continue;
                if (classPair.second.number == tmpTmpTimetable.classes[currentClassId].number)
                {
                    int teacherId = classPair.second.teacherId;
                    classPair.second = tmpTmpTimetable.classes[currentClassId];
                    classPair.second.teacherId = teacherId;
                }
            }
            UpdateClassLetters(tmpTmpTimetable);
        }
        else
        {
            if (classTeacherIndex >= 0 && (size_t)classTeacherIndex < classTeacherIds.size())
            {
                tmpTmpTimetable.classes[currentClassId].teacherId =
                    classTeacherIds[classTeacherIndex];
            }
            else
            {
                tmpTmpTimetable.classes[currentClassId].teacherId = -1;
            }
        }
        tmpTimetable.classes = tmpTmpTimetable.classes;
        tmpTimetable.maxClassId = tmpTmpTimetable.maxClassId;
        tmpTimetable.orderedClasses = tmpTmpTimetable.orderedClasses;
        tmpLessons = tmpTmpLessons;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}

bool isClasses = false;
void ShowClasses(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Classes"), isOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::TextColored(
        ImVec4(255, 255, 0, 255), "%s",
        gettext(
            "Warning: changing the current year can be quite destructive.\nIf something went wrong, press the Cancel button to revert all changes"));
    if (ImGui::Button(gettext("Back"))) ShiftClasses(tmpTimetable, -1);
    ImGui::SameLine();
    ImGui::Text("%s", std::to_string(tmpTimetable.year).c_str());
    ImGui::SameLine();
    if (ImGui::Button(gettext("Next"))) ShiftClasses(tmpTimetable, 1);
    ImGui::Separator();

    if (ImGui::Button(gettext("+")))
    {
        newClass = true;
        bulkEditClass = true;
        tmpTmpTimetable.classes = tmpTimetable.classes;
        tmpTmpTimetable.maxClassId = tmpTimetable.maxClassId;
        tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
        tmpTmpTimetable.maxClassId++;
        currentClassId = tmpTmpTimetable.maxClassId;
        LogInfo("Adding a new class with Id " + std::to_string(currentClassId));
        tmpTmpTimetable.orderedClasses.push_back(currentClassId);
        tmpTmpTimetable.classes[currentClassId] = Class();
        tmpTmpTimetable.classes[currentClassId].number = "0";
        ResetVariables();
        FetchClassLessonsFromSimularClasses(tmpTmpTimetable, currentClassId);
        isEditClass = true;
    }
    ImGui::Separator();

    ImGui::Columns(2);
    std::string lastClassNumber = "";
    int buttonId = 0;
    for (size_t i = 0; i < tmpTimetable.orderedClasses.size(); i++)
    {
        if (lastClassNumber != tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number)
        {
            lastClassNumber = tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number;
            ImGui::PushID(buttonId);

            if (ImGui::Button(gettext("-")))
            {
                LogInfo("Removed classes with number " + lastClassNumber);
                ImGui::PopID();
                for (auto it = tmpTimetable.classes.begin(); it != tmpTimetable.classes.end();)
                {
                    if (it->second.number == lastClassNumber)
                    {
                        tmpTimetable.orderedClasses.erase(
                            std::find(tmpTimetable.orderedClasses.begin(),
                                      tmpTimetable.orderedClasses.end(), it->first));
                        it = tmpTimetable.classes.erase(it);
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
                bulkClassesAmount = 0;
                for (auto& classPair: tmpTimetable.classes)
                {
                    if (classPair.second.number == lastClassNumber) bulkClassesAmount++;
                }
                newClass = false;
                bulkEditClass = true;
                tmpTmpTimetable.classes = tmpTimetable.classes;
                tmpTmpTimetable.maxClassId = tmpTimetable.maxClassId;
                tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
                currentClassId = tmpTmpTimetable.orderedClasses[i];
                ResetVariables();
                isEditClass = true;
            }
            ImGui::SameLine();

            ImGui::Text("%s", lastClassNumber.c_str());
            ImGui::Indent();
            if (ImGui::Button(gettext("+")))
            {
                newClass = true;
                bulkEditClass = false;
                for (size_t j = 0; j < tmpTimetable.orderedClasses.size(); j++)
                {
                    if (tmpTimetable.classes[tmpTimetable.orderedClasses[j]].number ==
                        lastClassNumber)
                        currentClassId = j;
                }
                currentClassId++;
                tmpTmpTimetable.classes = tmpTimetable.classes;
                tmpTmpTimetable.maxClassId = tmpTimetable.maxClassId;
                tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
                tmpTmpTimetable.maxClassId++;
                tmpTmpTimetable.orderedClasses.insert(tmpTmpTimetable.orderedClasses.begin() +
                                                          currentClassId,
                                                      tmpTmpTimetable.maxClassId);
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassId] = Class();
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassId].number = lastClassNumber;
                tmpTmpTimetable.classes[tmpTmpTimetable.maxClassId].letter =
                    GetNthUtf8Character(gettext("abcdefghijklmnopqrstuvwxyz"), currentClassId);
                currentClassId = tmpTmpTimetable.maxClassId;
                ResetVariables();
                FetchClassLessonsFromSimularClasses(tmpTmpTimetable, tmpTmpTimetable.maxClassId);
                LogInfo("Adding a new class with number " + lastClassNumber + " and Id " +
                        std::to_string(currentClassId));
                isEditClass = true;
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
            LogInfo("Removed a class with Id " + std::to_string(tmpTimetable.orderedClasses[i]));
            tmpTimetable.classes.erase(tmpTimetable.orderedClasses[i]);
            tmpTimetable.orderedClasses.erase(tmpTimetable.orderedClasses.begin() + i);
            i--;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            newClass = false;
            bulkEditClass = false;
            tmpTmpTimetable.classes = tmpTimetable.classes;
            tmpTmpTimetable.maxClassId = tmpTimetable.maxClassId;
            tmpTmpTimetable.orderedClasses = tmpTimetable.orderedClasses;
            currentClassId = tmpTmpTimetable.orderedClasses[i];
            LogInfo("Editing class with Id " + std::to_string(currentClassId));
            ResetVariables();
            isEditClass = true;
        }
        ImGui::SameLine();

        ImGui::Text("%s", (tmpTimetable.classes[tmpTimetable.orderedClasses[i]].number +
                           tmpTimetable.classes[tmpTimetable.orderedClasses[i]].letter)
                              .c_str());
        ImGui::PopID();
        buttonId++;
        ImGui::Unindent();
        ImGui::NextColumn();

        if (currentTimetable.teachers.find(
                tmpTimetable.classes[tmpTimetable.orderedClasses[i]].teacherId) !=
            currentTimetable.teachers.end())
            ImGui::LabelText(
                "", "%s",
                currentTimetable
                    .teachers[tmpTimetable.classes[tmpTimetable.orderedClasses[i]].teacherId]
                    .name.c_str());
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok in the classes menu");
        currentTimetable.classes = tmpTimetable.classes;
        currentTimetable.maxClassId = tmpTimetable.maxClassId;
        currentTimetable.orderedClasses = tmpTimetable.orderedClasses;
        currentTimetable.lessons = tmpLessons;
        currentTimetable.year = tmpTimetable.year;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}
