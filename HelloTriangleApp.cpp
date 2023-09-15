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

    if (logDebug) 
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

    if (logDebug) 
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

    if(logDebug)
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

    return dProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
        dFeatures.geometryShader && FindQueueFamilies(requestedPhysicalDevice).IsComplete();
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

void HelloTriangleApp::CreateLogicalDevice()
{
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

    VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    deviceQueueCreateInfo.queueCount = 1;
    float queuePriority = 1.0f;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo logicalDeviceCreateInfo{};
    logicalDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    logicalDeviceCreateInfo.queueCreateInfoCount = 1;
    logicalDeviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;

    //TODO
    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    logicalDeviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
    logicalDeviceCreateInfo.enabledExtensionCount = 0;
    
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
}

void HelloTriangleApp::MainLoop() 
{
    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
    }
}

void HelloTriangleApp::Cleanup() 
{
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