#include "GfxObject.h"
#include "gfxMaths.h"
#include "GfxPipelineManager.h"
#include "GfxContext.h"


GfxObject::GfxObject(VkPipeline graphicsPipeline, VkPipelineLayout graphicsPipelineLayout, const char* Name)
    :graphicsPipeline(graphicsPipeline), graphicsPipelineLayout(graphicsPipelineLayout), name(Name)
{
}

void GfxObject::SetDescriptorSetAndLayout(std::vector<VkDescriptorSet> descriptorSet,
VkDescriptorSetLayout descriptorSetLayout)
{
    this->descriptorSet = descriptorSet;
    this->descriptorSetLayout = descriptorSetLayout;
}

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

    std::string debugName = std::string(name) + "VertexBuffer";
    std::string debugNameMemory = std::string(name) + "VertexBufferMemory";
    CreateBuffer_Internal(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory, debugName.c_str(), debugNameMemory.c_str());

    CopyBuffer_Internal(stagingBuffer, vertexBuffer, bufferSize);
    vkDestroyBuffer(gfxCtx->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, stagingBufferMemory, nullptr);
}

void GfxObject::CreateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer_Internal(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(gfxCtx->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), bufferSize);
    vkUnmapMemory(gfxCtx->logicalDevice, stagingBufferMemory);

    std::string debugName = std::string(name) + "IndexBuffer";
    std::string debugNameMemory = std::string(name) + "IndexBufferMemory";
    CreateBuffer_Internal(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory, debugName.c_str(), debugNameMemory.c_str());

    CopyBuffer_Internal(stagingBuffer, indexBuffer, bufferSize);
    vkDestroyBuffer(gfxCtx->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, stagingBufferMemory, nullptr);
}
