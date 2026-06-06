#include "GfxObjectManager.h"
#include "Imgui/imgui.h"
#include "GfxObject.h"

void GfxObjectManager::DrawImgui()
{

	for (size_t i = 0; i < objects.size(); i++)
	{
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        /*if (node->children.empty())
            flags |= ImGuiTreeNodeFlags_Leaf;

        if (node == m_selectedNode)
            flags |= ImGuiTreeNodeFlags_Selected;*/

        // Use pointer as unique ID to avoid name collisions
        bool opened = ImGui::TreeNodeEx((void*)objects[i], flags, "%s", objects[i]->name);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            selectedObject = objects[i];

        // Right-click context menu per node
        //if (ImGui::BeginPopupContextItem())
        //{
        //    if (ImGui::MenuItem("Add Child")) { /* add child node */ }
        //    if (ImGui::MenuItem("Delete")) { /* delete node */ }
        //    ImGui::EndPopup();
        //}

        if (opened)
        {
            /*for (SceneNode* child : node->children)
                drawNode(child);*/

            ImGui::TreePop();
        }
	}


}
