#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "gfxMaths.h"
#include "GfxMeshData.h"

class GfxGeometry
{
public:
    explicit GfxGeometry(const std::string& name);
    ~GfxGeometry();
    // Copying owning Vulkan handles would cause double destruction.
    GfxGeometry(const GfxGeometry&) = delete;
    GfxGeometry& operator=(const GfxGeometry&) = delete;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<GfxSubmesh> submeshes;
    std::vector<GfxSourceMaterial> sourceMaterials;
    std::string name;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;
    uint32_t indexCount = 0;
    // Local-space bounds, computed during upload for every mesh type.
    glm::vec3 boundsMin = glm::vec3(0.0f);
    glm::vec3 boundsMax = glm::vec3(0.0f);

    // One-time upload; CPU data can be discarded later without changing draw counts.
    void Upload();

private:
    VkDevice device = VK_NULL_HANDLE;
    void DestroyBuffers();
};

