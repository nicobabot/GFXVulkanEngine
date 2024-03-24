#include "GfxObject.h"
#include "gfxMaths.h"
#include "GfxPipelineManager.h"

void GfxObject::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer_Internal(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(gfxCtx->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(gfxCtx->logicalDevice, stagingBufferMemory);

    CreateBuffer_Internal(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    CopyBuffer_Internal(stagingBuffer, vertexBuffer, bufferSize);
    vkDestroyBuffer(gfxCtx->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, stagingBufferMemory, nullptr);
}

void GfxObject::CreateIndexBuffer()
{
}
