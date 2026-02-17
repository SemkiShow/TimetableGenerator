// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief Utils for classes UI
 */

#pragma once

#include "Timetable.hpp"
#include <unordered_map>

void FetchClassLessonsFromSimularClasses(Timetable& timetable, int classId);
void ChangeClassesAmount(Timetable& timetable, const std::string& classNumber, int classesAmount);
void UpdateClassLetters(Timetable& timetable);
void ShiftClasses(Timetable& timetable, const int direction);
void LoadTimetableLessonsFromSelection(Timetable& timetable, int classId,
                                       std::unordered_map<std::string, bool>& lessons,
                                       std::unordered_map<std::string, int>& lessonAmounts,
                                       std::unordered_map<std::string, bool>& lessonTeachers);
