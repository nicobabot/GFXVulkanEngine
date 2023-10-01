#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>

#include "Utils.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define WINDOW_NAME "GFXVulkanEngine"
#define APP_NAME "GFXVulkanEngine"
#define APP_VERSION VK_MAKE_VERSION(0,0,1)
#define VULKAN_API_VERSION VK_API_VERSION_1_0

#define MAX_FRAMES_IN_FLIGHT 2

enum LogVerbosity 
{
    NONE = 0,
    INFO,
    VERBOSE
};

#if NDEBUG
static LogVerbosity logDebug = LogVerbosity::NONE;
static bool enableValidationLayers = false;
#else //#if NDEBUG
static LogVerbosity logDebug = LogVerbosity::VERBOSE;
static bool enableValidationLayers = true;
#endif //#else //#if NDEBUG

const std::vector<const char*> validationLayers 
{
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> deviceExtensionsRequired 
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentationFamily;

    bool IsComplete() 
    {
        return graphicsFamily.has_value() && presentationFamily.has_value();
    }
};

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class HelloTriangleApp
{
//Variables
private:
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice logicalDevice;
    VkQueue graphicsQueue;
    VkQueue presentationQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;


//Methods
public:
    void Run();

private:
    void InitWindow();
    void InitVulkan();
    void CreateInstance();
    bool InstanceHasRequiredExtensions(std::vector<const char*> requiredExtensions);
    bool CheckValidationLayerSupport(std::vector<const char*> validationLayers);
    void SetupDebugMessenger();
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, 
    const VkAllocationCallbacks* pAllocator);
    void CreateSurface();
    void PickPhysicalDevice();
    bool IsSuitableDevice(VkPhysicalDevice requestedPhysicalDevice);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice requestedPhysicalDevice);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice requestedPhysicalDevice);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    void CreateLogicalDevice();
    void GetLogicalDeviceQueues();
    void CreateSwapChain();
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void CreateSwapChainImageViews();
    void CreateRenderPass();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void MainLoop();
    void DrawFrame();
    void Cleanup();
};

