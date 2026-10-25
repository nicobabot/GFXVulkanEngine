#pragma once
#include "gfxMaths.h"
#include <cstdint>
#include <string>
#include <vector>

// Index ranges share one vertex/index buffer. Slot 0 is the fallback material.
struct GfxSubmesh
{
    uint32_t firstIndex = 0;
    uint32_t indexCount = 0;
    uint32_t materialSlot = 0;
    std::string name;
};

// CPU import metadata, not an owning GPU material.
struct GfxSourceMaterial
{
    std::string name;
    glm::vec3 baseColor = glm::vec3(1.0f);
    std::string baseColorTexture;
};

struct GfxMeshData
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<GfxSubmesh> submeshes;
    std::vector<GfxSourceMaterial> materials;
    std::string warnings;
};
