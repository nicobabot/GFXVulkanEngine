#pragma once
#include <string>
#include <vulkan/vulkan_core.h>

class GfxTexture;
class GfxMaterialLibrary;

// Immutable surface resources. The texture library owns the referenced texture.
class GfxMaterial
{
public:
    const std::string& GetName() const { return name; }
    const GfxTexture* GetBaseColorTexture() const { return baseColorTexture; }
    VkDescriptorSet GetDescriptorSet() const { return descriptorSet; }

private:
    friend class GfxMaterialLibrary;
    std::string name;
    const GfxTexture* baseColorTexture = nullptr;
    // The library descriptor pool owns this set, not the scene object.
    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
};
