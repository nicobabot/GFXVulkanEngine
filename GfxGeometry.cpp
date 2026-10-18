#include "GfxGeometry.h"
#include "GfxPipelineManager.h"
#include "GfxContext.h"
#include <cstring>
#include <limits>
#include <stdexcept>

GfxGeometry::GfxGeometry(const std::string& name) : name(name)
{
}

GfxGeometry::~GfxGeometry()
{
    // The owner must release geometry after GPU completion and before device destruction.
    DestroyBuffers();
}

void GfxGeometry::DestroyBuffers()
{
    if (device == VK_NULL_HANDLE)
        return;
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    vertexBuffer = VK_NULL_HANDLE;
    vertexBufferMemory = VK_NULL_HANDLE;
    indexBuffer = VK_NULL_HANDLE;
    indexBufferMemory = VK_NULL_HANDLE;
    indexCount = 0;
    device = VK_NULL_HANDLE;
}

static void UploadBuffer(const void* source, VkDeviceSize size, VkBufferUsageFlags usage,
    VkBuffer& buffer, VkDeviceMemory& memory, const std::string& name)
{
    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
    try
    {
        CreateBuffer_Internal(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingMemory);
        void* mapped = nullptr;
        if (vkMapMemory(gfxCtx->logicalDevice, stagingMemory, 0, size, 0, &mapped) != VK_SUCCESS)
            throw std::runtime_error("Could not map geometry staging memory: " + name);
        std::memcpy(mapped, source, static_cast<size_t>(size));
        vkUnmapMemory(gfxCtx->logicalDevice, stagingMemory);

        const std::string memoryName = name + "Memory";
        CreateBuffer_Internal(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, memory, name.c_str(), memoryName.c_str());
        // Existing synchronous copy helper waits for graphics completion before returning.
        CopyBuffer_Internal(stagingBuffer, buffer, size);
    }
    catch (...)
    {
        vkDestroyBuffer(gfxCtx->logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(gfxCtx->logicalDevice, stagingMemory, nullptr);
        throw;
    }
    vkDestroyBuffer(gfxCtx->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, stagingMemory, nullptr);
}

void GfxGeometry::Upload()
{
    if (device != VK_NULL_HANDLE)
        throw std::runtime_error("Geometry is already uploaded: " + name);
    if (vertices.empty() || indices.empty() || indices.size() > (std::numeric_limits<uint32_t>::max)())
        throw std::runtime_error("Geometry must have vertices and a valid index count: " + name);
    for (uint32_t index : indices)
        if (index >= vertices.size())
            throw std::runtime_error("Geometry index is out of range: " + name);

    device = gfxCtx->logicalDevice;
    try
    {
        UploadBuffer(vertices.data(), sizeof(Vertex) * static_cast<VkDeviceSize>(vertices.size()),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertexBuffer, vertexBufferMemory, name + "VertexBuffer");
        UploadBuffer(indices.data(), sizeof(uint32_t) * static_cast<VkDeviceSize>(indices.size()),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indexBuffer, indexBufferMemory, name + "IndexBuffer");
        indexCount = static_cast<uint32_t>(indices.size());
    }
    catch (...)
    {
        DestroyBuffers();
        throw;
    }
}
