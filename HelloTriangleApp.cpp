#include "HelloTriangleApp.h"
#include "MainDefines.h"

void HelloTriangleApp::Run()
{
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
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    modelLoader.LoadModel();
    CreateVertexBuffers();
    CreateIndexBuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
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

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << "\n" << std::endl;

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
    createInfo.pfnUserCallback = debugCallback;
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
    vkGetPhysicalDeviceProperties(physicalDevice, &prop);
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
            physicalDevice = pDevice;
            msaaSamples = GetMaxUsableSampleCount();
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) 
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
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);
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

    if (vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating logical device");
    }
}

void HelloTriangleApp::GetLogicalDeviceQueues()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.presentationFamily.value(), 0, &presentationQueue);
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphicsAndComputeFamily.value(), 0, &computeQueue);
}

void HelloTriangleApp::CreateSwapChain()
{
    SwapChainSupportDetails swapChainDetails = QuerySwapChainSupport(physicalDevice);

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

    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);
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

    if (vkCreateSwapchainKHR(logicalDevice, &createSwapChainInfo, nullptr, &swapChain) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating SwapChain");
    }

    uint32_t swapchainImagesCount = 0;
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapchainImagesCount, nullptr);
    swapChainImages.resize(swapchainImagesCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapChain, &swapchainImagesCount, swapChainImages.data());
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

    if (vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &newImageView) != VK_SUCCESS)
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

    if (vkCreateRenderPass(logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating renderpass!");
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
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
    VkDescriptorSetLayoutCreateInfo descriptorSetCreateInfo{};
    descriptorSetCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetCreateInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    descriptorSetCreateInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logicalDevice, &descriptorSetCreateInfo,
        nullptr, &descriptorSetLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating descriptor set layout!");
    }
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
    vertexPipelineCreateInfo.pName = "main";
    vertexPipelineCreateInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragmentPipelineCreateInfo{};
    fragmentPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentPipelineCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentPipelineCreateInfo.module = fragmentShaderModule;
    fragmentPipelineCreateInfo.pName = "main";
    fragmentPipelineCreateInfo.pSpecializationInfo = nullptr;

    std::array<VkPipelineShaderStageCreateInfo, 2>  shaderStages {vertexPipelineCreateInfo,
        fragmentPipelineCreateInfo};
    
    VkVertexInputBindingDescription vertexBindingDescription = Vertex::GetBindingDesctiption();
    std::array<VkVertexInputAttributeDescription, 4>
        vertexAttributeDescription = Vertex::GetAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo{};
    vertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexStateCreateInfo.vertexBindingDescriptionCount = 1;
    vertexStateCreateInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(vertexAttributeDescription.size());
    vertexStateCreateInfo.pVertexBindingDescriptions = &vertexBindingDescription;
    vertexStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescription.data();
    

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{};
    scissor.offset = { 0,0 };
    scissor.extent = swapChainExtent;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.viewportCount = 1;

    std::vector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();


    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
    rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCreateInfo.lineWidth = 1.0f;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    //Counter-Clockwise -> Y flip on projection matrix
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    //Useful for shadowmaps
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor= 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
    multisampleStateCreateInfo.rasterizationSamples = msaaSamples;
    multisampleStateCreateInfo.minSampleShading = .2f; //min fraction for sample shading;closer to one is smoother
    multisampleStateCreateInfo.pSampleMask = nullptr; // Optional
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencilStateAttachment{};
    depthStencilStateAttachment.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateAttachment.depthTestEnable = VK_TRUE;
    //depthStencilStateAttachment.stencilTestEnable = VK_TRUE; -> WRONG!
    depthStencilStateAttachment.depthWriteEnable = VK_TRUE;
    depthStencilStateAttachment.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilStateAttachment.depthBoundsTestEnable = VK_FALSE;
    depthStencilStateAttachment.minDepthBounds = 0.0f;
    depthStencilStateAttachment.maxDepthBounds = 1.0f;

    depthStencilStateAttachment.stencilTestEnable = VK_FALSE;
    depthStencilStateAttachment.front = {};
    depthStencilStateAttachment.back = {};


    VkPipelineColorBlendAttachmentState colorBlendStateAttachment{};
    colorBlendStateAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendStateAttachment.blendEnable = VK_TRUE;
    colorBlendStateAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendStateAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendStateAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    //finalColor.rgb = newAlpha * newColor + (1 - newAlpha) * oldColor;
    colorBlendStateAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendStateAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendStateAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    //finalColor.a = newAlpha.a;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
    colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendStateAttachment;
    colorBlendStateCreateInfo.blendConstants[0] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[1] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[2] = 0.0f; // Optional
    colorBlendStateCreateInfo.blendConstants[3] = 0.0f; // Optional

    //TODO
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating pipeline layuout!");
    }

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = shaderStages.size();
    graphicsPipelineCreateInfo.pStages = shaderStages.data();

    graphicsPipelineCreateInfo.pVertexInputState = &vertexStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateAttachment;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicState;

    graphicsPipelineCreateInfo.layout = pipelineLayout;

    graphicsPipelineCreateInfo.renderPass = renderPass;
    graphicsPipelineCreateInfo.subpass = 0;

    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    graphicsPipelineCreateInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1,
        &graphicsPipelineCreateInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)     
    {
        throw std::runtime_error("Error creating graphic pipeline!");
    }

    std::vector<char> computeShader = ReadFile("CompiledShaders/compute.spv");
    VkShaderModule computeShaderModule = CreateShaderModule(computeShader);

    VkPipelineShaderStageCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    computePipelineCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    computePipelineCreateInfo.module = computeShaderModule;
    computePipelineCreateInfo.pName = "main";

    vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
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

        if (vkCreateFramebuffer(logicalDevice, &framebufferCreateInfo, nullptr, &swapchainFramebuffers[i]) != VK_SUCCESS) 
        {   
            throw std::runtime_error("Error creating framebuffer!");
        }
    }
}

void HelloTriangleApp::CreateCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

    VkCommandPoolCreateInfo commandoPoolCreateInfo{};
    commandoPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandoPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandoPoolCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(logicalDevice, &commandoPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating command pool!");
    }

}

VkFormat HelloTriangleApp::FindSupportedFormat(std::vector<VkFormat> candidates, 
    VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) 
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

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

VkCommandBuffer HelloTriangleApp::BeginSingleTimeCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    //TODO: Create a separate commandPool (VK_COMMAND_POOL_CREATE_TRANSIENT_BIT) for this short living buffers
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo commandBufferBeginInfo{};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    return commandBuffer;
}

void HelloTriangleApp::EndSingleTimeCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.commandBufferCount = 1;
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
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
    
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
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

    if (vkCreateImage(logicalDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating image!");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(logicalDevice, image, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.allocationSize = memoryRequirements.size;
    memoryAllocateInfo.memoryTypeIndex = FindMemoryType(memoryRequirements.memoryTypeBits,
        properties);

    if (vkAllocateMemory(logicalDevice, &memoryAllocateInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating memory for created image!");
    }

    vkBindImageMemory(logicalDevice, image, imageMemory, 0);
}

void HelloTriangleApp::CreateTextureImage()
{
    int texWidth, texHeight, texChannels;
    unsigned char* pixels = modelLoader.LoadTexture(&texWidth, &texHeight, &texChannels);

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
    vkMapMemory(logicalDevice, staginBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(logicalDevice, staginBufferMemory);

    modelLoader.FreeTextureArrayInfo(pixels);

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
    /*TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);*/
    GenerateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);

    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, staginBufferMemory, nullptr);
}

void HelloTriangleApp::GenerateMipmaps(VkImage image, VkFormat format, uint32_t texWidth, uint32_t texHeight, uint32_t mipLevels)
{
    VkFormatProperties formatProp{};
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProp);

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
    vkGetPhysicalDeviceFeatures(physicalDevice, &dFeatures);
    samplerCreateInfo.anisotropyEnable = dFeatures.samplerAnisotropy ? VK_TRUE : VK_FALSE;
    
    //TODO: store phys device properties in global variable instead of query here
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

    samplerCreateInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;

    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.maxLod = static_cast<float>(mipLevels);
    samplerCreateInfo.minLod = 0.0f;

    if (vkCreateSampler(logicalDevice, &samplerCreateInfo, nullptr, &textureSampler) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating sampler!");
    }

}

void HelloTriangleApp::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usageFlags, 
    VkMemoryPropertyFlags memoryFlags, VkBuffer& newBuffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo createBuffer{};
    createBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createBuffer.size = size;
    createBuffer.usage = usageFlags;
    createBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(logicalDevice, &createBuffer, nullptr, &newBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating vertex buffer!");
    }

    VkMemoryRequirements bufferMemoryRequirements;
    vkGetBufferMemoryRequirements(logicalDevice, newBuffer, &bufferMemoryRequirements);

    //FindMemoryType
    VkMemoryAllocateInfo allocateMemory{};
    allocateMemory.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateMemory.allocationSize = bufferMemoryRequirements.size;
    allocateMemory.memoryTypeIndex = FindMemoryType(bufferMemoryRequirements.memoryTypeBits,
        memoryFlags);

    if (vkAllocateMemory(logicalDevice, &allocateMemory, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating vertex buffer memory");
    }

    vkBindBufferMemory(logicalDevice, newBuffer, bufferMemory, 0);
}

void HelloTriangleApp::CreateVertexBuffers()
{
    std::vector<Vertex> modelVertices = modelLoader.vertices;
    VkDeviceSize bufferSize = sizeof(modelVertices[0]) * modelVertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0,  &data);
    memcpy(data, modelVertices.data(), (size_t)bufferSize);
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,vertexBuffer, vertexBufferMemory);

    CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);    
    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
}

void HelloTriangleApp::CreateIndexBuffers()
{
    std::vector<uint32_t> modelIndices = modelLoader.indices; 
    VkDeviceSize bufferSize = sizeof(modelIndices[0]) * modelIndices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);
    
    void*data;
    vkMapMemory(logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, modelIndices.data(), bufferSize);
    vkUnmapMemory(logicalDevice, stagingBufferMemory);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    CopyBuffer(stagingBuffer, indexBuffer, bufferSize);
    vkDestroyBuffer(logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(logicalDevice, stagingBufferMemory, nullptr);
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

        vkMapMemory(logicalDevice, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }

}

void HelloTriangleApp::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSize;
    descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorPoolSize[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorPoolSize[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSize.size());
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize.data();
    descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(logicalDevice, &descriptorPoolCreateInfo,
        nullptr, &descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating descriptor pool!");
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
    if (vkAllocateDescriptorSets(logicalDevice, &descriptorSetAllocateInfo,
        descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error allocating descriptor sets!");
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageView = textureImageView;
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler = textureSampler;

        std::array<VkWriteDescriptorSet, 2> writeDescriptorSet{};
        writeDescriptorSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[0].dstSet = descriptorSets[i];
        writeDescriptorSet[0].dstBinding = 0;
        writeDescriptorSet[0].dstArrayElement = 0;
        writeDescriptorSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptorSet[0].descriptorCount = 1;
        writeDescriptorSet[0].pBufferInfo = &bufferInfo;

        writeDescriptorSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet[1].dstSet = descriptorSets[i];
        writeDescriptorSet[1].dstBinding = 1;
        writeDescriptorSet[1].dstArrayElement = 0;
        writeDescriptorSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet[1].descriptorCount = 1;
        writeDescriptorSet[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(logicalDevice, static_cast<uint32_t>(writeDescriptorSet.size()),
            writeDescriptorSet.data(), 0, nullptr);
    }

}

void HelloTriangleApp::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
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
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if(vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating command buffer!");
    }
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
        if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Error creating sync objects!");
        }
    }
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

    VkRenderPassBeginInfo renderPassBeginInfo{};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapchainFramebuffers[imageIndex];
    renderPassBeginInfo.renderArea.offset = {0,0};
    renderPassBeginInfo.renderArea.extent = swapChainExtent;

    std::array<VkClearValue, 3> clearValues{};
    clearValues[0].color = {0.0f,0.0f,0.0f,1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    clearValues[2].color = { 0.0f,0.0f,0.0f,1.0f };
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassBeginInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

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

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(modelLoader.indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error recording command buffer!");
    }
}

uint32_t HelloTriangleApp::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags memoryFlags)
{
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

    for (int i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; ++i) 
    {
        if(typeFilter & (1 << i) && memoryFlags && 
            (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & memoryFlags) == memoryFlags)
        {
            return i;
        }
    }
    
    throw std::runtime_error("Error finding memory type");
}

VkShaderModule HelloTriangleApp::CreateShaderModule(const std::vector<char>& code)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) 
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

    vkDeviceWaitIdle(logicalDevice);
}

void HelloTriangleApp::UpdateUniformBuffers(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, 
        std::chrono::seconds::period>(startTime - currentTime).count();

    time = 1;

    UniformBufferObject ubo{};
    ubo.modelM = glm::rotate(glm::mat4(1.0f), time * glm::radians(0.0f), glm::vec3(0,0,1));
    glm::vec3 eyePos = inputHandler.GetPosition();
    ubo.viewPos = eyePos;
    ubo.viewM = glm::lookAt(eyePos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    ubo.projM = glm::perspective(glm::radians(45.0f), 
        swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 500.0f);
    ubo.projM[1][1] *= -1;
    ubo.debugUtil = inputHandler.IsDebugEnabled() ? 1:0;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void HelloTriangleApp::DrawFrame()
{
    vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX,
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

    vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);

    UpdateUniformBuffers(currentFrame);
    
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);
    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
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

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("Error submiting draw command buffer!");
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
    
    vkDeviceWaitIdle(logicalDevice);

    //Clear swpachain resources
    CleanupSwapChain();
    
    //Recreate
    CreateSwapChain();
    CreateSwapChainImageViews();
    CreateColorResources();
    CreateDepthResources();
    CreateFramebuffers();
}

void HelloTriangleApp::CleanupSwapChain()
{
    vkDestroyImageView(logicalDevice, depthImageView, nullptr);
    vkDestroyImage(logicalDevice, depthImage, nullptr);
    vkFreeMemory(logicalDevice, depthImageMemory, nullptr);

    vkDestroyImageView(logicalDevice, colorImageView, nullptr);
    vkDestroyImage(logicalDevice, colorImage, nullptr);
    vkFreeMemory(logicalDevice, colorImageMemory, nullptr);

    for (VkFramebuffer framebuffer : swapchainFramebuffers)
    {
        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
    }
    for (VkImageView imageView : swapChainImageViews)
    {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
}

void HelloTriangleApp::CleanupBuffers()
{
    vkDestroyBuffer(logicalDevice, vertexBuffer, nullptr);
    vkFreeMemory(logicalDevice, vertexBufferMemory, nullptr);
    vkDestroyBuffer(logicalDevice, indexBuffer, nullptr);
    vkFreeMemory(logicalDevice, indexBufferMemory, nullptr);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) 
    {
        vkDestroyBuffer(logicalDevice, uniformBuffers[i], nullptr);
        vkFreeMemory(logicalDevice, uniformBuffersMemory[i], nullptr);
    }
}

void HelloTriangleApp::Cleanup() 
{
    CleanupSwapChain();

    vkDestroySampler(logicalDevice, textureSampler, nullptr);
    vkDestroyImageView(logicalDevice, textureImageView, nullptr);
    vkDestroyImage(logicalDevice, textureImage, nullptr);
    vkFreeMemory(logicalDevice, textureImageMemory, nullptr);

    CleanupBuffers();
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) 
    {
        vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
    }
    
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);
    vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
    vkDestroyDevice(logicalDevice, nullptr);
    if (enableValidationLayers) 
    {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}