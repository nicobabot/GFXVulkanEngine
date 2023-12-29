#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <optional>
#include <set>
#include <algorithm>
#include <array>

#include "Utils.h"

#include "InputHandler.h"
//#include "gfxMaths.h"

#include "ModelLoader.h"

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

//TODO: Add/Create memory allocator
//https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/

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
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    VkCommandPool commandPool;

    //Depth
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    //First texture
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    uint32_t mipLevels;
    //TODO: Make sampler not related with texture
    VkSampler textureSampler;

    //TODO: store vertex + index in the same buffer for memory aliasing
    //https://developer.nvidia.com/vulkan-memory-management 
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    InputHandler inputHandler;
    ModelLoader modelLoader;

//Methods
public:
    void Run();
    void MarkNeedResize();

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
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mipLevels);
    void CreateSwapChainImageViews();
    void CreateRenderPass();
    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateGraphicsPipeline();
    void CreateFramebuffers();
    void CreateCommandPool();
    VkFormat FindSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();
    bool HasStencilComponent(VkFormat format);
    void CreateDepthResources();
    VkCommandBuffer BeginSingleTimeCommandBuffer();
    void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void TransitionImageLayout(VkImage image, VkFormat format, 
        VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void CreateTextureImage();
    void GenerateMipmaps(VkImage image, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels );
    void CreateTextureImageView();
    void CreateTextureSampler();
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, 
        VkMemoryPropertyFlags memoryFlags, VkBuffer& newBuffer, VkDeviceMemory& bufferMemory);
    void CreateVertexBuffers();
    void CreateIndexBuffers();
    void CreateUniformBuffers();
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    void MainLoop();
    void UpdateUniformBuffers(uint32_t currentImage);
    void DrawFrame();
    void RecreateSwapChain();
    void CleanupSwapChain();
    void CleanupBuffers();
    void Cleanup();
};

