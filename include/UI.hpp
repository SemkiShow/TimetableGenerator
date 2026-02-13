// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Timetable.hpp"
#include <map>
#include <string>

enum class Style
{
    Dark,
    Light,
    Classic
};

#define DEFAULT_FONT_SIZE 16

#define WIZARD_STEPS 6

extern int menuOffset;
extern int windowSize[2];
extern std::string weekDays[7];
extern std::string wizardTexts[WIZARD_STEPS];
extern std::string styleValues;

extern bool wasGenerateTimetable;
extern std::string generateTimetableStatus;
extern std::map<int, Lesson> tmpLessons;
extern bool isEditClassroom;
extern bool isClassrooms;
extern bool isEditLesson;
extern bool isLessons;
extern bool isEditTeacher;
extern bool isTeachers;
extern bool isCombineLessons;
extern bool isRules;
extern bool isEditClass;
extern bool isClasses;
extern bool isAbout;
extern bool isWizard;
extern bool isNewVersion;
extern bool isGenerateTimetable;

void OpenClassrooms();
void OpenLessons();
void OpenTeachers();
void OpenClasses();

void ShowEditClassroom(bool* isOpen);
void ShowClassrooms(bool* isOpen);
void ShowEditLesson(bool* isOpen);
void ShowLessons(bool* isOpen);
void ShowEditTeacher(bool* isOpen);
void ShowTeachers(bool* isOpen);
void ShowCombineLessons(bool* isOpen);
void ShowRules(bool* isOpen);
void ShowEditClass(bool* isOpen);
void ShowClasses(bool* isOpen);
void ShowWizard(bool* isOpen);

void LoadFonts();
void LoadStyle();
void LoadFAQScreenshots();
void DrawFrame();
