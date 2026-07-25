// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/NewVersion.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "Updates.hpp"
#include <imgui.h>
#include <memory>
#include <string>
#include <thread>

std::shared_ptr<NewVersionMenu> g_newVersionMenu;

void NewVersionMenu::Draw()
{
    if (!ImGui::Begin(gettext("Updates"), &visible_))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("The latest version is %s", g_latestVersion.c_str());
    ImGui::Text("Your version is %s", g_version.c_str());
    if (g_version == g_latestVersion)
        ImGui::Text("%s", gettext("There are no new versions available"));
    else if (g_latestVersion != gettext("loading..."))
    {
        ImGui::Text("%s", gettext("A new version is available!"));
        if (ImGui::TreeNode(gettext("Release notes")))
        {
            for (int i = 0; i < (int)g_releaseNotes.size() - 2; i++)
            {
                ImGui::Text("%s", g_releaseNotes[i].c_str());
            }
            ImGui::TreePop();
        }
        if (!g_downloadStatus.empty()) ImGui::Text("%s", g_downloadStatus.c_str());
        if (ImGui::Button(gettext("Update")))
        {
            std::thread updateThread(UpdateToLatestVersion);
            updateThread.detach();
        }
    }
    ImGui::End();
}
