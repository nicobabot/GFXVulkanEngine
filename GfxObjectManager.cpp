#include "GfxObjectManager.h"
#include "Imgui/imgui.h"
#include "GfxObject.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

void GfxObjectManager::DrawImgui()
{
	ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Scene Objects"))
	{
		for (size_t i = 0; i < objects.size(); i++)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow
				| ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_OpenOnDoubleClick;

			// Use pointer as unique ID to avoid name collisions
			bool opened = ImGui::TreeNodeEx((void*)objects[i], flags, "%s", objects[i]->name);

			if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
				selectedObject = objects[i];

			if (opened)
			{
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	// Draw Transform Editor Window
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Transform Editor"))
	{
		if (selectedObject != nullptr)
		{
			ImGui::Text("Selected Object: %s", selectedObject->name);
			ImGui::Separator();

			// Position
			glm::vec3 position = selectedObject->transform.position;
			float posArray[3] = { position.x, position.y, position.z };
			float transSpeed = 0.25f;
			if (ImGui::DragFloat3("Position##Transform", posArray, transSpeed, -500.0f, 500.0f))
			{
				selectedObject->transform.position = glm::vec3(posArray[0], posArray[1], posArray[2]);
			}

			// Rotation (Euler angles in degrees)
			glm::vec3 eulerAngles = glm::degrees(selectedObject->transform.GetEulerAngles());
			float rotArray[3] = { eulerAngles.x, eulerAngles.y, eulerAngles.z };
			float rootSpeed = 0.25f;
			if (ImGui::DragFloat3("Rotation (deg)##Transform", rotArray, rootSpeed, -180.0f, 180.0f))
			{
				selectedObject->transform.SetEulerAngles(glm::radians(glm::vec3(rotArray[0], rotArray[1], rotArray[2])));
			}

			// Scale
			glm::vec3 scale = selectedObject->transform.scale;
			float scaleArray[3] = { scale.x, scale.y, scale.z };
			float scaleSpeed = 0.25f;
			if (ImGui::DragFloat3("Scale##Transform", scaleArray, scaleSpeed, -500.0f, 500.0f))
			{
				selectedObject->transform.scale = glm::vec3(scaleArray[0], scaleArray[1], scaleArray[2]);
			}

			// Reset button
			ImGui::Separator();
			if (ImGui::Button("Reset Transform##ResetBtn", ImVec2(-1, 0)))
			{
				selectedObject->transform.position = glm::vec3(0.0f);
				selectedObject->transform.rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
				selectedObject->transform.scale = glm::vec3(1.0f);
			}
		}
		else
		{
			ImGui::Text("No object selected");
			ImGui::Text("Select an object from the Scene Objects window");
		}
		ImGui::End();
	}
}

