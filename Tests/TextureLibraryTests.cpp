#include "../GfxTextureLibrary.h"
#include "../GfxMaterialLibrary.h"
#include "../GfxGeometryLibrary.h"
#include "../GfxGeometry.h"
#include "../GfxObject.h"
#include "../GfxContext.h"
#include "../GfxPipelineManager.h"
#include "../DebugUtils.h"
#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

static std::atomic<int> validationErrors{ 0 };

static void Require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

static void Check(VkResult result, const char* operation)
{
    if (result != VK_SUCCESS)
        throw std::runtime_error(std::string(operation) + ": " + std::to_string(result));
}

static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data, void*)
{
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        ++validationErrors;
    std::cerr << data->pMessage << '\n';
    return VK_FALSE;
}

// No window or swapchain: isolate resource tests from unrelated render-pass/VRS errors.
class TestDevice
{
public:
    GfxContext context{};
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;

    TestDevice()
    {
        gfxCtx = &context;
        try
        {
            VkApplicationInfo app{};
            app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            app.apiVersion = VK_API_VERSION_1_3;
            const char* layer = "VK_LAYER_KHRONOS_validation";
            const char* extension = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
            VkDebugUtilsMessengerCreateInfoEXT debug{};
            debug.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debug.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debug.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                | VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
            debug.pfnUserCallback = ValidationCallback;
            VkInstanceCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            info.pApplicationInfo = &app;
            info.enabledLayerCount = 1;
            info.ppEnabledLayerNames = &layer;
            info.enabledExtensionCount = 1;
            info.ppEnabledExtensionNames = &extension;
            info.pNext = &debug;
            Check(vkCreateInstance(&info, nullptr, &instance), "Creating test instance");
            const auto createDebug = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
            Require(createDebug != nullptr, "Debug utils entry point unavailable");
            Check(createDebug(instance, &debug, nullptr, &messenger), "Creating debug messenger");

            uint32_t count = 0;
            Check(vkEnumeratePhysicalDevices(instance, &count, nullptr), "Enumerating GPUs");
            std::vector<VkPhysicalDevice> devices(count);
            Check(vkEnumeratePhysicalDevices(instance, &count, devices.data()), "Enumerating GPUs");
            uint32_t queueFamily = 0;
            for (VkPhysicalDevice device : devices)
            {
                uint32_t queueCount = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, nullptr);
                std::vector<VkQueueFamilyProperties> queues(queueCount);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueCount, queues.data());
                for (uint32_t i = 0; i < queueCount; ++i)
                    if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    {
                        context.physicalDevice = device;
                        queueFamily = i;
                        break;
                    }
                if (context.physicalDevice != VK_NULL_HANDLE)
                    break;
            }
            Require(context.physicalDevice != VK_NULL_HANDLE, "No graphics queue available");
            const float priority = 1.0f;
            VkDeviceQueueCreateInfo queue{};
            queue.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue.queueFamilyIndex = queueFamily;
            queue.queueCount = 1;
            queue.pQueuePriorities = &priority;
            VkDeviceCreateInfo deviceInfo{};
            deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            deviceInfo.queueCreateInfoCount = 1;
            deviceInfo.pQueueCreateInfos = &queue;
            // Deliberately enable no optional features, including samplerAnisotropy.
            Check(vkCreateDevice(context.physicalDevice, &deviceInfo, nullptr, &context.logicalDevice), "Creating test device");
            vkGetDeviceQueue(context.logicalDevice, queueFamily, 0, &context.graphicsQueue);
            VkCommandPoolCreateInfo pool{};
            pool.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            pool.queueFamilyIndex = queueFamily;
            Check(vkCreateCommandPool(context.logicalDevice, &pool, nullptr, &context.commandPool), "Creating command pool");
            DebugUtils::getInstance().Init();
        }
        catch (...)
        {
            Destroy();
            throw;
        }
    }

    ~TestDevice() { Destroy(); }

private:
    void Destroy()
    {
        if (context.logicalDevice != VK_NULL_HANDLE)
        {
            vkDeviceWaitIdle(context.logicalDevice);
            vkDestroyCommandPool(context.logicalDevice, context.commandPool, nullptr);
            vkDestroyDevice(context.logicalDevice, nullptr);
            context.logicalDevice = VK_NULL_HANDLE;
        }
        if (messenger != VK_NULL_HANDLE)
        {
            const auto destroyDebug = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
            destroyDebug(instance, messenger, nullptr);
            messenger = VK_NULL_HANDLE;
        }
        if (instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(instance, nullptr);
            instance = VK_NULL_HANDLE;
        }
        gfxCtx = nullptr;
    }
};

// Fixtures are generated in a unique test-owned temporary directory.
class Fixtures
{
public:
    std::filesystem::path directory;
    Fixtures()
    {
        directory = std::filesystem::temp_directory_path() / ("AEngineTextureTests-"
            + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        Require(std::filesystem::create_directory(directory), "Could not create fixture directory");
        Write("one.tga", 1, 1);
        Write("npot.tga", 7, 3);
        std::ofstream(directory / "invalid.tga") << "not an image";
    }
    ~Fixtures()
    {
        // Remove only the exact fixtures created here; never recursively delete a directory.
        std::error_code ignored;
        for (const char* file : { "one.tga", "npot.tga", "invalid.tga" })
            std::filesystem::remove(directory / file, ignored);
        std::filesystem::remove(directory, ignored);
    }
private:
    void Write(const char* name, uint16_t width, uint16_t height)
    {
        std::array<unsigned char, 18> header{};
        header[2] = 2; // Uncompressed true-color TGA.
        header[12] = static_cast<unsigned char>(width);
        header[14] = static_cast<unsigned char>(height);
        header[16] = 24;
        std::ofstream file(directory / name, std::ios::binary);
        file.write(reinterpret_cast<const char*>(header.data()), header.size());
        const unsigned char bgr[] = { 192, 128, 64 };
        for (uint32_t i = 0; i < static_cast<uint32_t>(width) * height; ++i)
            file.write(reinterpret_cast<const char*>(bgr), sizeof(bgr));
        Require(file.good(), "Could not write fixture");
    }
};

static void CheckLastMip(const GfxTexture& texture, bool white = false)
{
    struct Readback
    {
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        ~Readback()
        {
            vkDestroyBuffer(gfxCtx->logicalDevice, buffer, nullptr);
            vkFreeMemory(gfxCtx->logicalDevice, memory, nullptr);
        }
    } readback;
    CreateBuffer_Internal(4, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        readback.buffer, readback.memory);
    const VkCommandBuffer commands = BeginSingleTimeCommandBuffer_Internal();
    VkImageMemoryBarrier imageBarrier{};
    imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imageBarrier.image = texture.GetImage();
    imageBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, texture.GetMipLevels() - 1, 1, 0, 1 };
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    VkBufferImageCopy copy{};
    copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, texture.GetMipLevels() - 1, 0, 1 };
    copy.imageExtent = { 1, 1, 1 };
    vkCmdCopyImageToBuffer(commands, texture.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, readback.buffer, 1, &copy);
    imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &imageBarrier);
    VkBufferMemoryBarrier hostBarrier{};
    hostBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    hostBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    hostBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    hostBarrier.buffer = readback.buffer;
    hostBarrier.size = VK_WHOLE_SIZE;
    hostBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    hostBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    vkCmdPipelineBarrier(commands, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT,
        0, 0, nullptr, 1, &hostBarrier, 0, nullptr);
    EndSingleTimeCommandBuffer_Internal(commands);
    void* pixels = nullptr;
    Check(vkMapMemory(gfxCtx->logicalDevice, readback.memory, 0, 4, 0, &pixels), "Mapping readback");
    const auto* rgba = static_cast<const unsigned char*>(pixels);
    const bool matches = rgba[0] == (white ? 255 : 64) && rgba[1] == (white ? 255 : 128)
        && rgba[2] == (white ? 255 : 192) && rgba[3] == 255;
    vkUnmapMemory(gfxCtx->logicalDevice, readback.memory);
    Require(matches, "GPU mip readback did not match the constant fixture pixels");
}

static void RunTests()
{
    TestDevice device;
    Fixtures fixtures;
    GfxTextureLibrary library;
    {
        GfxGeometryLibrary geometries;
        const GfxGeometry* room = geometries.LoadObj("Models/viking_room.obj");
        Require(room == geometries.LoadObj("Models/../Models/viking_room.obj"), "OBJ aliases did not share geometry");
        Require(room == geometries.LoadObj(std::filesystem::absolute("Models/viking_room.obj").string()), "Absolute OBJ path did not share geometry");
        Require(room->vertexBuffer != VK_NULL_HANDLE && room->indexBuffer != VK_NULL_HANDLE
            && room->indexCount == room->indices.size(), "OBJ geometry was not uploaded correctly");
        const GfxGeometry* unflipped = geometries.LoadObj("Models/viking_room.obj", false);
        Require(unflipped != room && geometries.GetGeometryCount() == 2, "OBJ import options were not part of the cache key");
        Require(room->boundsMax.x > room->boundsMin.x && room->boundsMax.y > room->boundsMin.y
            && room->boundsMax.z > room->boundsMin.z, "Imported geometry bounds are invalid");
        bool rejected = false;
        try { geometries.LoadObj("Models/__missing_test_model.obj"); }
        catch (const std::exception&) { rejected = true; }
        Require(rejected && geometries.GetGeometryCount() == 2, "Failed OBJ load polluted the cache");
        const GfxGeometry* cube = geometries.GetOrCreateCube();
        Require(cube == geometries.GetOrCreateCube() && cube != room, "Procedural and imported cache keys collided");
        Check(vkDeviceWaitIdle(device.context.logicalDevice), "Waiting before geometry cleanup");
        geometries.Clear();
        geometries.Clear();
        Require(geometries.LoadObj("Models/viking_room.obj") != nullptr, "OBJ reload after Clear failed");
        geometries.Clear();
    }
    const GfxTexture* texture = library.Load("Textures/viking_room.png");
    Require(texture == library.Load("Textures/../Textures/viking_room.png"), "Normalized paths did not share a texture");
    Require(texture == library.Load(std::filesystem::absolute("Textures/viking_room.png").string()), "Absolute path did not share a texture");
    Require(texture->GetFormat() == VK_FORMAT_R8G8B8A8_SRGB, "Color texture must use sRGB");
    Require(texture->GetImageView() != VK_NULL_HANDLE && texture->GetSampler() != VK_NULL_HANDLE,
        "Loaded texture did not have a view/sampler");
    const GfxTexture* linear = library.Load("Textures/viking_room.png", GfxTextureColorSpace::Linear);
    Require(linear != texture && linear->GetFormat() == VK_FORMAT_R8G8B8A8_UNORM, "Color space was not part of the cache key");
    for (const std::string& path : { std::string(), (fixtures.directory / "missing.png").string(),
        (fixtures.directory / "invalid.tga").string() })
    {
        bool rejected = false;
        try { library.Load(path); }
        catch (const std::exception&) { rejected = true; }
        Require(rejected && library.GetLoadedTextureCount() == 2, "Failed load polluted the cache");
    }
    const GfxTexture* one = library.Load((fixtures.directory / "one.tga").string(), GfxTextureColorSpace::Linear);
    Require(one->GetWidth() == 1 && one->GetHeight() == 1 && one->GetMipLevels() == 1, "Incorrect 1x1 mip count");
    CheckLastMip(*one);
    const GfxTexture* npot = library.Load((fixtures.directory / "npot.tga").string(), GfxTextureColorSpace::Linear);
    Require(npot->GetWidth() == 7 && npot->GetHeight() == 3 && npot->GetMipLevels() == 3, "Incorrect NPOT mip chain");
    CheckLastMip(*npot);
    {
        GfxMaterialLibrary materials;
        materials.Initialize(device.context.logicalDevice, 2);
        const GfxMaterial* first = materials.Create("first", one);
        Require(first == materials.Create("first", one), "Repeated material definition was not shared");
        const GfxMaterial* second = materials.Create("second", npot);
        Require(first->GetDescriptorSet() != second->GetDescriptorSet(), "Distinct materials shared a descriptor set");
        Require(first->GetBaseColorTexture() == one && second->GetBaseColorTexture() == npot,
            "Materials did not reference their assigned textures");
        bool conflictRejected = false;
        try { materials.Create("first", npot); }
        catch (const std::exception&) { conflictRejected = true; }
        Require(conflictRejected, "Conflicting material definition was silently accepted");
        bool overflowRejected = false;
        try { materials.Create("overflow", one); }
        catch (const std::exception&) { overflowRejected = true; }
        Require(overflowRejected && materials.GetMaterialCount() == 2, "Material capacity limit was not respected");
        Check(vkDeviceWaitIdle(device.context.logicalDevice), "Waiting before clearing materials");
        materials.Clear();
        materials.Clear();
        materials.Initialize(device.context.logicalDevice);
        Require(materials.Create("reloaded", one) != nullptr, "Material library reinitialization failed");
        // Material sets are destroyed before the texture resources they reference.
        materials.Clear();
    }
    {
        GfxGeometryLibrary geometries;
        GfxMaterialLibrary materials;
        materials.Initialize(device.context.logicalDevice);
        const auto* fallback = materials.Create("fallback", texture);
        const auto* panels = geometries.LoadObj("Models/material_test.obj");
        const auto slots = materials.CreateObjMaterials(*panels, library, fallback);
        Require(slots.size() == 4 && slots[0] == fallback, "Imported fallback slot mismatch");
        Require(slots[1]->GetBaseColorTexture() == library.GetWhite()
            && slots[2]->GetBaseColorTexture() == library.GetWhite(), "Untextured materials did not share white");
        Require(slots[1]->GetBaseColorFactor() == glm::vec4(0.9f, 0.05f, 0.05f, 1.0f), "Imported Kd mismatch");
        Require(slots[3]->GetBaseColorTexture() == library.Load("Textures/texture.jpg"), "MTL texture not shared");
        Require(slots == materials.CreateObjMaterials(*panels, library, fallback), "OBJ materials were not cached");
        CheckLastMip(*library.GetWhite(), true);
        GfxObject first(VK_NULL_HANDLE, VK_NULL_HANDLE);
        GfxObject second(VK_NULL_HANDLE, VK_NULL_HANDLE);
        first.geometry = second.geometry = panels;
        first.material = second.material = fallback;
        first.materials = second.materials = slots;
        second.materials[1] = slots[2];
        Require(first.GetMaterial(1) == slots[1] && second.GetMaterial(1) == slots[2]
            && second.GetMaterial(999) == fallback, "Instance material override or fallback failed");
        bool conflictRejected = false;
        try { materials.Create(slots[1]->GetName(), library.GetWhite(), glm::vec4(1.0f)); }
        catch (const std::exception&) { conflictRejected = true; }
        Require(conflictRejected, "Conflicting material color was accepted");
        Check(vkDeviceWaitIdle(device.context.logicalDevice), "Waiting before imported material cleanup");
        materials.Clear();
        geometries.Clear();
    }
    Check(vkDeviceWaitIdle(device.context.logicalDevice), "Waiting before clearing textures");
    library.Clear();
    library.Clear();
    Require(library.GetLoadedTextureCount() == 0, "Clear did not release cached textures");
    Require(library.Load("Textures/viking_room.png") != nullptr, "Reload after Clear failed");
    library.Clear();
}

int main()
{
    try
    {
        RunTests();
        Require(validationErrors == 0, "Vulkan validation reported errors");
        std::cout << "PASS: OBJ cache/upload, MTL materials, white texture readback, instance overrides, descriptors/capacity, clear/reload; zero validation errors.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
