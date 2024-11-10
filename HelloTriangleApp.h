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
#include "DebugUtils.h"
#include "GfxPipelineManager.h";
void CreateGraphicsPipeline_Internal(const GraphicsPipelineInfo& graphicPipelineInfo,
    VkPipelineLayout& graphicPipelineLayout, VkPipeline& graphicPipeline, const char* VkPipelineName, const char* VkPipelineLayoutName);

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
    std::optional<uint32_t> graphicsAndComputeFamily;

    bool IsComplete() 
    {
        return graphicsFamily.has_value() && presentationFamily.has_value() 
            && graphicsAndComputeFamily.has_value();
    }
};

struct SwapChainSupportDetails 
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class GfxObject;

//TODO: Add/Create memory allocator
//https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/

class HelloTriangleApp
{
//Variables
private:
    GLFWwindow *window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkQueue presentationQueue;
    VkQueue computeQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkRenderPass shadowMapRenderPass;
    VkRenderPass renderPass;
    VkRenderPass postProcessRenderPass;
    VkRenderPass decalsRenderPass;

    VkDescriptorSetLayout shadowMapDescriptorSetLayout;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSetLayout postProcessDescriptorSetLayout;
    VkDescriptorSetLayout computeDescriptorSetLayout;
    VkDescriptorSetLayout decalsDescriptorSetLayout;

    VkPipelineLayout shadowMapPipelineLayout;
    VkPipeline shadowMapPipeline;

    VkPipelineLayout graphicsPipelineLayout;
    VkPipeline graphicsPipeline;

    VkPipelineLayout postProcessPipelineLayout;
    VkPipeline postProcessPipeline;

    VkPipelineLayout brdfPipelineLayout;
    VkPipeline brdfPipeline;

    VkPipelineLayout decalsPipelineLayout;
    VkPipeline decalsPipeline;

    VkPipelineLayout computePipelineLayout;
    VkPipeline computePipeline;
    std::vector<VkFramebuffer> shadowMapFramebuffers;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkFramebuffer> postProcessFramebuffers;
    std::vector<VkFramebuffer> decalsFramebuffers;

    VkCommandPool computeCommandPool;

    //Depth
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    //DirShadowMapDepth
    VkImage dirShadowMapDepthImage;
    VkDeviceMemory dirShadowMapDepthMemory;
    VkImageView dirShadowMapDepthImageView;

    //First texture
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    uint32_t mipLevels;
    //TODO: Make sampler not related with texture
    VkSampler textureSampler;

    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    VkImage resolveColorImage;
    VkDeviceMemory resolveColorImageMemory;
    VkImageView resolveColorImageView;

    //Blur
    VkImage blurImage;
    VkDeviceMemory blurImageMemory;
    VkImageView blurImageView;

    //PostProcess present
    VkImage postProcessImage;
    VkDeviceMemory postProcessImageMemory;
    VkImageView postProcessImageView;

    //PostProcess quad
    VkBuffer postProcessQuadBuffer;
    VkDeviceMemory postProcessQuadBufferMemory;
    VkBuffer postProcessQuadIndicesBuffer;
    VkDeviceMemory postProcessQuadIndicesBufferMemory;

    //Decals
    VkImage decalsImage;
    VkDeviceMemory decalsImageMemory;
    VkImageView decalsImageView;

    //TODO: store vertex + index in the same buffer for memory aliasing
    //https://developer.nvidia.com/vulkan-memory-management 

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    //Compute
    std::vector<VkBuffer> shaderStorageBuffers;
    std::vector<VkDeviceMemory> shaderStorageBuffersMemory;

    VkDescriptorPool shadowMapDescriptorPool;
    VkDescriptorPool descriptorPool;
    VkDescriptorPool computeDescriptorPool;
    VkDescriptorPool postProcessDescriptorPool;
    VkDescriptorPool decalsDescriptorPool;

    std::vector<VkDescriptorSet> shadowMapDescriptorSets;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkDescriptorSet> computeDescriptorSets;
    std::vector<VkDescriptorSet> postProcessDescriptorSets;
    std::vector<VkDescriptorSet> decalsDescriptorSets;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> computeCommandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame = 0;
    bool framebufferResized = false;

    InputHandler inputHandler;
    GfxLoader gfxLoader;
    std::vector<GfxObject*> objects;

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
    VkSampleCountFlagBits GetMaxUsableSampleCount();
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
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mipLevels,
        const char* imageName = "Unknown");
    void CreateSwapChainImageViews();
    void CreateShadowMapRenderPass();
    void CreateColorRenderPass();
    void CreatePostProcessRenderPass();
    void CreateDecalsRenderPass();
    void CreateShadowMapDescriptorSetLayout();
    void CreateDescriptorSetLayouts();
    void CreatePostProcessDescriptorSetLayout();
    void CreateDecalsDescriptorSetLayout();
    void CreateShadowMapDescriptorPool();
    void CreateColorPassDescriptorPool();
    void CreatePostProcessDescriptorPool();
    void CreateDecalsDescriptorPool();
    void CreateShadowMapDescriptorSets();
    void CreateDescriptorSets();
    void CreatePostProcessDescriptorSets();
    void CreateDecalsDescriptorSets();
    void UpdatePostProcessDescriptorSets();
    void UpdateDecalsDescriptorSets();
    void UpdateDescriptorSets();
    void UpdateComputeDescriptorSets();
    void CreateGraphicsPipeline();
    void CreateDecalsPipeline();
    void CreateShadowMapFramebuffers();
    void CreateFramebuffers();
    void CreatePostProcessFramebuffers();
    void CreateDecalsFramebuffers();
    void CreateCommandPool();
    VkFormat FindSupportedFormat(std::vector<VkFormat> candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat();
    void CreateColorResources();
    void CreateDepthResources();
    void CreateShadowMapResources();
    void CreatePostProcessResources();
    void CreateDecalsResources();
    VkCommandBuffer BeginSingleTimeCommandBuffer();
    void EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSample, VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, const char* imageName = "Unknown");
    void CreateTextureImage();
    void GenerateMipmaps(VkImage image, VkFormat format, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels );
    void CreateTextureImageView();
    void CreateTextureSampler();
    void PopulateObjects();
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, 
        VkMemoryPropertyFlags memoryFlags, VkBuffer& newBuffer, VkDeviceMemory& bufferMemory, const char* BufferName = "Unknown", const char* BufferMemoryName = "Unknown");
    void CreateUniformBuffers();
    void CreateShaderStorageBuffers();
    void CreatePostProcessingQuadBuffer();
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void CreateCommandBuffers();
    void CreateSyncObjects();
    void SetDescriptorsToObjects();
    void RecordComputeCommandBuffer(VkCommandBuffer commandBuffer);
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags);
    VkShaderModule CreateShaderModule(const std::vector<char>& code, const char* Name = "Unknown");
    void MainLoop();
    void UpdateUniformBuffers(uint32_t currentImage);
    void DrawFrame();
    void EndFrameLayoutTransitions(VkCommandBuffer commandBuffer);
    void EndFrame();
    void RecreateSwapChain();
    void CleanupSwapChain();
    void CleanupBuffers();
    void Cleanup();
};

