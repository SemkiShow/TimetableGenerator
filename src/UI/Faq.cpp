// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "UI/Faq.hpp"
#include "Settings.hpp"
#include "Translations.hpp"
#include "UI.hpp"
#include <imgui.h>
#include <raylib.h>

std::shared_ptr<FaqMenu> faqMenu;
std::vector<Texture> faqScreenshots;

void DrawImage(const Texture& texture)
{
    ImGui::Image(static_cast<ImTextureID>(texture.id),
                 ImVec2(texture.width * 1.0f * settings.fontSize / DEFAULT_FONT_SIZE,
                        texture.height * 1.0f * settings.fontSize / DEFAULT_FONT_SIZE));
}

void FaqMenu::Draw()
{
    if (!ImGui::Begin(gettext("FAQ"), &visible))
    {
        ImGui::End();
        return;
    }
    if (ImGui::TreeNode(gettext("How do I contact the developer?")))
    {
        ImGui::Text("%s",
                    gettext("You can contact me by sending an email to mgdeveloper123@gmail.com"));
        ImGui::TreePop();
    }
    if (ImGui::TreeNode(gettext("How do I add multiple lessons to one timetable cell?")))
    {
        ImGui::Text(
            "%s",
            gettext(
                "To add multiple lessons to one timetable cell, click\nCombine lessons while editing a class."));
        DrawImage(faqScreenshots[0]);
        ImGui::Text("%s", gettext("Select the lessons and teachers to combine and press Ok."));
        DrawImage(faqScreenshots[1]);
        ImGui::Text("%s", gettext("Then set the amount per week for the created combined lesson."));
        DrawImage(faqScreenshots[2]);
        ImGui::TreePop();
    }
    ImGui::End();
}
