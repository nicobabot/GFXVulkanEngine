#include "GfxEditorLayout.h"
#include "Imgui/imgui_internal.h"

ImGuiID GfxEditorLayout::DrawDockspace(bool resetLayout)
{
    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable))
        return 0;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("Layout"))
        {
            resetLayout |= ImGui::MenuItem("Reset panels");
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImGuiID dockspace = ImHashStr("AEngineDockspace");
    const ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode
        | ImGuiDockNodeFlags_NoDockingOverCentralNode;
    // DockBuilder is internal API: keep default-layout construction isolated here.
    if (resetLayout || ImGui::DockBuilderGetNode(dockspace) == nullptr)
    {
        ImGui::DockBuilderRemoveNode(dockspace);
        ImGui::DockBuilderAddNode(dockspace, flags | ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodePos(dockspace, viewport->WorkPos);
        ImGui::DockBuilderSetNodeSize(dockspace, viewport->WorkSize);
        ImGuiID center = dockspace;
        ImGuiID left = 0, right = 0, options = 0;
        ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.18f, &left, &center);
        ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.28f, &right, &center);
        ImGui::DockBuilderSplitNode(right, ImGuiDir_Down, 0.32f, &options, &right);
        ImGui::DockBuilderDockWindow("Scene Objects", left);
        ImGui::DockBuilderDockWindow("Transform Editor", right);
        ImGui::DockBuilderDockWindow("GFX options", options);
        ImGui::DockBuilderFinish(dockspace);
    }
    // No Scene image widget yet: preserve the existing full-frame scene underneath.
    return ImGui::DockSpaceOverViewport(dockspace, viewport, flags);
}
