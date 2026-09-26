#include "Style.h"
#include <imgui.h>

void SetupEditorStyle() {
    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    colors[ImGuiCol_WindowBg]          = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_TitleBg]           = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBgActive]     = ImVec4(0.16f, 0.16f, 0.20f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]  = ImVec4(0.08f, 0.08f, 0.10f, 0.60f);
    colors[ImGuiCol_MenuBarBg]         = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_Header]            = ImVec4(0.25f, 0.40f, 0.70f, 0.50f);
    colors[ImGuiCol_HeaderHovered]     = ImVec4(0.30f, 0.45f, 0.75f, 0.70f);
    colors[ImGuiCol_HeaderActive]      = ImVec4(0.35f, 0.50f, 0.80f, 0.90f);
    colors[ImGuiCol_Button]            = ImVec4(0.20f, 0.20f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered]     = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive]      = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);
    colors[ImGuiCol_FrameBg]           = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]    = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_FrameBgActive]     = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    colors[ImGuiCol_Tab]               = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TabHovered]        = ImVec4(0.25f, 0.40f, 0.70f, 0.60f);
    colors[ImGuiCol_TabActive]         = ImVec4(0.25f, 0.40f, 0.70f, 0.80f);
    colors[ImGuiCol_TabUnfocused]      = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]= ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
    colors[ImGuiCol_CheckMark]         = ImVec4(0.40f, 0.60f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrab]        = ImVec4(0.40f, 0.60f, 0.90f, 0.70f);
    colors[ImGuiCol_SliderGrabActive]  = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]       = ImVec4(0.06f, 0.06f, 0.08f, 0.90f);
    colors[ImGuiCol_ScrollbarGrab]     = ImVec4(0.25f, 0.25f, 0.30f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.35f, 0.35f, 0.40f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.45f, 0.45f, 0.50f, 0.80f);
    colors[ImGuiCol_DockingPreview]    = ImVec4(0.30f, 0.45f, 0.75f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg]    = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
    colors[ImGuiCol_Text]              = ImVec4(0.85f, 0.85f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]      = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);
    colors[ImGuiCol_Separator]         = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_ResizeGrip]        = ImVec4(0.25f, 0.40f, 0.70f, 0.30f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.30f, 0.45f, 0.75f, 0.50f);
    colors[ImGuiCol_ResizeGripActive]  = ImVec4(0.35f, 0.50f, 0.80f, 0.70f);
    colors[ImGuiCol_PlotLines]         = ImVec4(0.40f, 0.60f, 0.90f, 1.00f);

    style.WindowRounding    = 4.0f;
    style.FrameRounding     = 3.0f;
    style.GrabRounding      = 3.0f;
    style.ScrollbarRounding = 3.0f;
    style.TabRounding       = 3.0f;
    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);
    style.WindowMenuButtonPosition = ImGuiDir_None;
    style.WindowPadding     = ImVec2(6, 6);
    style.FramePadding      = ImVec2(4, 3);
    style.ItemSpacing       = ImVec2(6, 4);
    style.ItemInnerSpacing  = ImVec2(4, 4);
    style.IndentSpacing     = 16.0f;
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 8.0f;
}
