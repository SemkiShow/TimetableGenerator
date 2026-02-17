// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Classes/Edit/CombineLessons.hpp"
#include "Logging.hpp"
#include "Translations.hpp"
#include <imgui.h>

std::shared_ptr<CombineLessonsMenu> combineLessonsMenu;

void CombineLessonsMenu::Open(Timetable* prevTimetable, bool newCombinedLesson, int classId,
                              int lessonId,
                              const std::unordered_map<std::string, bool>& classLessons,
                              const std::unordered_map<std::string, bool>& classLessonTeachers)
{
    this->prevTimetable = prevTimetable;
    this->newCombinedLesson = newCombinedLesson;
    this->classId = classId;
    this->lessonId = lessonId;
    this->classLessons = classLessons;
    this->classLessonTeachers = classLessonTeachers;

    timetable = *prevTimetable;
    if (newCombinedLesson)
    {
        timetable.classes[classId].timetableLessons[lessonId] = TimetableLesson();
    }

    // Set everything to false
    lessons.clear();
    lessonTeachers.clear();
    for (auto& lesson: timetable.lessons)
    {
        if (!this->classLessons[std::to_string(lesson.first) + "0"]) continue;
        lessons[lesson.first] = false;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!this->classLessonTeachers[std::to_string(lesson.first) + teacher.second.name +
                                           "0"])
                continue;
            lessonTeachers[std::to_string(lesson.first) + teacher.second.name] = false;
        }
    }

    // Load already selected stuff, if editing
    if (!newCombinedLesson)
    {
        TimetableLesson& currentLesson = timetable.classes[classId].timetableLessons[lessonId];
        for (size_t j = 0; j < currentLesson.lessonTeacherPairs.size(); j++)
        {
            if (!this->classLessons[std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) +
                                    "0"])
                continue;
            if (!this->classLessonTeachers
                     [std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) +
                      currentTimetable.teachers[currentLesson.lessonTeacherPairs[j].teacherId]
                          .name +
                      "0"])
                continue;
            lessons[currentLesson.lessonTeacherPairs[j].lessonId] = true;
            lessonTeachers[std::to_string(currentLesson.lessonTeacherPairs[j].lessonId) +
                           currentTimetable.teachers[currentLesson.lessonTeacherPairs[j].teacherId]
                               .name] = true;
        }
    }

    Window::Open();
}

void CombineLessonsMenu::Draw()
{
    if (!ImGui::Begin(gettext("Combine lessons"), &visible))
    {
        ImGui::End();
        return;
    }

    // Lessons
    ImGui::Columns(2);
    int pushId = 0;
    for (auto& lesson: prevTimetable->lessons)
    {
        if (!classLessons[std::to_string(lesson.first) + "0"]) continue;
        ImGui::PushID(pushId);
        ImGui::Checkbox(lesson.second.name.c_str(), &lessons[lesson.first]);
        ImGui::NextColumn();
        ImGui::PopID();
        pushId++;
        for (auto& teacher: currentTimetable.teachers)
        {
            if (!lessonTeachers[std::to_string(lesson.first) + teacher.second.name + "0"]) continue;
            ImGui::PushID(pushId);
            ImGui::Checkbox(teacher.second.name.c_str(),
                            &lessonTeachers[std::to_string(lesson.first) + teacher.second.name]);
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
        LogInfo("Pressed the Ok button in combine lessons of class with id " +
                std::to_string(classId));
        prevTimetable->classes[classId].timetableLessons[lessonId].lessonTeacherPairs.clear();
        int counter = 0;
        for (auto& lesson: prevTimetable->lessons)
        {
            if (!lessons[lesson.first]) continue;
            for (auto& teacher: currentTimetable.teachers)
            {
                if (!lessonTeachers[std::to_string(lesson.first) + teacher.second.name]) continue;
                auto& lessonTeacherPairs =
                    prevTimetable->classes[classId].timetableLessons[lessonId].lessonTeacherPairs;
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
