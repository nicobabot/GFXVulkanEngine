#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>
#include <set>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define WINDOW_NAME "GFXVulkanEngine"
#define APP_NAME "GFXVulkanEngine"
#define APP_VERSION VK_MAKE_VERSION(0,0,1)
#define VULKAN_API_VERSION VK_API_VERSION_1_0

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

const std::vector<const char*> validationLayers {
    "VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> deviceExtensionsRequired {
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
    void CreateLogicalDevice();
    void GetLogicalDeviceQueues();
    void MainLoop();
    void Cleanup();
};

