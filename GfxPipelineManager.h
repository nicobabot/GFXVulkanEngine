#include <vulkan/vulkan_core.h>
#include <vector>
class GfxContext;

struct GraphicsPipelineInfo 
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkDescriptorSetLayout descriptorSetLayout;
	VkExtent2D viewportExtent;
	VkSampleCountFlagBits msaaSamples;
	VkRenderPass renderPass;

	GraphicsPipelineInfo() = default;
};

VkCommandBuffer BeginSingleTimeCommandBuffer_Internal();

void EndSingleTimeCommandBuffer_Internal(VkCommandBuffer commandBuffer);

uint32_t FindMemoryType_Internal(uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags);

void CreateGraphicsPipeline_Internal(const GraphicsPipelineInfo& graphicPipelineInfo,
	VkPipelineLayout& graphicPipelineLayout, VkPipeline& graphicPipeline);

void CreateBuffer_Internal(VkDeviceSize size, VkBufferUsageFlags usageFlags,
	VkMemoryPropertyFlags memoryFlags, VkBuffer& newBuffer, VkDeviceMemory& bufferMemory);

void CopyBuffer_Internal(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

extern GfxContext* gfxCtx;