#include "DebugUtils.h"
#include "GfxContext.h"
#include <iostream>

void DebugUtils::Init()
{
    vkSetDebugUtilsObjectNameEXT =
        (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(gfxCtx->logicalDevice, "vkSetDebugUtilsObjectNameEXT");
    if(vkSetDebugUtilsObjectNameEXT == nullptr)
    {
        throw std::runtime_error("Failed creating vkSetDebugUtilsObjectNameEXT");
    }

    vkDebugMarkerSetObjectNameEXT =
        (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(gfxCtx->logicalDevice, "vkDebugMarkerSetObjectNameEXT");
    if (vkDebugMarkerSetObjectNameEXT == nullptr)
    {
        //throw std::runtime_error("Failed creating vkDebugMarkerSetObjectNameEXT");
    }
}

void DebugUtils::SetVulkanObjectName(VkImage image, const char* Name)
{
    if (true) 
    {
        // Set the debug name for the VkImage (render target)
        VkDebugUtilsObjectNameInfoEXT nameInfo = {};
        nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        nameInfo.objectType = VK_OBJECT_TYPE_IMAGE;  // The type of Vulkan object (e.g., VkImage)
        nameInfo.objectHandle = (uint64_t)image;  // Cast the image handle to uint64_t
        nameInfo.pObjectName = "RenderTarget";  // The debug name you want to assign

        VkResult result = vkSetDebugUtilsObjectNameEXT(gfxCtx->logicalDevice, &nameInfo);
        if(result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed setting image name");
        }
    }
    else
    {
        if(vkDebugMarkerSetObjectNameEXT != nullptr)
        {
            VkDebugMarkerObjectNameInfoEXT nameInfo = {};
            nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
            nameInfo.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT;
            nameInfo.object = (uint64_t)image;
            nameInfo.pObjectName = Name;

            vkDebugMarkerSetObjectNameEXT(gfxCtx->logicalDevice, &nameInfo);
        }
    }
}
