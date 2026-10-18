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
        VkPipeline pipeline, VkPipelineLayout pipelineLayout);
    void Clear();
    void DrawImgui();

    std::vector<std::unique_ptr<GfxObject>>& GetObjects() { return objects; }
    const std::vector<std::unique_ptr<GfxObject>>& GetObjects() const { return objects; }

private:
    std::vector<std::unique_ptr<GfxObject>> objects;
    GfxObject* selectedObject = nullptr;
};
