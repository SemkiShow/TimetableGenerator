#include "raylib.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "rlImGui.h"
#include "ImGuiFileDialog.h"

extern bool isSettings;
extern int menuOffset;
extern int windowSize[2];

extern std::string classrooms;

void LoadFonts();
void DrawFrame();
