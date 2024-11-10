#include <vulkan/vulkan_core.h>
#include <vector>
//#include "DebugUtils.h"
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
	VkPipelineLayout& graphicPipelineLayout, VkPipeline& graphicPipeline, bool enableBlending = false, const char* VkPipelineName = "Unknown", const char* VkPipelineLayoutName = "Unknown");

void CreateBuffer_Internal(VkDeviceSize size, VkBufferUsageFlags usageFlags,
	VkMemoryPropertyFlags memoryFlags, VkBuffer& newBuffer, VkDeviceMemory& bufferMemory, const char* BufferName = "Unknown", const char* BufferMemoryName = "Unknown");

void CopyBuffer_Internal(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void CopyImage_Internal(VkDevice device, VkImage srcImage, VkFormat srcFormat, int srcMipCount, VkImage dstImage, VkFormat dstFormat, int dstMipCount, uint32_t width, uint32_t height);

void CreateDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo descriptorCreateInfo, VkDescriptorSetLayout &descriptorSetLayout, const char* Name = "Unknown");

void AllocateDescriptorSets(VkDescriptorSetAllocateInfo descriptorSetInfo, std::vector<VkDescriptorSet> &descriptorSets, const char* Name = "Unknown");

void CreateDescriptorPool(VkDescriptorPoolCreateInfo descriptorPoolCreateInfo, VkDescriptorPool &descriptorPool, const char* Name = "Unknown");

void CreateFrameBuffer(VkFramebufferCreateInfo frameBufferCreateInfo, VkFramebuffer& frameBuffer, const char* Name = "Unknown");

void CreateRenderPass(VkRenderPassCreateInfo renderpassCreateInfo, VkRenderPass& renderpass, const char* Name = "Unknown");

extern GfxContext* gfxCtx;