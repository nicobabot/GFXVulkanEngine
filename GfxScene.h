#pragma once
#include <memory>
#include <vector>
#include "GfxObject.h"

class GfxGeometry;

// Owns scene instances and the editor selection. Geometry remains owned by the library.
class GfxScene
{
public:
    GfxObject& CreateObject(const char* name, const GfxGeometry* geometry,
        const GfxMaterial* material, VkPipeline pipeline, VkPipelineLayout pipelineLayout);
    void Clear();
    void DrawImgui(const glm::mat4& view, const glm::mat4& vulkanProjection);

    std::vector<std::unique_ptr<GfxObject>>& GetObjects() { return objects; }
    const std::vector<std::unique_ptr<GfxObject>>& GetObjects() const { return objects; }

private:
    std::vector<std::unique_ptr<GfxObject>> objects;
    GfxObject* selectedObject = nullptr;
    bool gizmoEnabled = true;
    int gizmoOperation = 0; // Translate, rotate, scale (editor UI order).
    bool gizmoLocal = true;
    bool gizmoSnap = false;
    glm::vec3 translationSnap = glm::vec3(1.0f);
    float rotationSnap = 15.0f;
    float scaleSnap = 0.1f;
    void DrawGizmo(const glm::mat4& view, const glm::mat4& vulkanProjection);
};
