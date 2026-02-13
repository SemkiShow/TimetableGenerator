// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Logging.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <string>
#include <unordered_map>

int currentTeacherID = 0;
bool newTeacher = false;
bool allTeacherLessons = true;
std::unordered_map<int, bool> teacherLessons;
std::vector<int> allAvailableTeacherLessonsVertical;
std::vector<int> allAvailableTeacherLessonsHorizontal;
std::unordered_map<int, int> availableTeacherLessons;
std::string teacherLessonValues = "";

void ResetTeacherLessonValues()
{
    teacherLessonValues = "";
    teacherLessonValues += gettext("no lesson");
    teacherLessonValues += '\0';
    teacherLessonValues += gettext("any lesson");
    teacherLessonValues += '\0';
    for (auto& lesson: currentTimetable.lessons)
    {
        if (!teacherLessons[lesson.first]) continue;
        if (lesson.second.name == "")
            teacherLessonValues += gettext("error");
        else
            teacherLessonValues += lesson.second.name;
        teacherLessonValues += '\0';
    }
    teacherLessonValues += '\0';
}

static void ResetVariables()
{
    LogInfo("Resetting teacher variables");
    allTeacherLessons = false;
    allAvailableTeacherLessonsVertical.clear();
    allAvailableTeacherLessonsVertical.resize(daysPerWeek, 1);
    allAvailableTeacherLessonsHorizontal.clear();
    allAvailableTeacherLessonsHorizontal.resize(lessonsPerDay, 1);
    teacherLessons.clear();
    for (auto& lesson: currentTimetable.lessons)
        teacherLessons[lesson.first] = false;
    for (size_t i = 0; i < tmpTmpTimetable.teachers[currentTeacherID].lessonIDs.size(); i++)
        teacherLessons[tmpTmpTimetable.teachers[currentTeacherID].lessonIDs[i]] = true;
    availableTeacherLessons.clear();
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        for (size_t j = 0; j < lessonsPerDay; j++)
            availableTeacherLessons[i * lessonsPerDay + j] = 1;
    }
    tmpTmpTimetable.teachers[currentTeacherID].workDays.resize(daysPerWeek);
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        for (size_t j = 0;
             j < tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.size(); j++)
        {
            int lessonID = tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs[j];
            if (lessonID == ANY_LESSON || lessonID == NO_LESSON)
                availableTeacherLessons[i * lessonsPerDay + j] = lessonID + 3;
            else
            {
                int counter = 2;
                for (auto& lesson: currentTimetable.lessons)
                {
                    if (lessonID == lesson.first)
                    {
                        availableTeacherLessons[i * lessonsPerDay + j] = counter;
                        break;
                    }
                    counter++;
                }
            }
        }
    }
    if (newTeacher)
    {
        tmpTmpTimetable.teachers[currentTeacherID].workDays.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.resize(lessonsPerDay);
            for (size_t j = 0; j < lessonsPerDay; j++)
                tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.push_back(1);
        }
    }

    ResetTeacherLessonValues();
}

bool isEditTeacher = false;
void ShowEditTeacher(bool* isOpen)
{
    if (!ImGui::Begin((newTeacher ? gettext("New teacher") : gettext("Edit teacher")), isOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::InputText(gettext("name"), &tmpTmpTimetable.teachers[currentTeacherID].name);
    ImGui::Separator();
    ImGui::Text("%s", gettext("lessons"));

    // Deselect/select all lessons
    if (ImGui::Checkbox(
            (std::string(allTeacherLessons ? gettext("Deselect all") : gettext("Select all")) +
             "##1")
                .c_str(),
            &allTeacherLessons))
    {
        LogInfo("Clicked allTeacherLessons in a teacher with ID " +
                std::to_string(currentTeacherID));
        for (auto& lesson: currentTimetable.lessons)
            teacherLessons[lesson.first] = allTeacherLessons;
    }

    // No lessons warning
    if (currentTimetable.lessons.size() == 0)
    {
        ImGui::TextColored(
            ImVec4(255, 0, 0, 255), "%s",
            gettext(
                "You need to add lessons\nin the Lessons menu\nto select lessons for this teacher!"));
    }

    // Lesons
    ImGui::Columns(3);
    int pushID = 0;
    for (auto& lesson: currentTimetable.lessons)
    {
        ImGui::PushID(pushID);
        if (ImGui::Checkbox(lesson.second.name.c_str(), &teacherLessons[lesson.first]))
        {
            ResetTeacherLessonValues();
        }
        ImGui::NextColumn();
        std::string classNames = "";
        for (size_t j = 0; j < lesson.second.classIDs.size(); j++)
        {
            classNames += currentTimetable.classes[lesson.second.classIDs[j]].number;
            classNames += currentTimetable.classes[lesson.second.classIDs[j]].letter;
            if (j < lesson.second.classIDs.size() - 1) classNames += ' ';
        }
        ImGui::Text("%s", classNames.c_str());
        ImGui::NextColumn();
        std::string lessonClassrooms = "";
        for (size_t j = 0; j < lesson.second.classroomIDs.size(); j++)
        {
            lessonClassrooms += currentTimetable.classrooms[lesson.second.classroomIDs[j]].name;
            if (j < lesson.second.classroomIDs.size() - 1) lessonClassrooms += ' ';
        }
        ImGui::Text("%s", lessonClassrooms.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
        pushID++;
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Available lessons
    ImGui::Text("%s", gettext("available lessons"));
    ImGui::Columns(daysPerWeek + 1);
    ImGui::LabelText("##1", "%s", "");
    ImGui::LabelText("##2", "%s", "");
    allAvailableTeacherLessonsHorizontal.resize(lessonsPerDay, 1);
    for (size_t i = 0; i < lessonsPerDay; i++)
    {
        ImGui::PushID(pushID);
        if (ImGui::Combo(std::to_string(i).c_str(), &allAvailableTeacherLessonsHorizontal[i],
                         teacherLessonValues.c_str()))
        {
            LogInfo("Clicked allAvailableTeacherLessonsHorizontal in a teacher with ID " +
                    std::to_string(currentTeacherID));
            for (size_t j = 0; j < daysPerWeek; j++)
            {
                availableTeacherLessons[j * lessonsPerDay + i] =
                    allAvailableTeacherLessonsHorizontal[i];
            }
        }
        ImGui::PopID();
        pushID++;
    }
    ImGui::NextColumn();
    allAvailableTeacherLessonsVertical.resize(daysPerWeek, 1);
    for (size_t i = 0; i < daysPerWeek; i++)
    {
        int weekDay = i;
        while (weekDay >= 7)
            weekDay -= 7;
        ImGui::Text("%s", weekDays[weekDay].c_str());
        ImGui::PushID(pushID);
        if (ImGui::Combo("", &allAvailableTeacherLessonsVertical[i], teacherLessonValues.c_str()))
        {
            LogInfo("Clicked allAvailableTeacherLessonsVertical in a teacher with ID " +
                    std::to_string(currentTeacherID));
            for (size_t j = 0; j < lessonsPerDay; j++)
                availableTeacherLessons[i * lessonsPerDay + j] =
                    allAvailableTeacherLessonsVertical[i];
        }
        ImGui::PopID();
        pushID++;
        for (size_t j = 0; j < lessonsPerDay; j++)
        {
            ImGui::PushID(pushID);
            ImGui::Combo("", &availableTeacherLessons[i * lessonsPerDay + j],
                         teacherLessonValues.c_str());
            ImGui::PopID();
            pushID++;
        }
        ImGui::NextColumn();
    }
    ImGui::Columns(1);

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok while editing a teacher with ID " + std::to_string(currentTeacherID));
        tmpTmpTimetable.teachers[currentTeacherID].lessonIDs.clear();
        for (auto& lesson: currentTimetable.lessons)
        {
            if (teacherLessons[lesson.first])
                tmpTmpTimetable.teachers[currentTeacherID].lessonIDs.push_back(lesson.first);
        }
        tmpTmpTimetable.teachers[currentTeacherID].workDays.resize(daysPerWeek);
        for (size_t i = 0; i < daysPerWeek; i++)
        {
            tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.clear();
            for (size_t j = 0; j < lessonsPerDay; j++)
            {
                if (availableTeacherLessons[i * lessonsPerDay + j] == 0 ||
                    availableTeacherLessons[i * lessonsPerDay + j] == 1)
                    tmpTmpTimetable.teachers[currentTeacherID].workDays[i].lessonIDs.push_back(
                        availableTeacherLessons[i * lessonsPerDay + j] - 3);
                else
                {
                    availableTeacherLessons[i * lessonsPerDay + j] -= 2;
                    for (auto& lesson: currentTimetable.lessons)
                    {
                        if (availableTeacherLessons[i * lessonsPerDay + j] <= 0)
                        {
                            tmpTmpTimetable.teachers[currentTeacherID]
                                .workDays[i]
                                .lessonIDs.push_back(lesson.first);
                            break;
                        }
                        availableTeacherLessons[i * lessonsPerDay + j]--;
                    }
                }
            }
        }
        tmpTimetable.teachers = tmpTmpTimetable.teachers;
        tmpTimetable.maxTeacherID = tmpTmpTimetable.maxTeacherID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}

bool isTeachers = false;
void ShowTeachers(bool* isOpen)
{
    if (!ImGui::Begin(gettext("Teachers"), isOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::InputInt(gettext("min free periods"), &minFreePeriods);
    if (minFreePeriods < 0) minFreePeriods = 0;
    ImGui::InputInt(gettext("max free periods"), &maxFreePeriods);
    if (maxFreePeriods < 0) maxFreePeriods = 0;
    if (maxFreePeriods < minFreePeriods) maxFreePeriods = minFreePeriods;

    if (ImGui::Button(gettext("+")))
    {
        newTeacher = true;
        tmpTmpTimetable.teachers = tmpTimetable.teachers;
        tmpTmpTimetable.maxTeacherID = tmpTimetable.maxTeacherID;
        tmpTmpTimetable.maxTeacherID++;
        tmpTmpTimetable.teachers[tmpTmpTimetable.maxTeacherID] = Teacher();
        currentTeacherID = tmpTmpTimetable.maxTeacherID;
        LogInfo("Adding a new teacher with ID " + std::to_string(currentTeacherID));
        ResetVariables();
        isEditTeacher = true;
    }
    ImGui::Separator();

    ImGui::Columns(2);
    for (auto it = tmpTimetable.teachers.begin(); it != tmpTimetable.teachers.end();)
    {
        ImGui::PushID(it->first);
        if (ImGui::Button(gettext("-")))
        {
            LogInfo("Removed a teacher with ID " + std::to_string(it->first));
            ImGui::PopID();
            it = tmpTimetable.teachers.erase(it);
            continue;
        }
        ImGui::SameLine();

        if (ImGui::Button(gettext("Edit")))
        {
            newTeacher = false;
            tmpTmpTimetable.teachers = tmpTimetable.teachers;
            tmpTmpTimetable.maxTeacherID = tmpTimetable.maxTeacherID;
            currentTeacherID = it->first;
            LogInfo("Editing a teacher with ID " + std::to_string(currentTeacherID));
            ResetVariables();
            isEditTeacher = true;
        }
        ImGui::SameLine();

        ImGui::Text("%s", it->second.name.c_str());
        ImGui::NextColumn();

        std::string lessonNames = "";
        for (size_t j = 0; j < tmpTimetable.teachers[it->first].lessonIDs.size(); j++)
        {
            lessonNames +=
                currentTimetable.lessons[tmpTimetable.teachers[it->first].lessonIDs[j]].name;
            if (j < tmpTimetable.teachers[it->first].lessonIDs.size() - 1) lessonNames += ' ';
        }
        ImGui::Text("%s", lessonNames.c_str());
        ImGui::NextColumn();
        ImGui::PopID();
        ++it;
    }
    ImGui::Columns(1);
    ImGui::Separator();

    // Ok and Cancel
    if (ImGui::Button(gettext("Ok")))
    {
        LogInfo("Clicked Ok in the teachers menu");
        currentTimetable.teachers = tmpTimetable.teachers;
        currentTimetable.maxTeacherID = tmpTimetable.maxTeacherID;
        *isOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button(gettext("Cancel"))) *isOpen = false;
    ImGui::End();
}
