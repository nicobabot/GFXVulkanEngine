#include "HelloTriangleApp.h"


void HelloTriangleApp::Run()
{
    InitWindow();
    InitVulkan();
    MainLoop();
    Cleanup();
}

void HelloTriangleApp::InitWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, nullptr, nullptr);
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
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateCommandBuffer();
    CreateSyncObjects();
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

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

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

void HelloTriangleApp::CreateSwapChainImageViews()
{
    uint32_t swapchainImageCount = swapChainImages.size();
    swapChainImageViews.resize(swapchainImageCount);
    for (int i = 0; i < swapchainImageCount; ++i) 
    {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapChainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = swapChainImageFormat;

        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        
        if (vkCreateImageView(logicalDevice, &imageViewCreateInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("Error creating image view");
        }
    }

}

void HelloTriangleApp::CreateRenderPass()
{
    VkAttachmentDescription colorAttatchmentDescr{};
    colorAttatchmentDescr.format = swapChainImageFormat;
    colorAttatchmentDescr.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttatchmentDescr.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; 
    colorAttatchmentDescr.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttatchmentDescr.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttatchmentDescr.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttatchmentDescr.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttatchmentDescr.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachment{};
    colorAttachment.attachment = 0;
    colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescr{};
    subpassDescr.colorAttachmentCount = 1 ;
    subpassDescr.pColorAttachments = &colorAttachment;

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &colorAttatchmentDescr;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescr;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    if (vkCreateRenderPass(logicalDevice, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating renderpass!");
    }
}

void HelloTriangleApp::CreateGraphicsPipeline()
{
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

    VkPipelineShaderStageCreateInfo shaderStages[] {vertexPipelineCreateInfo, fragmentPipelineCreateInfo};

    //TODO VERTEX INFO (now hardcoded in shader)
    VkPipelineVertexInputStateCreateInfo vertexStateCreateInfo{};
    vertexStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexStateCreateInfo.pVertexBindingDescriptions = nullptr;
    vertexStateCreateInfo.vertexBindingDescriptionCount = 0;
    vertexStateCreateInfo.pVertexAttributeDescriptions = nullptr;
    vertexStateCreateInfo.vertexAttributeDescriptionCount = 0;

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
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //Useful for shadowmaps
    rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationStateCreateInfo.depthBiasConstantFactor= 0.0f;
    rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    //TODO
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleStateCreateInfo.minSampleShading = 1.0f; // Optional
    multisampleStateCreateInfo.pSampleMask = nullptr; // Optional
    multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE; // Optional

    //TODO VkPipelineDepthStencilStateCreateInfo

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
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    
    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error creating pipeline layuout!");
    }


    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = 2;
    graphicsPipelineCreateInfo.pStages = shaderStages;

    graphicsPipelineCreateInfo.pVertexInputState = &vertexStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = nullptr; //TODO
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


    vkDestroyShaderModule(logicalDevice, vertexShaderModule, nullptr);
    vkDestroyShaderModule(logicalDevice, fragmentShaderModule, nullptr);
}

void HelloTriangleApp::CreateFramebuffers()
{
    swapchainFramebuffers.resize(swapChainImageViews.size());
    for (int i = 0; i < swapChainImageViews.size(); ++i) 
    {
        VkImageView attachments[]
        {
            swapChainImageViews[i],
        };

        VkFramebufferCreateInfo framebufferCreateInfo{};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = attachments;
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

void HelloTriangleApp::CreateCommandBuffer()
{   
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    if(vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocateInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating command buffer!");
    }

}

void HelloTriangleApp::CreateSyncObjects()
{

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(logicalDevice, &fenceCreateInfo, nullptr, &inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Error creating sync objects!");
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
    VkClearValue clearColor = {{{0.0f,0.0f,0.0f,1.0f}}};
    renderPassBeginInfo.clearValueCount = 1;
    renderPassBeginInfo.pClearValues = &clearColor;

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

    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
    {
        throw std::runtime_error("Error recording command buffer!");
    }
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
    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        DrawFrame();
    }
}

void HelloTriangleApp::DrawFrame()
{
    vkWaitForFences(logicalDevice, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    vkResetFences(logicalDevice, 1, &inFlightFence);

    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
    vkResetCommandBuffer(commandBuffer, 0);
    RecordCommandBuffer(commandBuffer, imageIndex);

    VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
    VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Error submiting draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = waitSemaphores;
    VkSwapchainKHR swapchains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(presentationQueue, &presentInfo);
}

void HelloTriangleApp::Cleanup() 
{
    vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, nullptr);
    vkDestroySemaphore(logicalDevice, renderFinishedSemaphore, nullptr);
    vkDestroyFence(logicalDevice, inFlightFence, nullptr);
    vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
    for (VkFramebuffer framebuffer : swapchainFramebuffers) 
    {
        vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);
    }
    vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);
    vkDestroyRenderPass(logicalDevice, renderPass, nullptr);
    for (VkImageView imageView : swapChainImageViews) 
    {
        vkDestroyImageView(logicalDevice, imageView, nullptr);
    }
    vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);
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