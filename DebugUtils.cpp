#include "DebugUtils.h"
#include "GfxContext.h"
#include <iostream>
#include "ColorsDef.h"

extern GfxContext* gfxCtx;

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

void DebugUtils::SetVulkanObjectName(VkDescriptorSet descriptorSet, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)descriptorSet, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)descriptorSet, VK_OBJECT_TYPE_DESCRIPTOR_SET, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkDescriptorSetLayout descriptorSetLayout, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)descriptorSetLayout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)descriptorSetLayout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkBuffer buffer, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)buffer, VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkDeviceMemory deviceMemory, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)deviceMemory, VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)deviceMemory, VK_OBJECT_TYPE_DEVICE_MEMORY, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkShaderModule shaderModule, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)shaderModule, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)shaderModule, VK_OBJECT_TYPE_SHADER_MODULE, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkPipeline pipeline, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)pipeline, VK_OBJECT_TYPE_PIPELINE, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkPipelineLayout pipelineLayout, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)pipelineLayout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)pipelineLayout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkDescriptorPool descriptorPool, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)descriptorPool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)descriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkFramebuffer frameBuffer, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)frameBuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)frameBuffer, VK_OBJECT_TYPE_FRAMEBUFFER, Name);
    }
}

void DebugUtils::SetVulkanObjectName(VkRenderPass renderpass, const char* Name)
{
    if (vkDebugMarkerSetObjectNameEXT)
    {
        DebugMarkerSetObjectName((uint64_t)renderpass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, Name);
        return;
    }

    if (vkSetDebugUtilsObjectNameEXT)
    {
        DebugUtilsSetObjectName((uint64_t)renderpass, VK_OBJECT_TYPE_RENDER_PASS, Name);
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