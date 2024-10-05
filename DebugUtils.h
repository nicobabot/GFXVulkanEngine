#pragma once
#include <vulkan/vulkan_core.h>

class GfxContext;

class DebugUtils
{
public:
	void Init();
	void SetVulkanObjectName(VkImage image, const char* Name );
private:
	PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
	PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectNameEXT;
};

extern GfxContext* gfxCtx;