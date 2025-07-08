#pragma once

#include <string>

#define WIZARD_STEPS 6

extern int menuOffset;
extern int windowSize[2];
extern std::string weekDays[7];
extern std::string wizardTexts[WIZARD_STEPS];

extern std::string teacherLessonValues;
extern std::string classTeacherValues;
extern bool wasGenerateTimetable;
extern std::string generateTimetableStatus;
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
void ShowEditClass(bool* isOpen);
void ShowClasses(bool* isOpen);
void ShowWizard(bool* isOpen);

void ResetTeacherLessonValues();

void LoadFonts();
void LoadStyle();
void LoadFAQScreenshots();
void DrawFrame();
