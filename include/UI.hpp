#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

extern bool isSettings;
extern int menuOffset;
extern int windowSize[2];

void DrawFrame();
void ShowSettings(bool* isOpen);
void ShowMenuBar();
