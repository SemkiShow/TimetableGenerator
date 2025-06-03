#include "UI.hpp"
#include "Settings.hpp"

bool isSettings = false;
int menuOffset = 20;
int windowSize[2] = {16*50*2, 9*50*2};
bool lastVsync = vsync;

void DrawFrame()
{
    BeginDrawing();

    rlImGuiBegin();

    ClearBackground(BLACK);

    ShowMenuBar();
    if (isSettings) ShowSettings(&isSettings);

    if (lastVsync != vsync)
    {
        lastVsync = vsync;
        if (!vsync) ClearWindowState(FLAG_VSYNC_HINT);
        else SetWindowState(FLAG_VSYNC_HINT);
    }

    rlImGuiEnd();
    
    EndDrawing();
}

void ShowSettings(bool* isOpen)
{
    if (!ImGui::Begin("Settings", isOpen))
    {
        ImGui::End();
        return;
    }
    ImGui::Checkbox("vsync", &vsync);
    ImGui::End();
}

void ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Menu"))
        {
            if (ImGui::MenuItem("Settings"))
            {
                isSettings = true;
                ShowSettings(&isSettings);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    return;
}
