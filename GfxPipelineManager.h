#pragma once
#include <vulkan/vulkan_core.h>
#include <vector>

struct GraphicsPipelineInfo 
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
	VkDescriptorSetLayout descriptorSetLayout;
	VkExtent2D viewportExtent;
	VkSampleCountFlagBits msaaSamples;
	VkRenderPass renderPass;

	GraphicsPipelineInfo() = default;
};

class GfxPipelineManager
{
	public:

	void Init(VkDevice logicalDevice);

	void CreateGraphicsPipeline(const GraphicsPipelineInfo& graphicPipelineInfo,
		VkPipelineLayout &graphicPipelineLayout, VkPipeline& graphicPipeline);

	private:
	VkDevice logicalDevice;

};

