#pragma once
#include "GfxMaterial.h"
#include <map>
#include <memory>

class GfxMaterialLibrary
{
public:
    GfxMaterialLibrary() = default;
    ~GfxMaterialLibrary();
    GfxMaterialLibrary(const GfxMaterialLibrary&) = delete;
    GfxMaterialLibrary& operator=(const GfxMaterialLibrary&) = delete;

    // Initialize before creating pipeline layouts that reference material set 1.
    void Initialize(VkDevice device, uint32_t capacity = 64);
    const GfxMaterial* Create(const std::string& name, const GfxTexture* baseColorTexture);
    VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }
    size_t GetMaterialCount() const { return materials.size(); }
    // GPU use must be complete and scene references removed before clearing.
    void Clear();

private:
    VkDevice device = VK_NULL_HANDLE;
    VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    uint32_t capacity = 0;
    std::map<std::string, std::unique_ptr<GfxMaterial>> materials;
};
