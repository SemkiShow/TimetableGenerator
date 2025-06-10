#include "raylib.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "rlImGui.h"
#include "ImGuiFileDialog.h"
#include <unordered_map>

extern int menuOffset;
extern int windowSize[2];
extern std::unordered_map<int, std::string> weekDays;

extern std::string classrooms;
extern std::string classTeacherValues;
extern bool isClassrooms;
extern bool isEditLesson;
extern bool isLessons;
extern bool isEditTeacher;
extern bool isTeachers;
// extern bool isCombineLessons;
// extern bool isEditClass;
// extern bool isClasses;

void ShowClassrooms(bool* isOpen);
void ShowEditLesson(bool* isOpen);
void ShowLessons(bool* isOpen);
void ShowEditTeacher(bool* isOpen);
void ShowTeachers(bool* isOpen);
// void ShowCombineLessons(bool* isOpen);
// void ShowEditClass(bool* isOpen);
// void ShowClasses(bool* isOpen);

void LoadFonts();
void DrawFrame();
