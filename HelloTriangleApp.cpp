#include "HelloTriangleApp.h"
#include "MainDefines.h"
#include "ComputeObjectsManager.h"
#include "GfxContext.h"
#include "BasicPolygons.h"

void HelloTriangleApp::Run()
{
    gfxCtx = new GfxContext();
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void HelloTriangleApp::MarkNeedResize()
{
    framebufferResized = true;
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    HelloTriangleApp* program = reinterpret_cast<HelloTriangleApp*>(glfwGetWindowUserPointer(window));
    program->MarkNeedResize();
}

void HelloTriangleApp::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void HelloTriangleApp::InitVulkan()
{
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    GetLogicalDeviceQueues();
    CreateSwapChain();
    CreateSwapChainImageViews();
    CreateShadowMapRenderPass();
    CreateRenderPass();
    CreateShadowMapDescriptorSetLayout();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateColorResources();
    CreateDepthResources();
    CreateShadowMapResources();
    CreateShadowMapFramebuffers();
    CreateFramebuffers();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    gfxLoader.LoadModel();
    PopulateObjects();
    CreateUniformBuffers();
    CreateShaderStorageBuffers();
    CreateShadowMapDescriptorPool();
    CreateDescriptorPool();
    CreateShadowMapDescriptorSets();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
    SetDescriptorsToObjects();
    inputHandler.Init();
}

void HelloTriangleApp::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = APP_NAME;
    appInfo.applicationVersion = APP_VERSION;
    appInfo.pEngineName = APP_NAME;
    appInfo.engineVersion = APP_VERSION;
    appInfo.apiVersion = VULKAN_API_VERSION;
    appInfo.pNext = nullptr;

    VkInstanceCreateInfo instanceInfo{};
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) 
    {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    if (logDebug == LogVerbosity::VERBOSE)
    {
        std::cout << "Required instance extensions" << '\n';
        for (const char* rExtension : requiredExtensions)
        {
            std::cout << '\t' << rExtension << '\n';
        }
    }

    instanceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    instanceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    
    if (!InstanceHasRequiredExtensions(requiredExtensions)) 
    {
        throw std::runtime_error("Instance does not contain required extensions");
    }

    if(enableValidationLayers)
    {
        if (!CheckValidationLayerSupport(validationLayers)) 
        {
            throw std::runtime_error("Instance does not contain requested validation layers");
        }

        instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        instanceInfo.ppEnabledLayerNames = validationLayers.data();
        VkDebugUtilsMessengerCreateInfoEXT createInfoDebugMessenger{};
        PopulateDebugMessengerCreateInfo(createInfoDebugMessenger);
        instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &createInfoDebugMessenger;
        }
    else
    {
        instanceInfo.enabledLayerCount = 0;
        instanceInfo.pNext = nullptr;
    }
    
    if (vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed creating instance");
    }

}

bool HelloTriangleApp::InstanceHasRequiredExtensions(std::vector<const char*> requiredExtensions)
{
    uint32_t instanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> instanceExtensionProperties(instanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensionProperties.data());

    if (logDebug == LogVerbosity::VERBOSE)
    {
        std::cout << "Available instance extensions" << '\n';
        for (VkExtensionProperties iExtension : instanceExtensionProperties)
        {
            std::cout << '\t' << iExtension.extensionName << '\n';
        }
    }


    int32_t extensionsMatching = 0;
    for (const char* rExtension : requiredExtensions) 
    {
        for (VkExtensionProperties iExtension : instanceExtensionProperties) 
        {

            if (strcmp(rExtension, iExtension.extensionName) == 0) 
            {
                ++extensionsMatching;
            }
        }
    }

    return extensionsMatching >= requiredExtensions.size();
}

bool HelloTriangleApp::CheckValidationLayerSupport(std::vector<const char*> validationLayers)
{
    uint32_t instanceLayerCount = 0;
    vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
    std::vector<VkLayerProperties> instanceLayers(instanceLayerCount);
    vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers.data());

    if(logDebug == LogVerbosity::VERBOSE)
    {
        std::cout << "Instance Layers" << '\n';
        for (const VkLayerProperties& layerProperty : instanceLayers) 
        {
            std::cout << '\t' << layerProperty.layerName << '\n';
        }
    }

    for (const char* vLayer : validationLayers) 
    {
        bool validationLayerFound = false;
        for (const VkLayerProperties &layerProperty : instanceLayers) 
        {
            if (strcmp(vLayer, layerProperty.layerName) == 0) {
                validationLayerFound = true;
                break;
            }
        }

        if (!validationLayerFound)
        {
            return false;
        }        
    }

    return true;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationErrorLogger(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    if(messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        std::cerr << RED_TEXT << "validation layer: " << pCallbackData->pMessage << "\n" << std::endl;
    }
    else
    {
        std::cerr << YELLOW_TEXT << "validation layer: " << pCallbackData->pMessage << "\n" << std::endl;
    }

    std::cerr << RESET_TEXT << std::endl;

    return VK_FALSE;
}

void HelloTriangleApp::SetupDebugMessenger()
{
    if (!enableValidationLayers) 
    {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo {};
    PopulateDebugMessengerCreateInfo(debugMessengerCreateInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &debugMessengerCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) 
    {
        throw std::runtime_error("Unable creating debug messenger");
    }

}

void HelloTriangleApp::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = ValidationErrorLogger;
}

VkResult HelloTriangleApp::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void HelloTriangleApp::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void HelloTriangleApp::CreateSurface()
{
    if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating VkSurface");
    }
}

VkSampleCountFlagBits HelloTriangleApp::GetMaxUsableSampleCount()
{
    VkPhysicalDeviceProperties prop{};
    vkGetPhysicalDeviceProperties(gfxCtx->physicalDevice, &prop);
    VkSampleCountFlags sampleCount = prop.limits.framebufferColorSampleCounts 
        & prop.limits.framebufferDepthSampleCounts;

    if(sampleCount & VK_SAMPLE_COUNT_64_BIT) return VK_SAMPLE_COUNT_64_BIT;
    if(sampleCount & VK_SAMPLE_COUNT_32_BIT) return VK_SAMPLE_COUNT_32_BIT;
    if(sampleCount & VK_SAMPLE_COUNT_16_BIT) return VK_SAMPLE_COUNT_16_BIT;
    if(sampleCount & VK_SAMPLE_COUNT_8_BIT) return VK_SAMPLE_COUNT_8_BIT;
    if(sampleCount & VK_SAMPLE_COUNT_4_BIT) return VK_SAMPLE_COUNT_4_BIT;
    if(sampleCount & VK_SAMPLE_COUNT_2_BIT) return VK_SAMPLE_COUNT_2_BIT;

    return VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
}

void HelloTriangleApp::PickPhysicalDevice()
{
    uint32_t physicalDevicesCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, nullptr);
    if (physicalDevicesCount == 0) 
    {
        throw std::runtime_error("There is no physical devices");
    }

    std::vector<VkPhysicalDevice> physicalDevices(physicalDevicesCount);
    vkEnumeratePhysicalDevices(instance, &physicalDevicesCount, physicalDevices.data());

    for (VkPhysicalDevice pDevice : physicalDevices) 
    {
        if (IsSuitableDevice(pDevice)) 
        {
            gfxCtx->physicalDevice = pDevice;
            msaaSamples = GetMaxUsableSampleCount();
            break;
        }
    }

    if (gfxCtx->physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Error finding sutiable device");
    }
}

bool HelloTriangleApp::IsSuitableDevice(VkPhysicalDevice requestedPhysicalDevice)
{
    VkPhysicalDeviceProperties dProperties;
    vkGetPhysicalDeviceProperties(requestedPhysicalDevice, &dProperties);
    VkPhysicalDeviceFeatures dFeatures;
    vkGetPhysicalDeviceFeatures(requestedPhysicalDevice, &dFeatures);

    bool hasExtensionSupport = CheckDeviceExtensionSupport(requestedPhysicalDevice);

    bool isAdequateSwapchain = false;
    if (hasExtensionSupport) 
    {
        SwapChainSupportDetails swapchainDetails = QuerySwapChainSupport(requestedPhysicalDevice);
        isAdequateSwapchain = !swapchainDetails.formats.empty() && !swapchainDetails.presentModes.empty();
    }

    return dProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        dFeatures.geometryShader && FindQueueFamilies(requestedPhysicalDevice).IsComplete()
        && hasExtensionSupport && isAdequateSwapchain;
}

bool HelloTriangleApp::CheckDeviceExtensionSupport(VkPhysicalDevice requestedPhysicalDevice)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(requestedPhysicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(requestedPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions (deviceExtensionsRequired.begin(), deviceExtensionsRequired.end());

    if(logDebug == LogVerbosity::VERBOSE)
    {
        std::cout << "Available Extensions" << '\n';
        for (const VkExtensionProperties& extension : availableExtensions)
        {
            std::cout << '\t' << extension.extensionName << '\n';
        }
    }

    for (const VkExtensionProperties& extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices HelloTriangleApp::FindQueueFamilies(VkPhysicalDevice requestedPhysicalDevice)
{
    QueueFamilyIndices queueFamilyIndices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(requestedPhysicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(requestedPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());

    int i = 0;
    for (VkQueueFamilyProperties queue : queueFamilyProperties) 
    {
        if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queueFamilyIndices.graphicsFamily = i;
        }

        if ((queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) && 
            (queue.queueFlags & VK_QUEUE_COMPUTE_BIT)) 
        {
            queueFamilyIndices.graphicsAndComputeFamily = i;
        }

        VkBool32 hasSurfaceSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(requestedPhysicalDevice, i, surface, &hasSurfaceSupport);
        if (hasSurfaceSupport) 
        {
            queueFamilyIndices.presentationFamily = i;            
        }

        if (queueFamilyIndices.IsComplete()) 
        {
            break;
        }

        ++i;
    }

    return queueFamilyIndices;
}

SwapChainSupportDetails HelloTriangleApp::QuerySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);
    if (formatsCount > 0)
    {
        details.formats.resize(formatsCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, details.formats.data());
    }

    uint32_t presentationModesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationModesCount, nullptr);
    if (presentationModesCount > 0)
    {
        details.presentModes.resize(presentationModesCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationModesCount, details.presentModes.data());
    }

    return details;
}

void HelloTriangleApp::CreateLogicalDevice()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(gfxCtx->physicalDevice);
    std::set<uint32_t> queueIndices 
    {
        queueFamilyIndices.graphicsFamily.value(), 
        queueFamilyIndices.presentationFamily.value()
    };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t index : queueIndices) 
    {
        VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueFamilyIndex = index;
        deviceQueueCreateInfo.queueCount = 1;
        float queuePriority = 1.0f;
        deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(deviceQueueCreateInfo);
    }

    VkDeviceCreateInfo logicalDeviceCreateInfo{};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    logicalDeviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

    VkPhysicalDeviceFeatures physicalDeviceFeatures{};

    //The sample rate determines the number of subpixels that are actually sampled for that pixel. 
    //Subpixels with higher priority are sampled more frequently, 
    //while subpixels with lower priority are sampled less frequently.
    physicalDeviceFeatures.sampleRateShading = VK_TRUE;

    logicalDeviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    logicalDeviceCreateInfo.ppEnabledExtensionNames = deviceExtensionsRequired.data();
    logicalDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsRequired.size());
    
    if(enableValidationLayers)
    {
        logicalDeviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        logicalDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    }
    else
    {
        logicalDeviceCreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(gfxCtx->physicalDevice, &logicalDeviceCreateInfo, nullptr, &gfxCtx->logicalDevice) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating logical device");
    }
}

void HelloTriangleApp::GetLogicalDeviceQueues()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(gfxCtx->physicalDevice);
    vkGetDeviceQueue(gfxCtx->logicalDevice, queueFamilyIndices.graphicsFamily.value(), 0, &gfxCtx->graphicsQueue);
    vkGetDeviceQueue(gfxCtx->logicalDevice, queueFamilyIndices.presentationFamily.value(), 0, &presentationQueue);
    vkGetDeviceQueue(gfxCtx->logicalDevice, queueFamilyIndices.graphicsAndComputeFamily.value(), 0, &computeQueue);
}

void HelloTriangleApp::CreateSwapChain()
{
    SwapChainSupportDetails swapChainDetails = QuerySwapChainSupport(gfxCtx->physicalDevice);

    VkSurfaceFormatKHR swapChainFormat = ChooseSwapSurfaceFormat(swapChainDetails.formats);
    swapChainImageFormat = swapChainFormat.format;
    VkPresentModeKHR swapChainPesent = ChooseSwapPresentMode(swapChainDetails.presentModes);
    swapChainExtent = ChooseSwapExtent(swapChainDetails.capabilities);

    uint32_t imageCount = swapChainDetails.capabilities.minImageCount + 1;
    uint32_t maxImageInSwapChain = swapChainDetails.capabilities.maxImageCount;
    if (maxImageInSwapChain > 0 && imageCount > maxImageInSwapChain)
    {
        imageCount = maxImageInSwapChain;
    }

    VkSwapchainCreateInfoKHR createSwapChainInfo {};
    createSwapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createSwapChainInfo.surface = surface;
    createSwapChainInfo.imageFormat = swapChainFormat.format;
    createSwapChainInfo.imageColorSpace = swapChainFormat.colorSpace;
    createSwapChainInfo.presentMode = swapChainPesent;
    createSwapChainInfo.imageExtent = swapChainExtent;
    createSwapChainInfo.imageArrayLayers = 1;
    createSwapChainInfo.minImageCount = imageCount;
    //VK_IMAGE_USAGE_TRANSFER_DST_BIT -> to render to a separate image first to perform operations (post processing)
    createSwapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(gfxCtx->physicalDevice);
    uint32_t familyIndices[] = 
    { 
        queueFamilyIndices.graphicsFamily.value(), 
        queueFamilyIndices.presentationFamily.value() 
    };

    if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentationFamily) 
    {
        createSwapChainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createSwapChainInfo.queueFamilyIndexCount = 2;
        createSwapChainInfo.pQueueFamilyIndices = familyIndices;
    }
    else
    {
        createSwapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createSwapChainInfo.queueFamilyIndexCount = 0;
        createSwapChainInfo.pQueueFamilyIndices = nullptr;
    }
    createSwapChainInfo.preTransform = swapChainDetails.capabilities.currentTransform;
    createSwapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createSwapChainInfo.clipped = VK_TRUE;
    //TODO RECREATE SWAPCHAIN (resize?)
    createSwapChainInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(gfxCtx->logicalDevice, &createSwapChainInfo, nullptr, &swapChain) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating SwapChain");
    }

    uint32_t swapchainImagesCount = 0;
    vkGetSwapchainImagesKHR(gfxCtx->logicalDevice, swapChain, &swapchainImagesCount, nullptr);
    swapChainImages.resize(swapchainImagesCount);
    vkGetSwapchainImagesKHR(gfxCtx->logicalDevice, swapChain, &swapchainImagesCount, swapChainImages.data());
}

VkSurfaceFormatKHR HelloTriangleApp::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (VkSurfaceFormatKHR availableSurfaceFormat : availableFormats) 
    {
        if (availableSurfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB
            && availableSurfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableSurfaceFormat;
        }
    }
    
    return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApp::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (VkPresentModeKHR availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D HelloTriangleApp::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return capabilities.currentExtent;
    }
    else 
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = 
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, 
        capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, 
        capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

VkImageView HelloTriangleApp::CreateImageView(VkImage image, VkFormat format, 
    VkImageAspectFlags flags, uint32_t mipLevels)
{
    VkImageView newImageView;
    VkImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.image = image;
    imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCreateInfo.format = format;

    imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    imageViewCreateInfo.subresourceRange.aspectMask = flags;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(gfxCtx->logicalDevice, &imageViewCreateInfo, nullptr, &newImageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating image view");
    }
    return newImageView;
}

void HelloTriangleApp::CreateSwapChainImageViews()
{
    uint32_t swapchainImageCount = swapChainImages.size();
    swapChainImageViews.resize(swapchainImageCount);
    for (int i = 0; i < swapchainImageCount; ++i) 
    {
        swapChainImageViews[i] = CreateImageView(swapChainImages[i], 
            swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
    }
}

void HelloTriangleApp::CreateShadowMapRenderPass()
{
    VkAttachmentDescription depthAttachmentDescr{};
    depthAttachmentDescr.format = FindDepthFormat();
    depthAttachmentDescr.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDescr.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescr.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDescr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDescr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDescr.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDescr.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthAttachment{};
    depthAttachment.attachment = 0;
    depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescr{};
    subpassDescr.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescr.colorAttachmentCount = 0;
    subpassDescr.pDepthStencilAttachment = &depthAttachment;

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 1> attachments = { depthAttachmentDescr };
    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescr;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if (vkCreateRenderPass(gfxCtx->logicalDevice, &renderPassCreateInfo, nullptr, &shadowMapRenderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating renderpass!");
    }
}

void HelloTriangleApp::CreateRenderPass()
{
    VkAttachmentDescription colorAttachmentDescr{};
    colorAttachmentDescr.format = swapChainImageFormat;
    colorAttachmentDescr.samples = msaaSamples;
    colorAttachmentDescr.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; 
    colorAttachmentDescr.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDescr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDescr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDescr.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDescr.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachment{};
    colorAttachment.attachment = 0;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachmentDescr{};
    depthAttachmentDescr.format = FindDepthFormat();
    depthAttachmentDescr.samples = msaaSamples;
    depthAttachmentDescr.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDescr.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDescr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDescr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDescr.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDescr.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachment{};
    depthAttachment.attachment = 1;
    depthAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorResolveReference{};
    colorResolveReference.attachment = 2;
    colorResolveReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescr{};
    subpassDescr.colorAttachmentCount = 1;
    subpassDescr.pColorAttachments = &colorAttachment;
    subpassDescr.pDepthStencilAttachment = &depthAttachment;
    subpassDescr.pResolveAttachments = &colorResolveReference;

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 3> attachments = {colorAttachmentDescr, 
        depthAttachmentDescr, colorAttachmentResolve };
    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassCreateInfo.pAttachments = attachments.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescr;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if (vkCreateRenderPass(gfxCtx->logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating renderpass!");
    }
}

void HelloTriangleApp::CreateShadowMapDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
    VkDescriptorSetLayoutCreateInfo descriptorSetCreateInfo{};
    descriptorSetCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetCreateInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(gfxCtx->logicalDevice, &descriptorSetCreateInfo,
        nullptr, &shadowMapDescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating descriptor set layout!");
    }
}

void HelloTriangleApp::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding sampledImageLayoutBinding{};
    sampledImageLayoutBinding.binding = 2;
    sampledImageLayoutBinding.descriptorCount = 1;
    sampledImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    sampledImageLayoutBinding.pImmutableSamplers = nullptr;
    sampledImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding depthShadowImageLayoutBinding{};
    depthShadowImageLayoutBinding.binding = 3;
    depthShadowImageLayoutBinding.descriptorCount = 1;
    depthShadowImageLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    depthShadowImageLayoutBinding.pImmutableSamplers = nullptr;
    depthShadowImageLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 4> bindings = {uboLayoutBinding, 
        samplerLayoutBinding, sampledImageLayoutBinding, depthShadowImageLayoutBinding };
    VkDescriptorSetLayoutCreateInfo descriptorSetCreateInfo{};
    descriptorSetCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetCreateInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(gfxCtx->logicalDevice, &descriptorSetCreateInfo,
        nullptr, &descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating descriptor set layout!");
    }

    //Compute
#if COMPUTE_FEATURE
    std::array<VkDescriptorSetLayoutBinding, 3> layoutBindings{};
    layoutBindings[0].binding = 0;
    layoutBindings[0].descriptorCount = 1;
    layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[0].pImmutableSamplers = nullptr;

    layoutBindings[1].binding = 1;
    layoutBindings[1].descriptorCount = 1;
    layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[1].pImmutableSamplers = nullptr;

    layoutBindings[2].binding = 2;
    layoutBindings[2].descriptorCount = 1;
    layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layoutBindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    layoutBindings[2].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo computeLayoutInfo{};
    computeLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    computeLayoutInfo.bindingCount = layoutBindings.size();
    computeLayoutInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(gfxCtx->logicalDevice, &computeLayoutInfo,
        nullptr, &computeDescriptorSetLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating compute descriptr set layout!");
    }
#endif //#if COMPUTE_FEATURE
}

void HelloTriangleApp::CreateGraphicsPipeline()
{
    inputHandler.CompileShaders();

    std::vector<char> vertexShader = ReadFile("CompiledShaders/vert.spv");
    std::vector<char> fragmentShader = ReadFile("CompiledShaders/frag.spv");

    VkShaderModule vertexShaderModule = CreateShaderModule(vertexShader);
    VkShaderModule fragmentShaderModule = CreateShaderModule(fragmentShader);

    VkPipelineShaderStageCreateInfo vertexPipelineCreateInfo{};
    vertexPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexPipelineCreateInfo.module = vertexShaderModule;
    vertexPipelineCreateInfo.pName = "VSMain";
    vertexPipelineCreateInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragmentPipelineCreateInfo{};
    fragmentPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentPipelineCreateInfo.module = fragmentShaderModule;
    fragmentPipelineCreateInfo.pName = "PSMain";
    fragmentPipelineCreateInfo.pSpecializationInfo = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo>  shaderStages {vertexPipelineCreateInfo,
        fragmentPipelineCreateInfo};
    
    GraphicsPipelineInfo graphicPipelineInfo{};
    graphicPipelineInfo.descriptorSetLayout = descriptorSetLayout;
    graphicPipelineInfo.shaderStages = shaderStages;
    graphicPipelineInfo.renderPass = renderPass;
    graphicPipelineInfo.msaaSamples = msaaSamples;
    graphicPipelineInfo.viewportExtent = swapChainExtent;

    CreateGraphicsPipeline_Internal(graphicPipelineInfo, graphicsPipelineLayout, graphicsPipeline);

    //Shadow Map graphics pipeline
    std::vector<char> shadowMapVertexShader = ReadFile("CompiledShaders/shadowMapVert.spv");
    std::vector<char> shadowMapFragmentShader = ReadFile("CompiledShaders/shadowMapFrag.spv");

    VkShaderModule shadowMapVertexShaderModule = CreateShaderModule(shadowMapVertexShader);
    VkShaderModule shadowMapFragmentShaderModule = CreateShaderModule(shadowMapFragmentShader);

    VkPipelineShaderStageCreateInfo shadowMapVertexPipelineCreateInfo{};
    shadowMapVertexPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shadowMapVertexPipelineCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shadowMapVertexPipelineCreateInfo.module = shadowMapVertexShaderModule;
    shadowMapVertexPipelineCreateInfo.pName = "VSMain";
    shadowMapVertexPipelineCreateInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shadowMapFragmentPipelineCreateInfo{};
    shadowMapFragmentPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shadowMapFragmentPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shadowMapFragmentPipelineCreateInfo.module = shadowMapFragmentShaderModule;
    shadowMapFragmentPipelineCreateInfo.pName = "PSMain";
    shadowMapFragmentPipelineCreateInfo.pSpecializationInfo = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo>  shadowMapShaderStages{ shadowMapVertexPipelineCreateInfo,
        shadowMapFragmentPipelineCreateInfo };

    GraphicsPipelineInfo shadowMapGraphicPipelineInfo{};
    shadowMapGraphicPipelineInfo.descriptorSetLayout = shadowMapDescriptorSetLayout;
    shadowMapGraphicPipelineInfo.shaderStages = shadowMapShaderStages;
    shadowMapGraphicPipelineInfo.renderPass = shadowMapRenderPass;
    shadowMapGraphicPipelineInfo.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    shadowMapGraphicPipelineInfo.viewportExtent = swapChainExtent;

    CreateGraphicsPipeline_Internal(shadowMapGraphicPipelineInfo, shadowMapPipelineLayout, shadowMapPipeline);

   /*/ std::vector<char> brdfFragmentShader = ReadFile("CompiledShaders/brdfFrag.spv");

    VkShaderModule brdfFragmentShaderModule = CreateShaderModule(brdfFragmentShader);

    VkPipelineShaderStageCreateInfo brdfFragmentPipelineCreateInfo{};
    brdfFragmentPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    brdfFragmentPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    brdfFragmentPipelineCreateInfo.module = brdfFragmentShaderModule;
    brdfFragmentPipelineCreateInfo.pName = "main";
    brdfFragmentPipelineCreateInfo.pSpecializationInfo = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo>  brdfShaderStages {vertexPipelineCreateInfo,
        brdfFragmentPipelineCreateInfo};

    GraphicsPipelineInfo brdfGraphicPipelineInfo{};
    brdfGraphicPipelineInfo.descriptorSetLayout = descriptorSetLayout;
    brdfGraphicPipelineInfo.shaderStages = brdfShaderStages;
    brdfGraphicPipelineInfo.renderPass = renderPass;
    brdfGraphicPipelineInfo.msaaSamples = msaaSamples;
    brdfGraphicPipelineInfo.viewportExtent = swapChainExtent;

    CreateGraphicsPipeline_Internal(brdfGraphicPipelineInfo,
        brdfPipelineLayout, brdfPipeline);*/

    vkDestroyShaderModule(gfxCtx->logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(gfxCtx->logicalDevice, fragmentShaderModule, nullptr);    
    vkDestroyShaderModule(gfxCtx->logicalDevice, shadowMapVertexShaderModule, nullptr);
    vkDestroyShaderModule(gfxCtx->logicalDevice, shadowMapFragmentShaderModule, nullptr);
   // vkDestroyShaderModule(gfxCtx->logicalDevice, brdfFragmentShaderModule, nullptr);

#if COMPUTE_FEATURE

    std::vector<char> computeShader = ReadFile("CompiledShaders/compute.spv");
    VkShaderModule computeShaderModule = CreateShaderModule(computeShader);

    VkPipelineShaderStageCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computePipelineCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computePipelineCreateInfo.module = computeShaderModule;
    computePipelineCreateInfo.pName = "main";

    VkPipelineLayoutCreateInfo computePipelineLayoutInfo{};
    computePipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computePipelineLayoutInfo.setLayoutCount = 1;
    computePipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

    if (vkCreatePipelineLayout(gfxCtx->logicalDevice,
        &computePipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating compute pipeline layout!");
    }

    VkComputePipelineCreateInfo computePipelineInfo{};
    computePipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineInfo.layout = computePipelineLayout;
    computePipelineInfo.stage = computePipelineCreateInfo;

    if(vkCreateComputePipelines(gfxCtx->logicalDevice, VK_NULL_HANDLE, 1, 
        &computePipelineInfo, nullptr, &computePipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating compute pipeline!");
    }

    vkDestroyShaderModule(gfxCtx->logicalDevice, computeShaderModule, nullptr);
#endif//#if COMPUTE_FEATURE
}

void HelloTriangleApp::CreateShadowMapFramebuffers()
{
    shadowMapFramebuffers.resize(swapChainImageViews.size());
    for (int i = 0; i < swapChainImageViews.size(); ++i)
    {
        std::array<VkImageView, 1> attachments
        {
            dirShadowMapDepthImageView
        };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = shadowMapRenderPass;
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = swapChainExtent.width;
        framebufferCreateInfo.height = swapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(gfxCtx->logicalDevice, &framebufferCreateInfo, nullptr, &shadowMapFramebuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error creating framebuffer!");
        }
    }
}

void HelloTriangleApp::CreateFramebuffers()
{
    swapchainFramebuffers.resize(swapChainImageViews.size());
    for (int i = 0; i < swapChainImageViews.size(); ++i) 
    {
        std::array<VkImageView,3> attachments
        {
            colorImageView,
            depthImageView,
            swapChainImageViews[i],
        };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferCreateInfo.pAttachments = attachments.data();
        framebufferCreateInfo.width = swapChainExtent.width;
        framebufferCreateInfo.height = swapChainExtent.height;
        framebufferCreateInfo.layers = 1;

        if (vkCreateFramebuffer(gfxCtx->logicalDevice, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) 
        {   
            throw std::runtime_error("Error creating framebuffer!");
        }
    }
}

void HelloTriangleApp::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(gfxCtx->physicalDevice);

    VkCommandPoolCreateInfo commandoPoolCreateInfo{};
    commandoPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandoPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandoPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(gfxCtx->logicalDevice, &commandoPoolCreateInfo, nullptr, &gfxCtx->commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating command pool!");
    }

#if COMPUTE_FEATURE
    commandoPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();

    if (vkCreateCommandPool(gfxCtx->logicalDevice, &commandoPoolCreateInfo, nullptr, &computeCommandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating compute command pool!");
    }
#endif//#if COMPUTE_FEATURE
}

VkFormat HelloTriangleApp::FindSupportedFormat(std::vector<VkFormat> candidates, 
    VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) 
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(gfxCtx->physicalDevice, format, &formatProperties);

        if (tiling == VK_IMAGE_TILING_LINEAR && 
            (formatProperties.linearTilingFeatures & features) == features) 
        {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
            (formatProperties.optimalTilingFeatures & features) == features) 
        {
            return format;
        }

    }

    throw std::runtime_error("failed finding supported format!");
}

VkFormat HelloTriangleApp::FindDepthFormat()
{
    return FindSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool HelloTriangleApp::HasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void HelloTriangleApp::CreateColorResources()
{

    VkFormat colorFormat = swapChainImageFormat;

    CreateImage(swapChainExtent.width, swapChainExtent.height, 1, msaaSamples, 
        colorFormat, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        colorImage, colorImageMemory);
    colorImageView = CreateImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
}

void HelloTriangleApp::CreateDepthResources()
{
    VkFormat depthFormat = FindDepthFormat();

    CreateImage(swapChainExtent.width, swapChainExtent.height, 1,
        msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        depthImage, depthImageMemory);

    depthImageView = CreateImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    TransitionImageLayout(depthImage, depthFormat,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

void HelloTriangleApp::CreateShadowMapResources() 
{
    VkFormat depthFormat = FindDepthFormat();

    CreateImage(swapChainExtent.width, swapChainExtent.height, 1,
        VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        dirShadowMapDepthImage, dirShadowMapDepthMemory);

    dirShadowMapDepthImageView = CreateImageView(dirShadowMapDepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    TransitionImageLayout(dirShadowMapDepthImage, depthFormat,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkCommandBuffer HelloTriangleApp::BeginSingleTimeCommandBuffer()
{
    return BeginSingleTimeCommandBuffer_Internal();
}

void HelloTriangleApp::EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
    EndSingleTimeCommandBuffer_Internal(commandBuffer);
}

void HelloTriangleApp::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageExtent.width = width; 
    region.imageExtent.height = height; 
    region.imageExtent.depth = 1; 

    region.imageOffset = {0,0,0};

    region.imageSubresource.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    EndSingleTimeCommandBuffer(commandBuffer);
}

void HelloTriangleApp::TransitionImageLayout(VkImage image, VkFormat format, 
    VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

    VkImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageMemoryBarrier.oldLayout = oldLayout;
    imageMemoryBarrier.newLayout = newLayout;
    imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageMemoryBarrier.image = image;
    
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if(HasStencilComponent(format))
        {
            imageMemoryBarrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else 
    {
        imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    imageMemoryBarrier.subresourceRange.layerCount = 1;
    
    VkPipelineStageFlags srcStageFlags, dstStageFlags;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT 
            | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

        srcStageFlags = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStageFlags = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dstStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL &&
        newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;;

        srcStageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dstStageFlags = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::runtime_error("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(commandBuffer,
    srcStageFlags,
    dstStageFlags, 
    0, 
    0, nullptr, 
    0, nullptr, 
    1, &imageMemoryBarrier);

    EndSingleTimeCommandBuffer(commandBuffer);
}

void HelloTriangleApp::CreateImage(uint32_t width, uint32_t height, uint32_t mipLevels, 
    VkSampleCountFlagBits numSample, VkFormat format, VkImageTiling tiling, 
    VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = static_cast<uint32_t>(width);
    imageCreateInfo.extent.height = static_cast<uint32_t>(height);
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = tiling;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = numSample;
    imageCreateInfo.flags = 0;

    if (vkCreateImage(gfxCtx->logicalDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating image!");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(gfxCtx->logicalDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits,
        properties);

    if (vkAllocateMemory(gfxCtx->logicalDevice, &memoryAllocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating memory for created image!");
    }

    vkBindImageMemory(gfxCtx->logicalDevice, image, imageMemory, 0);
}

void HelloTriangleApp::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    unsigned char* pixels = gfxLoader.LoadTexture(&texWidth, &texHeight, &texChannels);

    /*
    -The max function selects the largest dimension. 
    -The log2 function calculates how many times that dimension can be divided by 2. 
    -The floor function handles cases where the largest dimension is not a power of 2. 
    -1 is added so that the original image has a mip level.
    */
    mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    if (pixels == nullptr) 
    {
        throw std::runtime_error("Error loading image!");
    }

    VkDeviceSize imageSize = texWidth * texHeight * 4;
    VkBuffer stagingBuffer;
    VkDeviceMemory staginBufferMemory;
    CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, staginBufferMemory);

    void* data;
    vkMapMemory(gfxCtx->logicalDevice, staginBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(gfxCtx->logicalDevice, staginBufferMemory);

    gfxLoader.FreeTextureArrayInfo(pixels);

    CreateImage(texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | 
        VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        textureImage, textureImageMemory);

    //TODO: make this operations in one command buffer async, now are various command buffers with single task

    TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
    CopyBufferToImage(stagingBuffer, textureImage, 
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    GenerateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

    vkDestroyBuffer(gfxCtx->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, staginBufferMemory, nullptr);
}

void HelloTriangleApp::GenerateMipmaps(VkImage image, VkFormat format, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels)
{
    VkFormatProperties formatProp{};
    vkGetPhysicalDeviceFormatProperties(gfxCtx->physicalDevice, format, &formatProp);

    //Not all formats support linear blitting
    //TODO: implement software runtime mipmap generation for non suported types (stb_image_resize)
    if(!(formatProp.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("texture format does not support linear blitting!");
    }
    
    VkCommandBuffer cmdBuff = BeginSingleTimeCommandBuffer();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = texWidth;
    int32_t mipHeight = texHeight;

    for (int i = 1; i < mipLevels; ++i) 
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        //barrier to transition previous mip to source optimal layout so we then copy
        vkCmdPipelineBarrier(cmdBuff, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0,nullptr, 0, nullptr, 1, &barrier);

        VkImageBlit imageBlit{};
        imageBlit.srcOffsets[0] = {0,0,0};
        imageBlit.srcOffsets[1] = { mipWidth, mipHeight, 1};
        imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.srcSubresource.mipLevel = i - 1;
        imageBlit.srcSubresource.baseArrayLayer = 0;
        imageBlit.srcSubresource.layerCount = 1;

        imageBlit.dstOffsets[0] = {0,0,0};
        imageBlit.dstOffsets[1] = {mipWidth > 1 ? mipWidth/2 : 1, mipHeight > 1 ? mipHeight/2 : 1, 1};
        imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageBlit.dstSubresource.mipLevel = i;
        imageBlit.dstSubresource.baseArrayLayer = 0;
        imageBlit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(cmdBuff, 
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &imageBlit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmdBuff, 
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 
        0, nullptr, 
        0, nullptr, 
        1, &barrier);

        if (mipWidth > 1) 
        {
            mipWidth /= 2;
        }
        if(mipHeight > 1)
        {
            mipHeight /= 2;
        }
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmdBuff, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
    0, nullptr,
    0, nullptr,
    1, &barrier);

    EndSingleTimeCommandBuffer(cmdBuff);
}

void HelloTriangleApp::CreateTextureImageView()
{    
    textureImageView = CreateImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, 
        VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

void HelloTriangleApp::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    VkPhysicalDeviceFeatures dFeatures;
    vkGetPhysicalDeviceFeatures(gfxCtx->physicalDevice, &dFeatures);
    samplerCreateInfo.anisotropyEnable = dFeatures.samplerAnisotropy ? VK_TRUE : VK_FALSE;
    
    //TODO: store phys device properties in global variable instead of query here
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(gfxCtx->physicalDevice, &physicalDeviceProperties);

    samplerCreateInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.maxLod = static_cast<float>(mipLevels);
    samplerCreateInfo.minLod = 0.0f;

    if (vkCreateSampler(gfxCtx->logicalDevice, &samplerCreateInfo, nullptr, &textureSampler) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating sampler!");
    }

}

void HelloTriangleApp::PopulateObjects()
{
    objects.push_back(new GfxCube(graphicsPipeline, graphicsPipelineLayout));
    objects.push_back(new GfxSphere(graphicsPipeline, graphicsPipelineLayout));
    objects.push_back(new GfxPlane(graphicsPipeline, graphicsPipelineLayout));
}

void HelloTriangleApp::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, 
    VkMemoryPropertyFlags memoryFlags, VkBuffer& newBuffer, VkDeviceMemory& bufferMemory)
{
    CreateBuffer_Internal(size, usageFlags, memoryFlags, newBuffer, bufferMemory);
}

void HelloTriangleApp::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) 
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            uniformBuffers[i], uniformBuffersMemory[i]);

        vkMapMemory(gfxCtx->logicalDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }

}

void HelloTriangleApp::CreateShaderStorageBuffers()
{
    shaderStorageBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    shaderStorageBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

    std::vector <TestComputeClass> objects = InitializeRandomClass();

    VkDeviceSize bufferSize = sizeof(TestComputeClass) * objects.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(gfxCtx->logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, objects.data(), bufferSize);
    vkUnmapMemory(gfxCtx->logicalDevice, stagingBufferMemory);

    for(int i=0; i<MAX_FRAMES_IN_FLIGHT; ++i)
    {
        CreateBuffer(bufferSize, 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, shaderStorageBuffers[i], shaderStorageBuffersMemory[i]);

        CopyBuffer(stagingBuffer, shaderStorageBuffers[i], bufferSize);
    }

    vkDestroyBuffer(gfxCtx->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, stagingBufferMemory, nullptr);
}

void HelloTriangleApp::CreateShadowMapDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 1> descriptorPoolSize;
    descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size());
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize.data();
    descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(gfxCtx->logicalDevice, &descriptorPoolCreateInfo,
        nullptr, &shadowMapDescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating descriptor pool!");
    }
}

void HelloTriangleApp::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 4> descriptorPoolSize;
    descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorPoolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorPoolSize[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorPoolSize[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorPoolSize[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorPoolSize[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size());
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize.data();
    descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(gfxCtx->logicalDevice, &descriptorPoolCreateInfo,
        nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating descriptor pool!");
    }

    //Compute
#if COMPUTE_FEATURE
    std::array<VkDescriptorPoolSize, 3> computeDescriptorPoolSize;
    computeDescriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    computeDescriptorPoolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    computeDescriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    computeDescriptorPoolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    computeDescriptorPoolSize[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    computeDescriptorPoolSize[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo computeDescriptorPoolCreateInfo{};
    computeDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    computeDescriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(computeDescriptorPoolSize.size());
    computeDescriptorPoolCreateInfo.pPoolSizes = computeDescriptorPoolSize.data();
    computeDescriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(gfxCtx->logicalDevice, &computeDescriptorPoolCreateInfo,
        nullptr, &computeDescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating compute descriptor pool!");
    }
#endif//#if COMPUTE_FEATURE
}

void HelloTriangleApp::CreateShadowMapDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, shadowMapDescriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = shadowMapDescriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorSetAllocateInfo.pSetLayouts = layouts.data();

    shadowMapDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(gfxCtx->logicalDevice, &descriptorSetAllocateInfo,
        shadowMapDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating descriptor sets!");
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 1> writeDescriptorSet{};
        writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[0].dstSet = shadowMapDescriptorSets[i];
        writeDescriptorSet[0].dstBinding = 0;
        writeDescriptorSet[0].dstArrayElement = 0;
        writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[0].descriptorCount = 1;
        writeDescriptorSet[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(gfxCtx->logicalDevice, static_cast<uint32_t>(writeDescriptorSet.size()),
            writeDescriptorSet.data(), 0, nullptr);
    }

}

void HelloTriangleApp::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool = descriptorPool;
    descriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorSetAllocateInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(gfxCtx->logicalDevice, &descriptorSetAllocateInfo,
        descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating descriptor sets!");
    }

#if COMPUTE_FEATURE
    std::vector<VkDescriptorSetLayout> conmputeLayouts(MAX_FRAMES_IN_FLIGHT, computeDescriptorSetLayout);
    VkDescriptorSetAllocateInfo computeDescriptorSetAllocateInfo{};
    computeDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    computeDescriptorSetAllocateInfo.descriptorPool = computeDescriptorPool;
    computeDescriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    computeDescriptorSetAllocateInfo.pSetLayouts = conmputeLayouts.data();

    computeDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(gfxCtx->logicalDevice, &computeDescriptorSetAllocateInfo,
        computeDescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating compute descriptor sets!");
    }
#endif//#if COMPUTE_FEATURE

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 4> writeDescriptorSet{};
        writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[0].dstSet = descriptorSets[i];
        writeDescriptorSet[0].dstBinding = 0;
        writeDescriptorSet[0].dstArrayElement = 0;
        writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[0].descriptorCount = 1;
        writeDescriptorSet[0].pBufferInfo = &bufferInfo;

        VkDescriptorImageInfo samplerInfo{};
        samplerInfo.sampler = textureSampler;
        samplerInfo.imageView = nullptr;
        samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[1].dstSet = descriptorSets[i];
        writeDescriptorSet[1].dstBinding = 1;
        writeDescriptorSet[1].dstArrayElement = 0;
        writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        writeDescriptorSet[1].descriptorCount = 1;
        writeDescriptorSet[1].pImageInfo = &samplerInfo;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = textureImageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler = nullptr;

        writeDescriptorSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[2].dstSet = descriptorSets[i];
        writeDescriptorSet[2].dstBinding = 2;
        writeDescriptorSet[2].dstArrayElement = 0;
        writeDescriptorSet[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writeDescriptorSet[2].descriptorCount = 1;
        writeDescriptorSet[2].pImageInfo = &imageInfo;

        VkDescriptorImageInfo imageInfo2{};
        imageInfo2.imageView = dirShadowMapDepthImageView;
        imageInfo2.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        imageInfo2.sampler = nullptr;

        writeDescriptorSet[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[3].dstSet = descriptorSets[i];
        writeDescriptorSet[3].dstBinding = 3;
        writeDescriptorSet[3].dstArrayElement = 0;
        writeDescriptorSet[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writeDescriptorSet[3].descriptorCount = 1;
        writeDescriptorSet[3].pImageInfo = &imageInfo2;

        vkUpdateDescriptorSets(gfxCtx->logicalDevice, static_cast<uint32_t>(writeDescriptorSet.size()),
            writeDescriptorSet.data(), 0, nullptr);

#if COMPUTE_FEATURE

        VkDescriptorBufferInfo storageBufferInfoLastFrame{};
        storageBufferInfoLastFrame.buffer = shaderStorageBuffers[(1-i) % MAX_FRAMES_IN_FLIGHT];
        storageBufferInfoLastFrame.offset = 0;
        storageBufferInfoLastFrame.range = sizeof(TestComputeClass) * OBJECT_COUNT;

        VkDescriptorBufferInfo storageBufferInfoCurrentFrame{};
        storageBufferInfoCurrentFrame.buffer = shaderStorageBuffers[i];
        storageBufferInfoCurrentFrame.offset = 0;
        storageBufferInfoCurrentFrame.range = sizeof(TestComputeClass) * OBJECT_COUNT;
        
        std::array<VkWriteDescriptorSet, 3> computeWriteDescriptorSet{};
        computeWriteDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        computeWriteDescriptorSet[0].dstSet = computeDescriptorSets[i];
        computeWriteDescriptorSet[0].dstBinding = 0;
        computeWriteDescriptorSet[0].dstArrayElement = 0;
        computeWriteDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        computeWriteDescriptorSet[0].descriptorCount = 1;
        computeWriteDescriptorSet[0].pBufferInfo = &bufferInfo;

        computeWriteDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        computeWriteDescriptorSet[1].dstSet = computeDescriptorSets[i];
        computeWriteDescriptorSet[1].dstBinding = 1;
        computeWriteDescriptorSet[1].dstArrayElement = 0;
        computeWriteDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        computeWriteDescriptorSet[1].descriptorCount = 1;
        computeWriteDescriptorSet[1].pBufferInfo = &storageBufferInfoLastFrame;

        computeWriteDescriptorSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        computeWriteDescriptorSet[2].dstSet = computeDescriptorSets[i];
        computeWriteDescriptorSet[2].dstBinding = 2;
        computeWriteDescriptorSet[2].dstArrayElement = 0;
        computeWriteDescriptorSet[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        computeWriteDescriptorSet[2].descriptorCount = 1;
        computeWriteDescriptorSet[2].pBufferInfo = &storageBufferInfoCurrentFrame;

        vkUpdateDescriptorSets(gfxCtx->logicalDevice, 
            computeWriteDescriptorSet.size(), computeWriteDescriptorSet.data(),
            0, nullptr);

#endif//#if COMPUTE_FEATURE
    }
}

void HelloTriangleApp::UpdateDescriptorSets()
{
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 1> writeDescriptorSet{};
        writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[0].dstSet = shadowMapDescriptorSets[i];
        writeDescriptorSet[0].dstBinding = 0;
        writeDescriptorSet[0].dstArrayElement = 0;
        writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[0].descriptorCount = 1;
        writeDescriptorSet[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(gfxCtx->logicalDevice, static_cast<uint32_t>(writeDescriptorSet.size()),
            writeDescriptorSet.data(), 0, nullptr);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        std::array<VkWriteDescriptorSet, 4> writeDescriptorSet{};
        writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[0].dstSet = descriptorSets[i];
        writeDescriptorSet[0].dstBinding = 0;
        writeDescriptorSet[0].dstArrayElement = 0;
        writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[0].descriptorCount = 1;
        writeDescriptorSet[0].pBufferInfo = &bufferInfo;

        VkDescriptorImageInfo samplerInfo{};
        samplerInfo.sampler = textureSampler;
        samplerInfo.imageView = nullptr;
        samplerInfo.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[1].dstSet = descriptorSets[i];
        writeDescriptorSet[1].dstBinding = 1;
        writeDescriptorSet[1].dstArrayElement = 0;
        writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        writeDescriptorSet[1].descriptorCount = 1;
        writeDescriptorSet[1].pImageInfo = &samplerInfo;

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = textureImageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler = nullptr;

        writeDescriptorSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[2].dstSet = descriptorSets[i];
        writeDescriptorSet[2].dstBinding = 2;
        writeDescriptorSet[2].dstArrayElement = 0;
        writeDescriptorSet[2].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writeDescriptorSet[2].descriptorCount = 1;
        writeDescriptorSet[2].pImageInfo = &imageInfo;

        VkDescriptorImageInfo imageInfo2{};
        imageInfo2.imageView = dirShadowMapDepthImageView;
        imageInfo2.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        imageInfo2.sampler = nullptr;

        writeDescriptorSet[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[3].dstSet = descriptorSets[i];
        writeDescriptorSet[3].dstBinding = 3;
        writeDescriptorSet[3].dstArrayElement = 0;
        writeDescriptorSet[3].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        writeDescriptorSet[3].descriptorCount = 1;
        writeDescriptorSet[3].pImageInfo = &imageInfo2;

        vkUpdateDescriptorSets(gfxCtx->logicalDevice, static_cast<uint32_t>(writeDescriptorSet.size()),
            writeDescriptorSet.data(), 0, nullptr);
    }
}

void HelloTriangleApp::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    CopyBuffer_Internal(srcBuffer, dstBuffer, size);
    VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    EndSingleTimeCommandBuffer(commandBuffer);
}

void HelloTriangleApp::CreateCommandBuffers()
{   
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = gfxCtx->commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if(vkAllocateCommandBuffers(gfxCtx->logicalDevice, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating command buffer!");
    }
#if COMPUTE_FEATURE
    computeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = computeCommandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t)computeCommandBuffers.size();

    if (vkAllocateCommandBuffers(gfxCtx->logicalDevice, &commandBufferAllocateInfo, computeCommandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating command buffer!");
    }
#endif//#if COMPUTE_FEATURE
}

void HelloTriangleApp::CreateSyncObjects()
{
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(int i =0; i<MAX_FRAMES_IN_FLIGHT;++i)
    {
        if (vkCreateSemaphore(gfxCtx->logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(gfxCtx->logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(gfxCtx->logicalDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error creating sync objects!");
        }
    }
}

void HelloTriangleApp::SetDescriptorsToObjects()
{
    for(GfxObject* object : objects)
    {
        object->SetDescriptorSetAndLayout(descriptorSets, descriptorSetLayout);
    }
}

void HelloTriangleApp::RecordComputeCommandBuffer(VkCommandBuffer commandBuffer)
{
#if COMPUTE_FEATURE
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
        computePipelineLayout, 0, 1, &computeDescriptorSets[currentFrame], 0, 0);

    vkCmdDispatch(commandBuffer, OBJECT_COUNT / 256, 1, 1);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Error recording command buffer!");
    }
#endif//#if COMPUTE_FEATURE
}

void HelloTriangleApp::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating command buffer!");
    }

    //Shadowmap renderpass
    VkRenderPassBeginInfo shadowMapRenderPassBeginInfo{};
    shadowMapRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    shadowMapRenderPassBeginInfo.renderPass = shadowMapRenderPass;
    shadowMapRenderPassBeginInfo.framebuffer = shadowMapFramebuffers[imageIndex];
    shadowMapRenderPassBeginInfo.renderArea.offset = { 0,0 };
    shadowMapRenderPassBeginInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 1> shadowMapClearValues{};
    shadowMapClearValues[0].depthStencil = { 1.0f, 0 };
    shadowMapRenderPassBeginInfo.clearValueCount = static_cast<uint32_t>(shadowMapClearValues.size());
    shadowMapRenderPassBeginInfo.pClearValues = shadowMapClearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &shadowMapRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        shadowMapPipeline);

    for (GfxObject* object : objects)
    {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = swapChainExtent;
        scissor.offset = { 0, 0 };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { object->vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, object->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            shadowMapPipelineLayout, 0, 1, &shadowMapDescriptorSets[currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object->indices.size()), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);

    //VkFormat depthFormat = FindDepthFormat();
    //TransitionImageLayout(dirShadowMapDepthImage, depthFormat,
    //    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
    //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    //VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL 
    //VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL
    //VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL

    //Color lighting renderpass

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0,0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 3> clearValues{};
    clearValues[0].color = {1.0f,0.0f,1.0f,1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    clearValues[2].color = { 0.0f,0.0f,0.0f,1.0f };
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    for(GfxObject* object : objects)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
            object->graphicsPipeline );

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapChainExtent.width);
        viewport.height = static_cast<float>(swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.extent = swapChainExtent;
        scissor.offset = {0, 0};
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { object->vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, object->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            object->graphicsPipelineLayout , 0, 1, &object->descriptorSet[currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(object->indices.size()), 1, 0, 0, 0);
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error recording command buffer!");
    }
}

uint32_t HelloTriangleApp::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags)
{
    return FindMemoryType_Internal(typeFilter, memoryFlags);
}

VkShaderModule HelloTriangleApp::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(gfxCtx->logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating shader module!");
    }
    
    return shaderModule;
}

void HelloTriangleApp::MainLoop() 
{
    while (!glfwWindowShouldClose(window) && !inputHandler.WantToExit()) 
    {
        glfwPollEvents();
        inputHandler.ReactToEvents(*window);
        DrawFrame();
    }

    vkDeviceWaitIdle(gfxCtx->logicalDevice);
}

void HelloTriangleApp::UpdateUniformBuffers(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, 
        std::chrono::seconds::period>(startTime - currentTime).count();

    UniformBufferObject ubo{};
    //ubo.modelM = glm::rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0,1,0));
    ubo.modelM = glm::mat4(1.0f);
    glm::vec3 eyePos = inputHandler.GetPosition();
    ubo.viewPos = eyePos;
    ubo.viewM = glm::lookAt(eyePos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    ubo.projM = glm::perspective(glm::radians(45.0f), 
        swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 500.0f);
    ubo.projM[1][1] *= -1;
    ubo.debugUtil = inputHandler.IsDebugEnabled() ? 1:0;
    ubo.deltaTime = time;

    glm::vec3 lightDirection = glm::vec3(-0.32f, -0.77f, 0.56f);
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    float left = -10.0f, right = 10.0f, bottom = -10.0f, top = 10.0f, near = -50.0f, far = 50.0f;


    //Having problems creating the projection with Orthogonal
    glm::mat4 lightView = glm::lookAt(glm::vec3(0.32, 0.77, -0.56), target, up);
    glm::mat4 lightProjection = glm::ortho(left, right, bottom, top, near, far);
    lightProjection[1][1] *= -1;
    ubo.lightSpaceMatrix =  lightProjection * lightView;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void HelloTriangleApp::DrawFrame()
{
    vkWaitForFences(gfxCtx->logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(gfxCtx->logicalDevice, swapChain, UINT64_MAX,
    imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("Error acquiring swapchain image!");
    }

    vkResetFences(gfxCtx->logicalDevice, 1, &inFlightFences[currentFrame]);

    UpdateUniformBuffers(currentFrame);

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

#if COMPUTE_FEATURE
    //Need to add more sync for compute?
    VkSubmitInfo computeSubmitInfo{};
    computeSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    computeSubmitInfo.waitSemaphoreCount = 0;
    //computeSubmitInfo.pWaitSemaphores = waitSemaphores;
    computeSubmitInfo.pWaitDstStageMask = waitStages;
    computeSubmitInfo.commandBufferCount = 1;
    computeSubmitInfo.pCommandBuffers = &computeCommandBuffers[currentFrame];
    computeSubmitInfo.signalSemaphoreCount = 0;

    RecordComputeCommandBuffer(computeCommandBuffers[currentFrame]);

    if (vkQueueSubmit(computeQueue, 1, &computeSubmitInfo, nullptr) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error submitting compute command buffer");
    }
#endif//#if COMPUTE_FEATURE

    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame]};
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(gfxCtx->graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Error submitting draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapchains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(presentationQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
    {
        framebufferResized = false;
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("Error presenting swapchain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void HelloTriangleApp::RecreateSwapChain()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) 
    {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(gfxCtx->logicalDevice);

    //Clear swpachain resources
    CleanupSwapChain();
    
    //Recreate
    CreateSwapChain();
    CreateSwapChainImageViews();
    CreateShadowMapResources();
    CreateColorResources();
    CreateDepthResources();
    CreateShadowMapFramebuffers();
    CreateFramebuffers();
    UpdateDescriptorSets();
    //CreateDescriptorSets();
}

void HelloTriangleApp::CleanupSwapChain()
{
    vkDestroyImageView(gfxCtx->logicalDevice, dirShadowMapDepthImageView, nullptr);
    vkDestroyImage(gfxCtx->logicalDevice, dirShadowMapDepthImage, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, dirShadowMapDepthMemory, nullptr);

    vkDestroyImageView(gfxCtx->logicalDevice, depthImageView, nullptr);
    vkDestroyImage(gfxCtx->logicalDevice, depthImage, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, depthImageMemory, nullptr);

    vkDestroyImageView(gfxCtx->logicalDevice, colorImageView, nullptr);
    vkDestroyImage(gfxCtx->logicalDevice, colorImage, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, colorImageMemory, nullptr);

    for (VkFramebuffer framebuffer : swapchainFramebuffers)
    {
        vkDestroyFramebuffer(gfxCtx->logicalDevice, framebuffer, nullptr);
    }
    for (VkFramebuffer framebuffer : shadowMapFramebuffers)
    {
        vkDestroyFramebuffer(gfxCtx->logicalDevice, framebuffer, nullptr);
    }
    for (VkImageView imageView : swapChainImageViews)
    {
        vkDestroyImageView(gfxCtx->logicalDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(gfxCtx->logicalDevice, swapChain, nullptr);
}

void HelloTriangleApp::CleanupBuffers()
{
    for(GfxObject* object : objects)
    {
        vkDestroyBuffer(gfxCtx->logicalDevice, object->vertexBuffer, nullptr);
        vkFreeMemory(gfxCtx->logicalDevice, object->vertexBufferMemory, nullptr);
        vkDestroyBuffer(gfxCtx->logicalDevice, object->indexBuffer, nullptr);
        vkFreeMemory(gfxCtx->logicalDevice, object->indexBufferMemory, nullptr);
    }

    for (int i = 0; i < shaderStorageBuffers.size(); i++)
    {
        vkDestroyBuffer(gfxCtx->logicalDevice, shaderStorageBuffers[i], nullptr);
    }
    for (int i = 0; i < shaderStorageBuffersMemory.size(); i++)
    {
        vkFreeMemory(gfxCtx->logicalDevice, shaderStorageBuffersMemory[i], nullptr);
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) 
    {
        vkDestroyBuffer(gfxCtx->logicalDevice, uniformBuffers[i], nullptr);
        vkFreeMemory(gfxCtx->logicalDevice, uniformBuffersMemory[i], nullptr);
    }
}

void HelloTriangleApp::Cleanup() 
{
    CleanupSwapChain();

    vkDestroySampler(gfxCtx->logicalDevice, textureSampler, nullptr);
    vkDestroyImageView(gfxCtx->logicalDevice, textureImageView, nullptr);
    vkDestroyImage(gfxCtx->logicalDevice, textureImage, nullptr);
    vkFreeMemory(gfxCtx->logicalDevice, textureImageMemory, nullptr);

    CleanupBuffers();
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) 
    {
        vkDestroySemaphore(gfxCtx->logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(gfxCtx->logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(gfxCtx->logicalDevice, inFlightFences[i], nullptr);
    }
    
    vkDestroyCommandPool(gfxCtx->logicalDevice, gfxCtx->commandPool, nullptr);
    vkDestroyDescriptorPool(gfxCtx->logicalDevice, descriptorPool, nullptr);
    vkDestroyDescriptorPool(gfxCtx->logicalDevice, shadowMapDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(gfxCtx->logicalDevice, descriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(gfxCtx->logicalDevice, shadowMapDescriptorSetLayout, nullptr);
    vkDestroyPipeline(gfxCtx->logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(gfxCtx->logicalDevice, graphicsPipelineLayout, nullptr);
    vkDestroyPipeline(gfxCtx->logicalDevice, shadowMapPipeline, nullptr);
    vkDestroyPipelineLayout(gfxCtx->logicalDevice, shadowMapPipelineLayout, nullptr);
    vkDestroyRenderPass(gfxCtx->logicalDevice, renderPass, nullptr);
    vkDestroyRenderPass(gfxCtx->logicalDevice, shadowMapRenderPass, nullptr);

#if COMPUTE_FEATURE
    vkDestroyCommandPool(gfxCtx->logicalDevice, computeCommandPool, nullptr);
    vkDestroyDescriptorPool(gfxCtx->logicalDevice, computeDescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(gfxCtx->logicalDevice, computeDescriptorSetLayout, nullptr);
    vkDestroyPipeline(gfxCtx->logicalDevice, computePipeline, nullptr);
    vkDestroyPipelineLayout(gfxCtx->logicalDevice, computePipelineLayout, nullptr);
#endif//#if COMPUTE_FEATURE

    vkDestroyDevice(gfxCtx->logicalDevice, nullptr);
    if (enableValidationLayers) 
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}