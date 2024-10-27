#pragma once
#include <vulkan/vulkan_core.h>

class GfxContext;

class DebugUtils 
{
public:
    // Delete the copy constructor and assignment operator to prevent copies
    DebugUtils(const DebugUtils&) = delete;
    DebugUtils& operator=(const DebugUtils&) = delete;

    static DebugUtils& getInstance() {
        static DebugUtils instance; // created once, destroyed at the program end
        return instance;
    }

	void Init();
	void SetVulkanObjectName(VkImage image, const char* Name);
	void SetVulkanObjectName(VkImageView image, const char* Name);
	void SetVulkanObjectName(VkDescriptorSet descriptorSet, const char* Name);
	void SetVulkanObjectName(VkDescriptorSetLayout descriptorSetLayout, const char* Name);
	void SetVulkanObjectName(VkBuffer buffer, const char* Name);
	void SetVulkanObjectName(VkDeviceMemory deviceMemory, const char* Name);
	void SetVulkanObjectName(VkShaderModule shaderModule, const char* Name);
	void SetVulkanObjectName(VkPipeline pipeline, const char* Name);
	void SetVulkanObjectName(VkPipelineLayout pipelineLayout, const char* Name);
	void SetVulkanObjectName(VkDescriptorPool descriptorPool, const char* Name);
	void SetVulkanObjectName(VkFramebuffer frameBuffer, const char* Name);
	void SetVulkanObjectName(VkRenderPass renderpass, const char* Name);

private:
	void DebugMarkerSetObjectName(uint64_t object, VkDebugReportObjectTypeEXT oType, const char* Name);
	void DebugUtilsSetObjectName(uint64_t object, VkObjectType oType, const char* Name);

	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT;

private:
    DebugUtils() {} // Private constructor to prevent direct instantiation
};