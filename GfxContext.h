#include <vulkan/vulkan_core.h>

class GfxContext
{
    public:
        VkDevice logicalDevice;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkCommandPool commandPool;
        VkQueue graphicsQueue;
};