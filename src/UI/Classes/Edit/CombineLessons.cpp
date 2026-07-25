// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes/Edit/CombineLessons.hpp"
#include "Logging.hpp"
#include "Timetable.hpp"
#include "Translations.hpp"
#include "Widgets/Window.hpp"
#include <cstddef>
#include <imgui.h>
#include <memory>
#include <string>
#include <unordered_map>

std::shared_ptr<CombineLessonsMenu> g_combineLessonsMenu;

void CombineLessonsMenu::Open(Timetable* prevTimetable, bool newCombinedLesson, int classId,
                              int lessonId,
                              const std::unordered_map<std::string, bool>& classLessons,
                              const std::unordered_map<std::string, bool>& classLessonTeachers)
{
    this->prevTimetable_ = prevTimetable;
    this->newCombinedLesson_ = newCombinedLesson;
    this->classId_ = classId;
    this->lessonId_ = lessonId;
    this->classLessons_ = classLessons;
    this->classLessonTeachers_ = classLessonTeachers;

    timetable_ = *prevTimetable;
    if (newCombinedLesson)
    {
        timetable_.classes[classId].timetableLessons[lessonId] = TimetableLesson();
    }

    // Set everything to false
    lessons_.clear();
    lessonTeachers_.clear();
    for (auto& lesson: timetable_.lessons)
    {
        if (!this->classLessons_[std::to_string(lesson.first) + "0"]) continue;
        lessons_[lesson.first] = false;
        for (auto& teacher: g_currentTimetable.teachers)
        {
            if (!this->classLessonTeachers_[std::to_string(lesson.first) + teacher.second.name +
                                           "0"])
                continue;
            lessonTeachers_[std::to_string(lesson.first) + teacher.second.name] = false;
        }
    }

    // Load already selected stuff, if editing
    if (!newCombinedLesson)
    {
        TimetableLesson& currentLesson = timetable_.classes[classId].timetableLessons[lessonId];
        for (size_t j = 0; j < currentLesson.lessonTeacherPairs.size(); j++)
        {
            if (!this->classLessons_[std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) +
                                    "0"])
                continue;
            if (!this->classLessonTeachers_
                     [std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) +
                      g_currentTimetable.teachers[currentLesson.lessonTeacherPairs[j].teacherId]
                          .name +
                      "0"])
                continue;
            lessons_[currentLesson.lessonTeacherPairs[j].lessonId] = true;
            lessonTeachers_[std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) +
                           g_currentTimetable.teachers[currentLesson.lessonTeacherPairs[j].teacherId]
                               .name] = true;
        }
    }

    Window::Open();
}

void CombineLessonsMenu::Draw()
{
    if (!ImGui::Begin(gettext("Combine lessons"), &visible_))
    {
        ImGui::End();
        return;
    }

    // Lessons
    ImGui::Columns(2);
    int pushId = 0;
    for (auto& lesson: prevTimetable_->lessons)
    {
        if (!classLessons_[std::to_string(lesson.first) + "0"]) continue;
        ImGui::PushID(pushId);
        ImGui::Checkbox(lesson.second.name.c_str(), &lessons_[lesson.first]);
        ImGui::NextColumn();
        ImGui::PopID();
        pushId++;
        for (auto& teacher: g_currentTimetable.teachers)
        {
            if (!lessonTeachers_[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
            ImGui::PushID(pushId);
            ImGui::Checkbox(teacher.second.name.c_str(),
                            &lessonTeachers_[std::to_string(lesson.first) + teacher.second.name]);
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
        LogInfo("Pressed the Ok button in combine lessons of class with id %d", classId_);
        prevTimetable_->classes[classId_].timetableLessons[lessonId_].lessonTeacherPairs.clear();
        int counter = 0;
        for (auto& lesson: prevTimetable_->lessons)
        {
            if (!lessons_[lesson.first]) continue;
            for (auto& teacher: g_currentTimetable.teachers)
            {
                if (!lessonTeachers_[std::to_string(lesson.first) + teacher.second.name]) continue;
                auto& lessonTeacherPairs =
                    prevTimetable_->classes[classId_].timetableLessons[lessonId_].lessonTeacherPairs;
                lessonTeacherPairs.push_back(LessonTeacherPair());
                lessonTeacherPairs[counter].lessonId = lesson.first;
                lessonTeacherPairs[counter].teacherId = teacher.first;
                counter++;
            }
        }
        Close();
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) Close();
    ImGui::End();
}
