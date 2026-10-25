#include "GfxScene.h"
#include "GfxGeometry.h"
#include "GfxMaterial.h"
#include "GfxTexture.h"
#include <stdexcept>
#include <cmath>
#include "Imgui/imgui.h"
#include "Imgui/ImGuizmo/ImGuizmo.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

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

void GfxScene::DrawImgui(const glm::mat4& view, const glm::mat4& vulkanProjection)
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
    }
    ImGui::End();

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
            ImGui::Checkbox("Show gizmo", &gizmoEnabled);
            if (!ImGuizmo::IsUsingAny())
            {
                const auto& io = ImGui::GetIO();
                // Do not steal text input or the camera's RMB navigation keys.
                if (!io.WantCaptureKeyboard && !io.MouseDown[1])
                {
                    if (ImGui::IsKeyPressed(ImGuiKey_T, false)) gizmoOperation = 0;
                    if (ImGui::IsKeyPressed(ImGuiKey_R, false)) gizmoOperation = 1;
                    if (ImGui::IsKeyPressed(ImGuiKey_E, false)) gizmoOperation = 2;
                }
            }
            ImGui::BeginDisabled(ImGuizmo::IsUsingAny());
            ImGui::RadioButton("Translate (T)", &gizmoOperation, 0);
            ImGui::SameLine();
            ImGui::RadioButton("Rotate (E)", &gizmoOperation, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Scale (R)", &gizmoOperation, 2);
            if (gizmoOperation != 2)
            {
                if (ImGui::RadioButton("Local", gizmoLocal)) gizmoLocal = true;
                ImGui::SameLine();
                if (ImGui::RadioButton("World", !gizmoLocal)) gizmoLocal = false;
            }
            else
                ImGui::TextUnformatted("Scale uses local axes (no shear).");
            ImGui::Checkbox("Snap", &gizmoSnap);
            if (gizmoSnap)
            {
                if (gizmoOperation == 0)
                    ImGui::DragFloat3("Translation step", glm::value_ptr(translationSnap), 0.1f,
                        0.001f, 1000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                else if (gizmoOperation == 1)
                    ImGui::DragFloat("Angle step (deg)", &rotationSnap, 1.0f,
                        0.1f, 180.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
                else
                    ImGui::DragFloat("Scale step", &scaleSnap, 0.01f,
                        0.001f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            }
            ImGui::EndDisabled();
            ImGui::TextUnformatted("RMB: camera look | WASD: move | Select in Scene Objects");
            ImGui::Separator();
            if (selectedObject->material != nullptr)
            {
                const GfxMaterial* material = selectedObject->material;
                ImGui::Text("Fallback material: %s", material->GetName().c_str());
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
                ImGui::Text("Submeshes: %zu", geometry->submeshes.size());
                for (size_t i = 0; i < geometry->submeshes.size(); ++i)
                {
                    const auto& submesh = geometry->submeshes[i];
                    const GfxMaterial* material = selectedObject->GetMaterial(submesh.materialSlot);
                    ImGui::Text("[%zu] %s: first=%u count=%u slot=%u", i, submesh.name.c_str(),
                        submesh.firstIndex, submesh.indexCount, submesh.materialSlot);
                    ImGui::TextWrapped("  %s | %s", material->GetName().c_str(),
                        material->GetBaseColorTexture()->GetPath().c_str());
                    const auto& color = material->GetBaseColorFactor();
                    ImGui::Text("  Kd: %.3f %.3f %.3f", color.r, color.g, color.b);
                }
                ImGui::Text("Local bounds min: %.3f, %.3f, %.3f",
                    geometry->boundsMin.x, geometry->boundsMin.y, geometry->boundsMin.z);
                ImGui::Text("Local bounds max: %.3f, %.3f, %.3f",
                    geometry->boundsMax.x, geometry->boundsMax.y, geometry->boundsMax.z);
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
    }
    ImGui::End();
    DrawGizmo(view, vulkanProjection);
}

void GfxScene::DrawGizmo(const glm::mat4& view, const glm::mat4& vulkanProjection)
{
    const ImGuiIO& io = ImGui::GetIO();
    if (!gizmoEnabled || selectedObject == nullptr || io.DisplaySize.x <= 0 || io.DisplaySize.y <= 0)
        return;
    const auto& transform = selectedObject->transform;
    if (std::abs(transform.scale.x) < 1e-5f || std::abs(transform.scale.y) < 1e-5f
        || std::abs(transform.scale.z) < 1e-5f)
        return; // ImGuizmo needs an invertible matrix; numeric controls remain usable.

    // Vulkan uses positive viewport height and a clip-Y flip in projection.
    // ImGuizmo already flips Y when mapping NDC to ImGui screen coordinates.
    glm::mat4 projection = vulkanProjection;
    for (int column = 0; column < 4; ++column)
        projection[column][1] = -projection[column][1];
    glm::mat4 model = transform.GetModelMatrix();
    const ImGuizmo::OPERATION operations[] = { ImGuizmo::TRANSLATE, ImGuizmo::ROTATE, ImGuizmo::SCALE };
    const float* snap = gizmoOperation == 0 ? glm::value_ptr(translationSnap)
        : gizmoOperation == 1 ? &rotationSnap : &scaleSnap;
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    // The scene occupies the full client area. Draw beneath editor windows.
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::Enable(!io.MouseDown[1] && (!ImGui::IsAnyItemActive() || ImGuizmo::IsUsingAny()));
    ImGuizmo::PushID(selectedObject);
    if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), operations[gizmoOperation],
        gizmoOperation == 2 || gizmoLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
        glm::value_ptr(model), nullptr, gizmoSnap ? snap : nullptr))
        selectedObject->transform.TrySetModelMatrix(model);
    ImGuizmo::PopID();
}
