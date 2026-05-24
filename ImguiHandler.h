#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_glfw.h"
#include "Imgui/imgui_impl_vulkan.h"

struct QueueFamilyIndices;

struct ImguiVulkanObjects
{
    VkDevice logicalDevice;
    VkPhysicalDevice physicalDevice;
    VkInstance instance;
    QueueFamilyIndices* queueFamilyIndices;
    VkQueue queue;
    VkRenderPass renderPass;
    uint32_t imageCount;
    uint32_t minImageCount;
    VkSampleCountFlagBits MSAASamples;
    VkAllocationCallbacks* allocator;
};

class ImguiHandler
{
    protected:
    static ImguiVulkanObjects vkObjects;
    static VkDescriptorPool imguiDescriptorPool;
    static void ImguiVulkanResultLogger(VkResult err);

	public:
	static void InitWindow(GLFWwindow* window);
	static void InitVulkan(ImguiVulkanObjects vulkanObj);
    static void NewFrame();
    static void Draw(const VkCommandBuffer& commandBuffer);
    static void Cleanup();

    public:
    static bool isOpen;
    static bool lastBlurSetting;
    static bool isBlurEnabled;

};

