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

bool HasStencilComponent(VkFormat format);

void TransitionImageLayout(VkImage image, VkFormat format,
	VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels, bool transitionToCompute = false, VkCommandBuffer commandBuffer = nullptr);

void CreateGraphicsPipeline_Internal(const GraphicsPipelineInfo& graphicPipelineInfo,
	VkPipelineLayout& graphicPipelineLayout, VkPipeline& graphicPipeline);

void CreateBuffer_Internal(VkDeviceSize size, VkBufferUsageFlags usageFlags,
	VkMemoryPropertyFlags memoryFlags, VkBuffer& newBuffer, VkDeviceMemory& bufferMemory);

void CopyBuffer_Internal(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void CopyImage_Internal(VkDevice device, VkImage srcImage, VkFormat srcFormat, int srcMipCount, VkImage dstImage, VkFormat dstFormat, int dstMipCount, uint32_t width, uint32_t height);

extern GfxContext* gfxCtx;