// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes/Edit.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "UI/Classes/Edit/CombineLessons.hpp"
#include "UI/Classes/Edit/Rules.hpp"
#include "UI/Classes/Utils.hpp"
#include "Utils.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

std::shared_ptr<EditClassMenu> editClassMenu;

void EditClassMenu::Open(Timetable* prevTimetable, bool newClass, int classId, bool bulkEdit,
                         int bulkAmount)
{
    this->prevTimetable = prevTimetable;
    this->newClass = newClass;
    this->classId = classId;
    this->bulkEdit = bulkEdit;
    this->bulkAmount = bulkAmount;

    timetable = *prevTimetable;

    if (newClass)
    {
        timetable.orderedClasses.insert(timetable.orderedClasses.begin() + classId,
                                        timetable.maxClassId + 1);
        timetable.classes[classId] = Class();
        timetable.classes[classId].number = "0";
        timetable.classes[timetable.maxClassId].letter =
            GetNthUtf8Character(GetText("abcdefghijklmnopqrstuvwxyz"), classId);

        classId = timetable.maxClassId + 1;
    }

    LogInfo("Resetting class variables");

    allAvailableLessonsVertical.clear();
    allAvailableLessonsVertical.resize(settings.daysPerWeek, true);
    allAvailableLessonsHorizontal.clear();
    allAvailableLessonsHorizontal.resize(settings.lessonsPerDay, true);

    timetable.classes[classId].days.resize(settings.daysPerWeek);
    for (size_t i = 0; i < settings.daysPerWeek; i++)
    {
        if (timetable.classes[classId].days[i].lessons.size() < settings.lessonsPerDay)
        {
            int iterations =
                settings.lessonsPerDay - timetable.classes[classId].days[i].lessons.size();
            for (int j = 0; j < iterations; j++)
                timetable.classes[classId].days[i].lessons.push_back(newClass);
        }
    }

    allLessons = true;
    allLessonTeachers.clear();
    for (auto& lesson: timetable.lessons) allLessonTeachers[lesson.first] = true;
    lessons.clear();

    lessonTeachers.clear();
    for (auto& lesson: timetable.lessons)
    {
        lessons[std::to_string(lesson.first) + "0"] = false;
        for (auto& teacher: prevTimetable->teachers)
        {
            lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = false;
            lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = newClass;
            lessonAmounts[std::to_string(lesson.first) + teacher.second.name] = 1;
        }
    }

    for (auto& lesson: timetable.lessons)
    {
        bool classIdFound = false;
        for (size_t i = 0; i < lesson.second.classIds.size(); i++)
        {
            if (classId == lesson.second.classIds[i] ||
                (timetable.classes[classId].number ==
                     timetable.classes[lesson.second.classIds[i]].number &&
                 bulkEdit))
            {
                classIdFound = true;
                break;
            }
        }
        if (!classIdFound) continue;
        lessons[std::to_string(lesson.first) + "0"] = true;
        for (auto& teacher: prevTimetable->teachers)
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
            lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"] = true;
        }
    }

    for (auto& lesson: timetable.lessons)
    {
        for (auto& teacher: prevTimetable->teachers)
        {
            bool lessonTeacherPairFound = false;
            int lessonTeacherPairId = -1;
            for (auto& timetableLesson: timetable.classes[classId].timetableLessons)
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
            lessonAmounts[std::to_string(lesson.first) + teacher.second.name] =
                timetable.classes[classId].timetableLessons[lessonTeacherPairId].amount;
            lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] = true;
        }
    }

    // Reset teacher values
    teacherValues = GetText("none");
    teacherValues += '\0';
    teacherIds.clear();
    teacherIds.push_back(-1);
    for (auto& teacher: currentTimetable.teachers)
    {
        bool isTeacherTaken = false;
        for (auto& classPair: timetable.classes)
        {
            if (classPair.second.teacherId == teacher.first && classPair.first != classId)
            {
                isTeacherTaken = true;
                break;
            }
        }

        if (!isTeacherTaken)
        {
            if (teacher.second.name == "")
            {
                teacherValues += GetText("error");
            }
            else
            {
                teacherValues += teacher.second.name;
            }
            teacherIds.push_back(teacher.first);
            teacherValues += '\0';
        }
    }
    teacherValues += '\0';

    teacherIndex = 0;
    for (size_t i = 0; i < teacherIds.size(); i++)
    {
        if (timetable.classes[classId].teacherId == teacherIds[i])
        {
            teacherIndex = i;
            break;
        }
    }

    if (newClass)
    {
        FetchClassLessonsFromSimularClasses(timetable, classId);
    }

    Window::Open();
}

void EditClassMenu::Draw()
{
    if (!ImGui::Begin((newClass ? gettext("New class") : gettext("Edit class")), &visible))
    {
        ImGui::End();
        return;
    }

    // Bulk editing warning
    if (bulkEdit && !newClass)
    {
        ImGui::TextColored(
            ImVec4(255, 255, 0, 255), "%s",
            gettext(
                "Warning: you are bulk editing classes!\nAfter pressing Ok ALL classes with the number below\nwill be OVERWRITTEN with the data you enter.\nIf you don't want that to happen, press the Cancel button."));
    }

    // Class number
    if (ImGui::InputText(gettext("number"), &timetable.classes[classId].number))
    {
        Open(prevTimetable, true, classId, bulkEdit, bulkAmount);
        timetable.classes[classId].timetableLessons.clear();
        timetable.classes[classId].maxTimetableLessonId = 0;
    }

    // Classes amount
    if (bulkEdit)
    {
        ImGui::InputInt(gettext("amount"), &bulkAmount);
        bulkAmount = std::max(1, bulkAmount);
        if (bulkAmount >= (int)GetText("abcdefghijklmnopqrstuvwxyz").size())
            bulkAmount = GetText("abcdefghijklmnopqrstuvwxyz").size() - 1;
    }
    // Class letter and teacher
    else
    {
        ImGui::InputText(gettext("letter"), &timetable.classes[classId].letter);
        ImGui::Combo(gettext("teacher"), &teacherIndex, teacherValues.c_str());
    }
    ImGui::Separator();

    // Class available lessons
    ImGui::Text("%s", gettext("available lessons"));
    ImGui::Separator();
    ImGui::Columns(settings.daysPerWeek + 1);
    ImGui::LabelText("##1", "%s", "");
    ImGui::LabelText("##2", "%s", "");
    int pushId = 3;
    allAvailableLessonsHorizontal.resize(settings.lessonsPerDay, true);
    timetable.classes[classId].days.resize(settings.daysPerWeek);
    for (size_t i = 0; i < settings.lessonsPerDay; i++)
    {
        ImGui::PushID(pushId++);
        bool availableClassLessonsHorizontal = allAvailableLessonsHorizontal[i];
        if (ImGui::Checkbox(std::to_string(i).c_str(), &availableClassLessonsHorizontal))
        {
            LogInfo("Clicked allAvailableLessonsHorizontal number  " + std::to_string(i) +
                    " in class with id " + std::to_string(classId));
            allAvailableLessonsHorizontal[i] = availableClassLessonsHorizontal;
            for (size_t j = 0; j < settings.daysPerWeek; j++)
            {
                timetable.classes[classId].days[j].lessons.resize(settings.lessonsPerDay);
                timetable.classes[classId].days[j].lessons[i] = allAvailableLessonsHorizontal[i];
            }
        }
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();
    allAvailableLessonsVertical.resize(settings.daysPerWeek, false);
    timetable.classes[classId].days.resize(settings.daysPerWeek);
    for (size_t i = 0; i < settings.daysPerWeek; i++)
    {
        timetable.classes[classId].days[i].lessons.resize(settings.lessonsPerDay);
        int weekDay = i;
        while (weekDay >= 7) weekDay -= 7;
        ImGui::Text("%s", weekDays[weekDay].c_str());
        ImGui::PushID(pushId++);
        bool availableClassLessonsVertical = allAvailableLessonsVertical[i];
        if (ImGui::Checkbox(
                (allAvailableLessonsVertical[i] ? gettext("Deselect all") : gettext("Select all")),
                &availableClassLessonsVertical))
        {
            LogInfo("Clicked allAvailableLessonsVertical number  " + std::to_string(i) +
                    " in class with id " + std::to_string(classId));
            allAvailableLessonsVertical[i] = availableClassLessonsVertical;
            for (size_t j = 0; j < settings.lessonsPerDay; j++)
                timetable.classes[classId].days[i].lessons[j] = allAvailableLessonsVertical[i];
        }
        ImGui::PopID();
        for (size_t j = 0; j < settings.lessonsPerDay; j++)
        {
            ImGui::PushID(pushId++);
            bool isLessonAvailable = timetable.classes[classId].days[i].lessons[j];
            if (ImGui::Checkbox("", &isLessonAvailable))
            {
                timetable.classes[classId].days[i].lessons[j] = isLessonAvailable;
                LogInfo("Clicked isLessonAvailable in day " + std::to_string(i) +
                        " in lesson number " + std::to_string(j) + " in class with id " +
                        std::to_string(classId));
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
        LogInfo("Creating a new timetable lesson in a class with id " + std::to_string(classId));
        combineLessonsMenu->Open(&timetable, true, classId,
                                 timetable.classes[classId].maxTimetableLessonId + 1, lessons,
                                 lessonTeachers);
    }
    for (auto it = timetable.classes[classId].timetableLessons.begin();
         it != timetable.classes[classId].timetableLessons.end();)
    {
        if (it->second.lessonTeacherPairs.size() <= 1)
        {
            ++it;
            continue;
        }
        ImGui::PushID(pushId++);
        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a timetable lesson with id " + std::to_string(it->first) +
                    " in a class with id " + std::to_string(it->first));
            ImGui::PopID();
            it = timetable.classes[classId].timetableLessons.erase(it);
            break;
        }
        ImGui::SameLine();
        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a timetable lesson with id " + std::to_string(it->first) +
                    " in a class with id " + std::to_string(classId));
            combineLessonsMenu->Open(&timetable, false, classId, it->first, lessons,
                                     lessonTeachers);
        }
        ImGui::SameLine();
        std::string text = "";
        for (size_t j = 0; j < it->second.lessonTeacherPairs.size(); j++)
        {
            text += timetable.lessons[it->second.lessonTeacherPairs[j].lessonId].name + " (";
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
    for (auto& lesson: timetable.lessons)
    {
        if (!lessons[std::to_string(lesson.first) + "0"]) continue;
        bool anyTeacherSelected = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
            if (lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"])
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
        if (ImGui::Checkbox((std::string(allLessonTeachers[lesson.first] ? gettext("Deselect all")
                                                                         : gettext("Select all")) +
                             "##1")
                                .c_str(),
                            &allLessonTeachers[lesson.first]))
        {
            LogInfo("Clicked allLessonTeachers in class with id " + std::to_string(classId));
            for (auto& teacher: currentTimetable.teachers)
            {
                lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"] =
                    allLessonTeachers[lesson.first];
            }
        }
        ImGui::NextColumn();
        ImGui::PopID();
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
            ImGui::PushID(pushId++);
            ImGui::BeginDisabled(
                !lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::InputInt(lesson.second.name.c_str(),
                            &lessonAmounts[std::to_string(lesson.first) + teacher.second.name]);
            if (lessonAmounts[std::to_string(lesson.first) + teacher.second.name] < 0)
                lessonAmounts[std::to_string(lesson.first) + teacher.second.name] = 0;
            ImGui::EndDisabled();
            ImGui::NextColumn();
            ImGui::Checkbox(
                teacher.second.name.c_str(),
                &lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Separator();
    }
    ImGui::Columns(1);

    // Additional rules
    if (ImGui::Button(gettext("Add a lesson rule")))
    {
        LogInfo("Adding a lesson rule to a class with id " + std::to_string(classId));
        rulesMenu->Open(&timetable, true, timetable.classes[classId].timetableLessonRules.size(),
                        classId, lessons, lessonAmounts, lessonTeachers);
    }
    ImGui::Columns(3);
    ImGui::Text("%s", gettext("Rules"));
    ImGui::NextColumn();
    ImGui::Text("%s", gettext("Preserve order"));
    ImGui::NextColumn();
    ImGui::Text("%s", gettext("Amount"));
    ImGui::NextColumn();
    ImGui::Separator();
    for (size_t i = 0; i < timetable.classes[classId].timetableLessonRules.size(); i++)
    {
        ImGui::PushID(pushId++);

        if (ImGui::Button(gettext("-")))
        {
            timetable.classes[classId].timetableLessonRules.erase(
                timetable.classes[classId].timetableLessonRules.begin() + i);
            i--;
            ImGui::PopID();
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a lesson rule in a class with id " + std::to_string(classId));
            rulesMenu->Open(&timetable, false, i, classId, lessons, lessonAmounts, lessonTeachers);
        }
        ImGui::SameLine();

        TimetableLessonRule& rule = timetable.classes[classId].timetableLessonRules[i];
        std::string ruleName = "";
        for (size_t j = 0; j < rule.timetableLessonIds.size(); j++)
        {
            int timetableLessonId = rule.timetableLessonIds[j];
            TimetableLesson& timetableLesson =
                timetable.classes[classId].timetableLessons[timetableLessonId];
            for (size_t k = 0; k < timetableLesson.lessonTeacherPairs.size(); k++)
            {
                LessonTeacherPair& lessonTeacherPair = timetableLesson.lessonTeacherPairs[k];
                ruleName += timetable.lessons[lessonTeacherPair.lessonId].name + " (" +
                            currentTimetable.teachers[lessonTeacherPair.teacherId].name + ")";
                if (k < timetableLesson.lessonTeacherPairs.size() - 1) ruleName += '\n';
            }
            if (j < rule.timetableLessonIds.size() - 1) ruleName += "\n\n";
        }
        ImGui::Text("%s", ruleName.c_str());
        ImGui::NextColumn();

        ImGui::Text("%s", (rule.preserveOrder ? gettext("Yes") : gettext("No")));
        ImGui::NextColumn();

        ImGui::Text("%d", rule.amount);
        ImGui::NextColumn();
        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked the Ok button while editing class with id " + std::to_string(classId));
        LoadTimetableLessonsFromSelection(timetable, classId, lessons, lessonAmounts,
                                          lessonTeachers);
        if (bulkEdit)
        {
            ChangeClassesAmount(timetable, timetable.classes[classId].number, bulkAmount);
            for (auto& classPair: timetable.classes)
            {
                if (classPair.first == classId) continue;
                if (classPair.second.number == timetable.classes[classId].number)
                {
                    int teacherId = classPair.second.teacherId;
                    classPair.second = timetable.classes[classId];
                    classPair.second.teacherId = teacherId;
                }
            }
            UpdateClassLetters(timetable);
        }
        else
        {
            if (teacherIndex >= 0 && (size_t)teacherIndex < teacherIds.size())
            {
                timetable.classes[classId].teacherId = teacherIds[teacherIndex];
            }
            else
            {
                timetable.classes[classId].teacherId = -1;
            }
        }
        prevTimetable->classes = timetable.classes;
        prevTimetable->maxClassId = timetable.maxClassId;
        prevTimetable->orderedClasses = timetable.orderedClasses;
        prevTimetable->lessons = timetable.lessons;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
