// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes/Edit.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include "UI/Classes/Edit/CombineLessons.hpp"
#include "UI/Classes/Edit/Rules.hpp"
#include "UI/Classes/Utils.hpp"
#include "Utils.hpp"
#include "Widgets/Window.hpp"
#include <algorithm>
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <misc/cpp/imgui_stdlib.h>
#include <string>

std::shared_ptr<EditClassMenu> g_editClassMenu;

void EditClassMenu::Open(Timetable* prevTimetable, bool newClass, int classId, bool bulkEdit,
                         int bulkCount)
{
    this->prevTimetable_ = prevTimetable;
    this->newClass_ = newClass;
    this->classId_ = classId;
    this->bulkEdit_ = bulkEdit;
    this->bulkCount_ = bulkCount;

    timetable_ = *prevTimetable;

    if (newClass)
    {
        timetable_.orderedClasses.insert(timetable_.orderedClasses.begin() + classId,
                                         timetable_.maxClassId + 1);
        timetable_.classes[classId] = Class();
        timetable_.classes[classId].number = "0";
        timetable_.classes[timetable_.maxClassId].letter =
            GetNthUtf8Character(GetText("abcdefghijklmnopqrstuvwxyz"), classId);

        classId = timetable_.maxClassId + 1;
    }

    LogInfo("Resetting class variables");

    allAvailableLessonsVertical_.clear();
    allAvailableLessonsVertical_.resize(g_settings.daysPerWeek, true);
    allAvailableLessonsHorizontal_.clear();
    allAvailableLessonsHorizontal_.resize(g_settings.lessonsPerDay, true);

    timetable_.classes[classId].days.resize(g_settings.daysPerWeek);
    for (int i = 0; i < g_settings.daysPerWeek; i++)
    {
        if ((int)timetable_.classes[classId].days[i].lessons.size() < g_settings.lessonsPerDay)
        {
            int iterations =
                g_settings.lessonsPerDay - (int)timetable_.classes[classId].days[i].lessons.size();
            for (int j = 0; j < iterations; j++)
                timetable_.classes[classId].days[i].lessons.push_back(newClass);
        }
    }

    allLessons_ = true;
    allLessonTeachers_.clear();
    for (auto& lesson: timetable_.lessons) allLessonTeachers_[lesson.first] = true;
    lessons_.clear();

    lessonTeachers_.clear();
    for (auto& lesson: timetable_.lessons)
    {
        lessons_[std::to_string(lesson.first) + "0"] = false;
        for (auto& teacher: prevTimetable->teachers)
        {
            lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "0"] = false;
            lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "1"] = newClass;
            lessonCounts_[std::to_string(lesson.first) + teacher.second.name] = 1;
        }
    }

    for (auto& lesson: timetable_.lessons)
    {
        bool classIdFound = false;
        for (size_t i = 0; i < lesson.second.classIds.size(); i++)
        {
            if (classId == lesson.second.classIds[i] ||
                (timetable_.classes[classId].number ==
                     timetable_.classes[lesson.second.classIds[i]].number &&
                 bulkEdit))
            {
                classIdFound = true;
                break;
            }
        }
        if (!classIdFound) continue;
        lessons_[std::to_string(lesson.first) + "0"] = true;
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
            lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "0"] = true;
        }
    }

    for (auto& lesson: timetable_.lessons)
    {
        for (auto& teacher: prevTimetable->teachers)
        {
            bool lessonTeacherPairFound = false;
            int lessonTeacherPairId = -1;
            for (auto& timetableLesson: timetable_.classes[classId].timetableLessons)
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
            lessonCounts_[std::to_string(lesson.first) + teacher.second.name] =
                timetable_.classes[classId].timetableLessons[lessonTeacherPairId].count;
            lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "1"] = true;
        }
    }

    // Reset teacher values
    teacherValues_ = GetText("none");
    teacherValues_ += '\0';
    teacherIds_.clear();
    teacherIds_.push_back(-1);
    for (auto& teacher: g_currentTimetable.teachers)
    {
        bool isTeacherTaken = false;
        for (auto& classPair: timetable_.classes)
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
                teacherValues_ += GetText("error");
            }
            else
            {
                teacherValues_ += teacher.second.name;
            }
            teacherIds_.push_back(teacher.first);
            teacherValues_ += '\0';
        }
    }
    teacherValues_ += '\0';

    teacherIndex_ = 0;
    for (size_t i = 0; i < teacherIds_.size(); i++)
    {
        if (timetable_.classes[classId].teacherId == teacherIds_[i])
        {
            teacherIndex_ = (int)i;
            break;
        }
    }

    if (newClass)
    {
        FetchClassLessonsFromSimularClasses(timetable_, classId);
    }

    Window::Open();
}

void EditClassMenu::Draw()
{
    if (!ImGui::Begin((newClass_ ? gettext("New class") : gettext("Edit class")), &visible_))
    {
        ImGui::End();
        return;
    }

    // Bulk editing warning
    if (bulkEdit_ && !newClass_)
    {
        ImGui::TextColored(
            COLOR_WARNING, "%s",
            gettext(
                "Warning: you are bulk editing classes!\nAfter pressing Ok ALL classes with the number below\nwill be OVERWRITTEN with the data you enter.\nIf you don't want that to happen, press the Cancel button."));
    }

    // Class number
    if (ImGui::InputText(gettext("number"), &timetable_.classes[classId_].number))
    {
        Open(prevTimetable_, true, classId_, bulkEdit_, bulkCount_);
        timetable_.classes[classId_].timetableLessons.clear();
        timetable_.classes[classId_].maxTimetableLessonId = 0;
    }

    // Classes count
    if (bulkEdit_)
    {
        ImGui::InputInt(gettext("count"), &bulkCount_);
        bulkCount_ = std::max(1, bulkCount_);
        // TODO: This is incorrect in UTF-8
        if (bulkCount_ >= (int)GetText("abcdefghijklmnopqrstuvwxyz").size())
            bulkCount_ = (int)GetText("abcdefghijklmnopqrstuvwxyz").size() - 1;
    }
    // Class letter and teacher
    else
    {
        ImGui::InputText(gettext("letter"), &timetable_.classes[classId_].letter);
        ImGui::Combo(gettext("teacher"), &teacherIndex_, teacherValues_.c_str());
    }
    ImGui::Separator();

    // Class available lessons
    ImGui::Text("%s", gettext("available lessons"));
    ImGui::Separator();
    ImGui::Columns(g_settings.daysPerWeek + 1);
    ImGui::LabelText("##1", "%s", "");
    ImGui::LabelText("##2", "%s", "");
    int pushId = 3;
    allAvailableLessonsHorizontal_.resize(g_settings.lessonsPerDay, true);
    timetable_.classes[classId_].days.resize(g_settings.daysPerWeek);
    for (int i = 0; i < g_settings.lessonsPerDay; i++)
    {
        ImGui::PushID(pushId++);
        bool availableClassLessonsHorizontal = allAvailableLessonsHorizontal_[i];
        if (ImGui::Checkbox(std::to_string(i).c_str(), &availableClassLessonsHorizontal))
        {
            LogInfo("Clicked allAvailableLessonsHorizontal number %d in class with id %d", i,
                    classId_);
            allAvailableLessonsHorizontal_[i] = availableClassLessonsHorizontal;
            for (int j = 0; j < g_settings.daysPerWeek; j++)
            {
                timetable_.classes[classId_].days[j].lessons.resize(g_settings.lessonsPerDay);
                timetable_.classes[classId_].days[j].lessons[i] = allAvailableLessonsHorizontal_[i];
            }
        }
        ImGui::PopID();
        pushId++;
    }
    ImGui::NextColumn();
    allAvailableLessonsVertical_.resize(g_settings.daysPerWeek, false);
    timetable_.classes[classId_].days.resize(g_settings.daysPerWeek);
    for (int i = 0; i < g_settings.daysPerWeek; i++)
    {
        timetable_.classes[classId_].days[i].lessons.resize(g_settings.lessonsPerDay);
        size_t weekDay = i;
        while (weekDay >= g_weekDays.size()) weekDay -= g_weekDays.size();
        ImGui::Text("%s", g_weekDays[weekDay]);
        ImGui::PushID(pushId++);
        bool availableClassLessonsVertical = allAvailableLessonsVertical_[i];
        if (ImGui::Checkbox(
                (allAvailableLessonsVertical_[i] ? gettext("Deselect all") : gettext("Select all")),
                &availableClassLessonsVertical))
        {
            LogInfo("Clicked allAvailableLessonsVertical number %d in class with id %d", i,
                    classId_);
            allAvailableLessonsVertical_[i] = availableClassLessonsVertical;
            for (int j = 0; j < g_settings.lessonsPerDay; j++)
                timetable_.classes[classId_].days[i].lessons[j] = allAvailableLessonsVertical_[i];
        }
        ImGui::PopID();
        for (int j = 0; j < g_settings.lessonsPerDay; j++)
        {
            ImGui::PushID(pushId++);
            bool isLessonAvailable = timetable_.classes[classId_].days[i].lessons[j];
            if (ImGui::Checkbox("", &isLessonAvailable))
            {
                timetable_.classes[classId_].days[i].lessons[j] = isLessonAvailable;
                LogInfo(
                    "Clicked isLessonAvailable in day %d in lesson number %d in class with id %d",
                    i, j, classId_);
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
        LogInfo("Creating a new timetable lesson in a class with id %d", classId_);
        g_combineLessonsMenu->Open(&timetable_, true, classId_,
                                   timetable_.classes[classId_].maxTimetableLessonId + 1, lessons_,
                                   lessonTeachers_);
    }
    for (auto it = timetable_.classes[classId_].timetableLessons.begin();
         it != timetable_.classes[classId_].timetableLessons.end();)
    {
        if (it->second.lessonTeacherPairs.size() <= 1)
        {
            ++it;
            continue;
        }
        ImGui::PushID(pushId++);
        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a timetable lesson with id %d in a class with id %d", it->first,
                    classId_);
            ImGui::PopID();
            it = timetable_.classes[classId_].timetableLessons.erase(it);
            break;
        }
        ImGui::SameLine();
        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a timetable lesson with id %d in a class with id %d", it->first,
                    classId_);
            g_combineLessonsMenu->Open(&timetable_, false, classId_, it->first, lessons_,
                                       lessonTeachers_);
        }
        ImGui::SameLine();
        std::string text;
        for (size_t j = 0; j < it->second.lessonTeacherPairs.size(); j++)
        {
            text += timetable_.lessons[it->second.lessonTeacherPairs[j].lessonId].name + " (";
            text +=
                g_currentTimetable.teachers[it->second.lessonTeacherPairs[j].teacherId].name + ")";
            if (j < it->second.lessonTeacherPairs.size() - 1) text += "\n";
        }
        ImGui::InputInt(text.c_str(), &it->second.count);
        ImGui::PopID();
        ++it;
    }
    ImGui::Separator();
    ImGui::Columns(2);

    // Lessons
    for (auto& lesson: timetable_.lessons)
    {
        if (!lessons_[std::to_string(lesson.first) + "0"]) continue;
        bool anyTeacherSelected = false;
        for (auto& teacher: g_currentTimetable.teachers)
        {
            if (!lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            if (lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "1"])
            {
                anyTeacherSelected = true;
                break;
            }
        }
        if (!anyTeacherSelected)
        {
            ImGui::PushID(pushId++);
            ImGui::TextColored(COLOR_WARNING, "%s",
                               gettext("Warning: no teacher selected for this lesson"));
            ImGui::PopID();
        }
        ImGui::PushID(pushId++);
        ImGui::NextColumn();
        if (ImGui::Checkbox((std::string(allLessonTeachers_[lesson.first] ? gettext("Deselect all")
                                                                          : gettext("Select all")) +
                             "##1")
                                .c_str(),
                            &allLessonTeachers_[lesson.first]))
        {
            LogInfo("Clicked allLessonTeachers in class with id %d", classId_);
            for (auto& teacher: g_currentTimetable.teachers)
            {
                lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "1"] =
                    allLessonTeachers_[lesson.first];
            }
        }
        ImGui::NextColumn();
        ImGui::PopID();
        for (auto& teacher: g_currentTimetable.teachers)
        {
            if (!lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "0"])
                continue;
            ImGui::PushID(pushId++);
            ImGui::BeginDisabled(
                !lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::InputInt(lesson.second.name.c_str(),
                            &lessonCounts_[std::to_string(lesson.first) + teacher.second.name]);
            lessonCounts_[std::to_string(lesson.first) + teacher.second.name] =
                std::max(lessonCounts_[std::to_string(lesson.first) + teacher.second.name], 0);
            ImGui::EndDisabled();
            ImGui::NextColumn();
            ImGui::Checkbox(
                teacher.second.name.c_str(),
                &lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "1"]);
            ImGui::NextColumn();
            ImGui::PopID();
        }
        ImGui::Separator();
    }
    ImGui::Columns(1);

    // Additional rules
    if (ImGui::Button(gettext("Add a lesson rule")))
    {
        LogInfo("Adding a lesson rule to a class with id %d", classId_);
        g_rulesMenu->Open(&timetable_, true, timetable_.classes[classId_].maxTimetableLessonId + 1,
                          classId_, lessons_, lessonCounts_, lessonTeachers_);
    }
    ImGui::Columns(3);
    ImGui::Text("%s", gettext("Rules"));
    ImGui::NextColumn();
    ImGui::Text("%s", gettext("Preserve order"));
    ImGui::NextColumn();
    ImGui::Text("%s", gettext("Count"));
    ImGui::NextColumn();
    ImGui::Separator();
    for (size_t i = 0; i < timetable_.classes[classId_].timetableLessonRules.size(); i++)
    {
        ImGui::PushID(pushId++);

        if (ImGui::Button(gettext("-")))
        {
            timetable_.classes[classId_].timetableLessonRules.erase(
                timetable_.classes[classId_].timetableLessonRules.begin() + (int)i);
            i--;
            ImGui::PopID();
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            LogInfo("Editing a lesson rule in a class with id %d", classId_);
            g_rulesMenu->Open(&timetable_, false, (int)i, classId_, lessons_, lessonCounts_,
                              lessonTeachers_);
        }
        ImGui::SameLine();

        TimetableLessonRule& rule = timetable_.classes[classId_].timetableLessonRules[i];
        std::string ruleName;
        for (size_t j = 0; j < rule.timetableLessonIds.size(); j++)
        {
            int timetableLessonId = rule.timetableLessonIds[j];
            TimetableLesson& timetableLesson =
                timetable_.classes[classId_].timetableLessons[timetableLessonId];
            for (size_t k = 0; k < timetableLesson.lessonTeacherPairs.size(); k++)
            {
                LessonTeacherPair& lessonTeacherPair = timetableLesson.lessonTeacherPairs[k];
                ruleName += timetable_.lessons[lessonTeacherPair.lessonId].name + " (" +
                            g_currentTimetable.teachers[lessonTeacherPair.teacherId].name + ")";
                if (k < timetableLesson.lessonTeacherPairs.size() - 1) ruleName += '\n';
            }
            if (j < rule.timetableLessonIds.size() - 1) ruleName += "\n\n";
        }
        ImGui::Text("%s", ruleName.c_str());
        ImGui::NextColumn();

        ImGui::Text("%s", (rule.preserveOrder ? gettext("Yes") : gettext("No")));
        ImGui::NextColumn();

        ImGui::Text("%d", rule.count);
        ImGui::NextColumn();
        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked the Ok button while editing class with id %d", classId_);
        LoadTimetableLessonsFromSelection(timetable_, classId_, lessons_, lessonCounts_,
                                          lessonTeachers_);
        if (bulkEdit_)
        {
            ChangeClassesCount(timetable_, timetable_.classes[classId_].number, bulkCount_);
            for (auto& classPair: timetable_.classes)
            {
                if (classPair.first == classId_) continue;
                if (classPair.second.number == timetable_.classes[classId_].number)
                {
                    int teacherId = classPair.second.teacherId;
                    classPair.second = timetable_.classes[classId_];
                    classPair.second.teacherId = teacherId;
                }
            }
            UpdateClassLetters(timetable_);
        }
        else
        {
            if (teacherIndex_ >= 0 && (size_t)teacherIndex_ < teacherIds_.size())
            {
                timetable_.classes[classId_].teacherId = teacherIds_[teacherIndex_];
            }
            else
            {
                timetable_.classes[classId_].teacherId = -1;
            }
        }
        prevTimetable_->classes = timetable_.classes;
        prevTimetable_->maxClassId = timetable_.maxClassId;
        prevTimetable_->orderedClasses = timetable_.orderedClasses;
        prevTimetable_->lessons = timetable_.lessons;
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
