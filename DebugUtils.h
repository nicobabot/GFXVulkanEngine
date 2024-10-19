#pragma once
#include <vulkan/vulkan_core.h>

class GfxContext;

class DebugUtils
{
public:
	void Init();
	void SetVulkanObjectName(VkImage image, const char* Name );
	void SetVulkanObjectName(VkImageView image, const char* Name );
private:
	void DebugMarkerSetObjectName(uint64_t object, VkDebugReportObjectTypeEXT oType, const char* Name );
	void DebugUtilsSetObjectName(uint64_t object, VkObjectType oType, const char* Name );
private:
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT;
};

extern GfxContext* gfxCtx;