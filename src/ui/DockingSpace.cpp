#include "DockingSpace.h"
#include "Panel.h"
#include <imgui.h>
#include <imgui_internal.h>

void DockingSpace::addPanel(std::unique_ptr<Panel> panel) {
    m_panels.push_back(std::move(panel));
}

Panel* DockingSpace::panel(u32 index) const {
    return index < m_panels.size() ? m_panels[index].get() : nullptr;
}

void DockingSpace::removePanel(u32 index) {
    if (index < m_panels.size())
        m_panels.erase(m_panels.begin() + index);
}

void DockingSpace::render() {
    ImGuiID dockspaceId = ImGui::GetID(m_dockspaceName.c_str());
    ImGui::DockSpaceOverViewport(dockspaceId, ImGui::GetMainViewport(),
        ImGuiDockNodeFlags_PassthruCentralNode);

    static bool firstFrame = true;
    if (firstFrame) {
        firstFrame = false;
        ImGui::DockBuilderRemoveNode(dockspaceId);
        ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceId, ImGui::GetMainViewport()->Size);

        ImGuiID left, right, center, leftBottom;
        ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Left, 0.20f, &left, &center);
        ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.25f, &right, &center);
        ImGui::DockBuilderSplitNode(left, ImGuiDir_Down, 0.60f, &leftBottom, &left);

        ImGui::DockBuilderDockWindow("Canvas", center);
        ImGui::DockBuilderDockWindow("Tools", left);
        ImGui::DockBuilderDockWindow("Layers", leftBottom);
        ImGui::DockBuilderDockWindow("Timeline", center);
        ImGui::DockBuilderDockWindow("Properties", right);
        ImGui::DockBuilderDockWindow("Color", right);
        ImGui::DockBuilderDockWindow("History", right);
        ImGui::DockBuilderFinish(dockspaceId);
    }

    for (auto& panel : m_panels)
        panel->render();
}
