#pragma once

#include "raylib.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "rlImGui.h"
#include <unordered_map>
#include <algorithm>

extern int menuOffset;
extern int windowSize[2];
extern std::unordered_map<int, std::string> weekDays;

extern std::string teacherLessonValues;
extern std::string classTeacherValues;
extern bool isEditClassroom;
extern bool isClassrooms;
extern bool isEditLesson;
extern bool isLessons;
extern bool isEditTeacher;
extern bool isTeachers;
extern bool isCombineLessons;
extern bool isEditClass;
extern bool isClasses;
extern bool isAbout;
extern bool isWizard;
extern bool isNewVersion;

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
void ShowEditClass(bool* isOpen);
void ShowClasses(bool* isOpen);
void ShowWizard(bool* isOpen);

void LoadFonts();
void DrawFrame();
