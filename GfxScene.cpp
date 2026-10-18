#include "GfxScene.h"
#include "GfxGeometry.h"
#include "GfxMaterial.h"
#include "GfxTexture.h"
#include <stdexcept>
#include "Imgui/imgui.h"
#include <glm/gtc/quaternion.hpp>

GfxObject& GfxScene::CreateObject(const char* name, const GfxGeometry* geometry,
    const GfxMaterial* material, VkPipeline pipeline, VkPipelineLayout pipelineLayout)
{
    if (geometry == nullptr || material == nullptr)
        throw std::runtime_error("Scene objects require geometry and a material");
    auto object = std::make_unique<GfxObject>(pipeline, pipelineLayout, name);
    object->geometry = geometry;
    object->material = material;
    GfxObject& result = *object;
    objects.push_back(std::move(object));
    return result;
}

void GfxScene::Clear()
{
    selectedObject = nullptr;
    objects.clear();
}

void GfxScene::DrawImgui()
{
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Scene Objects"))
    {
        for (const auto& object : objects)
        {
            const GfxObject* sceneObject = object.get();
            const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
                | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            const bool opened = ImGui::TreeNodeEx(sceneObject, flags, "%s", sceneObject->name);

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
                selectedObject = object.get();
            if (opened)
                ImGui::TreePop();
        }
        ImGui::End();
    }

    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Transform Editor"))
    {
        if (selectedObject == nullptr)
        {
            ImGui::Text("No object selected");
            ImGui::Text("Select an object from the Scene Objects window");
        }
        else
        {
            ImGui::Text("Selected Object: %s", selectedObject->name);
            if (selectedObject->material != nullptr)
            {
                const GfxMaterial* material = selectedObject->material;
                ImGui::Text("Material: %s", material->GetName().c_str());
                ImGui::Text("Material address: %p", static_cast<const void*>(material));
                ImGui::TextWrapped("Base color: %s", material->GetBaseColorTexture()->GetPath().c_str());
            }
            if (selectedObject->geometry != nullptr)
            {
                const GfxGeometry* geometry = selectedObject->geometry;
                int instanceCount = 0;
                for (const auto& object : objects)
                    if (object->geometry == geometry)
                        ++instanceCount;
                ImGui::Text("Geometry: %s", geometry->name.c_str());
                ImGui::Text("Geometry address: %p", static_cast<const void*>(geometry));
                ImGui::Text("Indices: %u", geometry->indexCount);
                ImGui::Text("Instances using geometry: %d", instanceCount);
            }
            ImGui::Separator();

            glm::vec3 position = selectedObject->transform.position;
            float posArray[3] = { position.x, position.y, position.z };
            if (ImGui::DragFloat3("Position##Transform", posArray, 0.25f, -500.0f, 500.0f))
                selectedObject->transform.position = glm::vec3(posArray[0], posArray[1], posArray[2]);

            const glm::vec3 eulerAngles = glm::degrees(selectedObject->transform.GetEulerAngles());
            float rotArray[3] = { eulerAngles.x, eulerAngles.y, eulerAngles.z };
            if (ImGui::DragFloat3("Rotation (deg)##Transform", rotArray, 0.25f, -180.0f, 180.0f))
                selectedObject->transform.SetEulerAngles(glm::radians(glm::vec3(rotArray[0], rotArray[1], rotArray[2])));

            glm::vec3 scale = selectedObject->transform.scale;
            float scaleArray[3] = { scale.x, scale.y, scale.z };
            if (ImGui::DragFloat3("Scale##Transform", scaleArray, 0.25f, -500.0f, 500.0f))
                selectedObject->transform.scale = glm::vec3(scaleArray[0], scaleArray[1], scaleArray[2]);

            ImGui::Separator();
            if (ImGui::Button("Reset Transform##ResetBtn", ImVec2(-1, 0)))
            {
                selectedObject->transform.position = glm::vec3(0.0f);
                selectedObject->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
                selectedObject->transform.scale = glm::vec3(1.0f);
            }
        }
        ImGui::End();
    }
}
