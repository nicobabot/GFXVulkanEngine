#include "DebugUtils.h"
#include "GfxContext.h"
#include <iostream>
#include "ColorsDef.h"

void DebugUtils::Init()
{
    vkDebugMarkerSetObjectNameEXT =
        (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(gfxCtx->logicalDevice, "vkDebugMarkerSetObjectNameEXT");
    if (vkDebugMarkerSetObjectNameEXT == nullptr)
    {
        std::cerr << YELLOW_TEXT << "failed creating vkDebugMarkerSetObjectNameEXT\n" << std::endl;
    }
    else 
    {
        std::cerr << GREEN_TEXT << "vkDebugMarkerSetObjectNameEXT created\n" << std::endl;
    }

    vkSetDebugUtilsObjectNameEXT =
        (PFN_vkSetDebugUtilsObjectNameEXT)vkGetDeviceProcAddr(gfxCtx->logicalDevice, "vkSetDebugUtilsObjectNameEXT");
    if (vkSetDebugUtilsObjectNameEXT == nullptr)
    {
        std::cerr << YELLOW_TEXT << "failed creating vkSetDebugUtilsObjectNameEXT\n" << std::endl;
    }
    else
    {
        std::cerr << GREEN_TEXT << "vkSetDebugUtilsObjectNameEXT created\n" << std::endl;
    }

    std::cerr << RESET_TEXT << std::endl;
}

void DebugUtils::SetVulkanObjectName(VkImage image, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT) 
    {
        DebugUtilsSetObjectName((uint64_t)image, VK_OBJECT_TYPE_IMAGE, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkImageView image, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)image, VK_OBJECT_TYPE_IMAGE_VIEW, Name);
    }
}

void DebugUtils::DebugMarkerSetObjectName(uint64_t object, VkDebugReportObjectTypeEXT oType, const char* Name)
{
    VkDebugMarkerObjectNameInfoEXT nameInfo = {};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
    nameInfo.objectType = oType;
    nameInfo.object = object;
    nameInfo.pObjectName = Name;

    VkResult result = vkDebugMarkerSetObjectNameEXT(gfxCtx->logicalDevice, &nameInfo);
    if (result != VK_SUCCESS)
    {
        std::cerr << YELLOW_TEXT << "failed adding name to " << Name << "\n" << std::endl;
    }
}

void DebugUtils::DebugUtilsSetObjectName(uint64_t object, VkObjectType oType, const char* Name)
{
    // Set the debug name for the VkImage (render target)
    VkDebugUtilsObjectNameInfoEXT nameInfo = {};
    nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    // The type of Vulkan object (e.g., VkImage)
    nameInfo.objectType = oType;
    // Cast the image handle to uint64_t
    nameInfo.objectHandle = object;
    // The debug name you want to assign
    nameInfo.pObjectName = Name;

    VkResult result = vkSetDebugUtilsObjectNameEXT(gfxCtx->logicalDevice, &nameInfo);
    if (result != VK_SUCCESS)
    {
        std::cerr << YELLOW_TEXT << "failed adding name to " << Name << "\n" << std::endl;
    }
}
