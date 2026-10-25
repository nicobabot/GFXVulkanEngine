#include "GfxMaterialLibrary.h"
#include "GfxTexture.h"
#include "GfxTextureLibrary.h"
#include "GfxGeometry.h"
#include <cmath>
#include <array>
#include <stdexcept>

GfxMaterialLibrary::~GfxMaterialLibrary() { Clear(); }

void GfxMaterialLibrary::Initialize(VkDevice requestedDevice, uint32_t requestedCapacity)
{
    if (device != VK_NULL_HANDLE || requestedDevice == VK_NULL_HANDLE || requestedCapacity == 0)
        throw std::runtime_error("Material library requires a valid device, capacity, and a single initialization");
    device = requestedDevice;
    capacity = requestedCapacity;
    try
    {
        // HLSL space1: sampler s0 and base-color image t1.
        const std::array<VkDescriptorSetLayoutBinding, 2> bindings = {{
            { 0, VK_DESCRIPTOR_TYPE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr },
            { 1, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
        }};
        VkDescriptorSetLayoutCreateInfo layout{};
        layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout.bindingCount = static_cast<uint32_t>(bindings.size());
        layout.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(device, &layout, nullptr, &descriptorSetLayout) != VK_SUCCESS)
            throw std::runtime_error("Could not create material descriptor set layout");
        const std::array<VkDescriptorPoolSize, 2> sizes = {{
            { VK_DESCRIPTOR_TYPE_SAMPLER, capacity }, { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, capacity }
        }};
        VkDescriptorPoolCreateInfo pool{};
        pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool.maxSets = capacity;
        pool.poolSizeCount = static_cast<uint32_t>(sizes.size());
        pool.pPoolSizes = sizes.data();
        if (vkCreateDescriptorPool(device, &pool, nullptr, &descriptorPool) != VK_SUCCESS)
            throw std::runtime_error("Could not create material descriptor pool");
    }
    catch (...) { Clear(); throw; }
}

const GfxMaterial* GfxMaterialLibrary::Create(const std::string& name, const GfxTexture* texture,
    const glm::vec4& baseColorFactor)
{
    for (int i = 0; i < 4; ++i)
        if (!std::isfinite(baseColorFactor[i]) || baseColorFactor[i] < 0)
            throw std::runtime_error("Material color must be finite and nonnegative: " + name);
    if (device == VK_NULL_HANDLE || texture == nullptr || name.empty())
        throw std::runtime_error("Creating a material requires an initialized library, name, and texture");
    const auto found = materials.find(name);
    if (found != materials.end())
    {
        if (found->second->baseColorTexture != texture || found->second->baseColorFactor != baseColorFactor)
            throw std::runtime_error("Material already exists with different properties: " + name);
        return found->second.get();
    }
    if (materials.size() >= capacity)
        throw std::runtime_error("Material library capacity exceeded while creating: " + name);

    auto material = std::make_unique<GfxMaterial>();
    material->name = name;
    material->baseColorTexture = texture;
    material->baseColorFactor = baseColorFactor;
    GfxMaterial* result = material.get();
    // Allocate the map node before consuming a descriptor slot. A CPU allocation
    // failure must not leave an unreachable set in the fixed-capacity pool.
    materials.emplace(name, std::move(material));
    VkDescriptorSetAllocateInfo allocation{};
    allocation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocation.descriptorPool = descriptorPool;
    allocation.descriptorSetCount = 1;
    allocation.pSetLayouts = &descriptorSetLayout;
    if (vkAllocateDescriptorSets(device, &allocation, &result->descriptorSet) != VK_SUCCESS)
    {
        materials.erase(name);
        throw std::runtime_error("Could not allocate material descriptor set: " + name);
    }
    VkDescriptorImageInfo sampler{};
    sampler.sampler = texture->GetSampler();
    VkDescriptorImageInfo image{};
    image.imageView = texture->GetImageView();
    image.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    std::array<VkWriteDescriptorSet, 2> writes{};
    for (uint32_t binding = 0; binding < writes.size(); ++binding)
    {
        writes[binding].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[binding].dstSet = result->descriptorSet;
        writes[binding].dstBinding = binding;
        writes[binding].descriptorCount = 1;
    }
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    writes[0].pImageInfo = &sampler;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    writes[1].pImageInfo = &image;
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
    return result;
}

std::vector<const GfxMaterial*> GfxMaterialLibrary::CreateObjMaterials(const GfxGeometry& geometry,
    GfxTextureLibrary& textures, const GfxMaterial* fallback)
{
    if (fallback == nullptr || geometry.sourceMaterials.empty())
        throw std::runtime_error("OBJ materials require source metadata and a fallback");
    std::vector<const GfxMaterial*> result(geometry.sourceMaterials.size(), fallback);
    // Import only referenced slots; unused MTL entries need no GPU resources.
    for (const auto& submesh : geometry.submeshes)
    {
        const uint32_t slot = submesh.materialSlot;
        if (slot >= result.size())
            throw std::runtime_error("OBJ submesh material slot out of range");
        if (slot == 0 || result[slot] != fallback)
            continue;
        const auto& source = geometry.sourceMaterials[slot];
        const GfxTexture* texture = source.baseColorTexture.empty()
            ? textures.GetWhite() : textures.Load(source.baseColorTexture);
        result[slot] = Create("obj:" + geometry.name + ":slot:" + std::to_string(slot),
            texture, glm::vec4(source.baseColor, 1.0f));
    }
    return result;
}

void GfxMaterialLibrary::Clear()
{
    materials.clear();
    if (device != VK_NULL_HANDLE)
    {
        // Destroy the pool once to release every material set.
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }
    descriptorPool = VK_NULL_HANDLE;
    descriptorSetLayout = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
    capacity = 0;
}
