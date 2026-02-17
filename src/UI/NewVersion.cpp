// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/NewVersion.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "Updates.hpp"
#include <imgui.h>
#include <thread>

std::shared_ptr<NewVersionMenu> newVersionMenu;

void NewVersionMenu::Draw()
{
    if (!ImGui::Begin(gettext("Updates"), &visible))
    {
        ImGui::End();
        return;
    }
    ImGui::Text("%s",
                (std::string(gettext("The latest version is")) + " " + latestVersion).c_str());
    ImGui::Text("%s", (std::string(gettext("Your version is")) + " " + version).c_str());
    if (version == latestVersion)
        ImGui::Text("%s", gettext("There are no new versions available"));
    else if (latestVersion != gettext("loading..."))
    {
        ImGui::Text("%s", gettext("A new version is available!"));
        if (ImGui::TreeNode(gettext("Release notes")))
        {
            for (int i = 0; i < (int)releaseNotes.size() - 2; i++)
            {
                ImGui::Text("%s", releaseNotes[i].c_str());
            }
            ImGui::TreePop();
        }
        if (downloadStatus != "") ImGui::Text("%s", downloadStatus.c_str());
        if (ImGui::Button(gettext("Update")))
        {
            std::thread updateThread(UpdateToLatestVersion);
            updateThread.detach();
        }
    }
    ImGui::End();
}
