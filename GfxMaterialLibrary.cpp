#include "GfxMaterialLibrary.h"
#include "GfxTexture.h"
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

const GfxMaterial* GfxMaterialLibrary::Create(const std::string& name, const GfxTexture* texture)
{
    if (device == VK_NULL_HANDLE || texture == nullptr || name.empty())
        throw std::runtime_error("Creating a material requires an initialized library, name, and texture");
    const auto found = materials.find(name);
    if (found != materials.end())
    {
        if (found->second->baseColorTexture != texture)
            throw std::runtime_error("Material already exists with a different texture: " + name);
        return found->second.get();
    }
    if (materials.size() >= capacity)
        throw std::runtime_error("Material library capacity exceeded while creating: " + name);

    auto material = std::make_unique<GfxMaterial>();
    material->name = name;
    material->baseColorTexture = texture;
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
