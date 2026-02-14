// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Timetable.hpp"
#include <string>
#include <unordered_map>
#include <vector>

extern int currentClassId;
extern bool newClass;
extern int classTeacherIndex;
extern std::unordered_map<std::string, bool> classLessons;
extern std::unordered_map<std::string, int> classLessonAmounts;
extern std::unordered_map<std::string, bool> classLessonTeachers;
extern std::string classTeacherValues;
extern std::vector<int> classTeacherIds;
extern std::map<int, Lesson> tmpLessons;
extern std::map<int, Lesson> tmpTmpLessons;

extern int currentLessonId;
extern bool newCombinedLesson;
extern bool newRule;
extern size_t currentRuleId;

void ResetClassTeacherValues();
bool CompareTimetableLessons(const TimetableLesson lesson1, const TimetableLesson lesson2);
void FetchClassLessonsFromSimularClasses(Timetable& timetable, int classId);
void ChangeClassesAmount(Timetable& timetable, const std::string& classNumber,
                         const int classesAmount);
void UpdateClassLetters(Timetable& timetable);
int GetClassesAmount(Timetable& timetable, const std::string& classNumber);
void ShiftClasses(Timetable& timetable, const int direction);
void LoadTimetableLessonsFromSelection();

void ResetCombineLessonsVariables();
void ResetRulesVariables();
