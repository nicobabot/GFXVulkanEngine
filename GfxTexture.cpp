#include "GfxTexture.h"
#include "GfxContext.h"
#include "GfxPipelineManager.h"
#include "DebugUtils.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>

static void CheckTextureResult(VkResult result, const std::string& operation)
{
    if (result != VK_SUCCESS)
        throw std::runtime_error(operation + " (VkResult " + std::to_string(result) + ")");
}

GfxTexture::GfxTexture(const std::string& path, GfxTextureColorSpace colorSpace)
    : path(path), format(colorSpace == GfxTextureColorSpace::SRGB
        ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM)
{
    try
    {
        Load();
    }
    catch (...)
    {
        // A throwing constructor does not call this object's destructor.
        Destroy();
        throw;
    }
}

GfxTexture::~GfxTexture()
{
    Destroy();
}

GfxTexture::GfxTexture() : path("builtin:white"), format(VK_FORMAT_R8G8B8A8_SRGB)
{
    if (gfxCtx == nullptr || gfxCtx->logicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("White texture requires an initialized Vulkan device");
    device = gfxCtx->logicalDevice;
    width = height = 1;
    try
    {
        const unsigned char pixels[] = { 255, 255, 255, 255 };
        CreateImage();
        UploadPixels(pixels, sizeof(pixels));
        CreateViewAndSampler();
    }
    catch (...) { Destroy(); throw; }
}

void GfxTexture::Destroy()
{
    if (device == VK_NULL_HANDLE)
        return;
    vkDestroySampler(device, sampler, nullptr);
    vkDestroyImageView(device, imageView, nullptr);
    vkDestroyImage(device, image, nullptr);
    vkFreeMemory(device, memory, nullptr);
    sampler = VK_NULL_HANDLE;
    imageView = VK_NULL_HANDLE;
    image = VK_NULL_HANDLE;
    memory = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
}

void GfxTexture::Load()
{
    int decodedWidth = 0, decodedHeight = 0, channels = 0;
    std::unique_ptr<unsigned char, decltype(&stbi_image_free)> pixels(
        stbi_load(path.c_str(), &decodedWidth, &decodedHeight, &channels, STBI_rgb_alpha),
        &stbi_image_free);
    if (!pixels || decodedWidth <= 0 || decodedHeight <= 0)
    {
        const char* reason = stbi_failure_reason();
        throw std::runtime_error("Could not load texture '" + path + "': "
            + (reason != nullptr ? reason : "invalid image dimensions"));
    }
    if (gfxCtx == nullptr || gfxCtx->logicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("Texture upload requires an initialized Vulkan device: " + path);
    device = gfxCtx->logicalDevice;
    width = static_cast<uint32_t>(decodedWidth);
    height = static_cast<uint32_t>(decodedHeight);

    // Integer halving handles both power-of-two and rectangular/NPOT images.
    for (uint32_t dimension = (std::max)(width, height); dimension > 1; dimension /= 2)
        ++mipLevels;

    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(gfxCtx->physicalDevice, format, &properties);
    const VkFormatFeatureFlags sampling = VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT
        | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    if ((properties.optimalTilingFeatures & sampling) != sampling)
        throw std::runtime_error("Texture format does not support linear sampling: " + path);
    const VkFormatFeatureFlags blitting = VK_FORMAT_FEATURE_BLIT_SRC_BIT
        | VK_FORMAT_FEATURE_BLIT_DST_BIT | VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT;
    if (mipLevels > 1 && (properties.optimalTilingFeatures & blitting) != blitting)
    {
        // Keep loading usable textures on devices without filtered mip generation.
        mipLevels = 1;
        std::cerr << "Texture mip generation unavailable; using mip 0: " << path << '\n';
    }

    const VkDeviceSize byteCount = static_cast<VkDeviceSize>(width) * height * 4;
    if (byteCount > (std::numeric_limits<size_t>::max)())
        throw std::runtime_error("Texture is too large for a CPU staging allocation: " + path);
    CreateImage();
    UploadPixels(pixels.get(), byteCount);
    CreateViewAndSampler();
}

void GfxTexture::CreateImage()
{
    const VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT
        | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    VkImageFormatProperties supported{};
    CheckTextureResult(vkGetPhysicalDeviceImageFormatProperties(gfxCtx->physicalDevice, format,
        VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0, &supported),
        "Unsupported texture image configuration: " + path);
    if (width > supported.maxExtent.width || height > supported.maxExtent.height
        || mipLevels > supported.maxMipLevels)
        throw std::runtime_error("Texture dimensions exceed device limits: " + path);

    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.extent = { width, height, 1 };
    info.mipLevels = mipLevels;
    info.arrayLayers = 1;
    info.format = format;
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    info.usage = usage;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    CheckTextureResult(vkCreateImage(device, &info, nullptr, &image), "Creating texture: " + path);

    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(device, image, &requirements);
    VkMemoryAllocateInfo allocation{};
    allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocation.allocationSize = requirements.size;
    allocation.memoryTypeIndex = FindMemoryType_Internal(requirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    CheckTextureResult(vkAllocateMemory(device, &allocation, nullptr, &memory), "Allocating texture: " + path);
    CheckTextureResult(vkBindImageMemory(device, image, memory, 0), "Binding texture memory: " + path);
    DebugUtils::getInstance().SetVulkanObjectName(image, path.c_str());
}

void GfxTexture::UploadPixels(const unsigned char* pixels, VkDeviceSize byteCount)
{
    // Staging resources must also be released if allocation or mapping fails.
    struct Staging
    {
        VkDevice device;
        VkBuffer buffer = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        ~Staging()
        {
            vkDestroyBuffer(device, buffer, nullptr);
            vkFreeMemory(device, memory, nullptr);
        }
    } staging{ device };
    CreateBuffer_Internal(byteCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging.buffer, staging.memory, "TextureStagingBuffer", "TextureStagingMemory");
    void* mapped = nullptr;
    CheckTextureResult(vkMapMemory(device, staging.memory, 0, byteCount, 0, &mapped),
        "Mapping texture staging memory: " + path);
    std::memcpy(mapped, pixels, static_cast<size_t>(byteCount));
    vkUnmapMemory(device, staging.memory);

    const VkCommandBuffer commandBuffer = BeginSingleTimeCommandBuffer_Internal();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1 };
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    VkBufferImageCopy copy{};
    copy.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    copy.imageExtent = { width, height, 1 };
    vkCmdCopyBufferToImage(commandBuffer, staging.buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy);

    int32_t mipWidth = static_cast<int32_t>(width);
    int32_t mipHeight = static_cast<int32_t>(height);
    barrier.subresourceRange.levelCount = 1;
    for (uint32_t mip = 1; mip < mipLevels; ++mip)
    {
        // The previous destination becomes the source for the next smaller mip.
        barrier.subresourceRange.baseMipLevel = mip - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);

        const int32_t nextWidth = (std::max)(mipWidth / 2, 1);
        const int32_t nextHeight = (std::max)(mipHeight / 2, 1);
        VkImageBlit blit{};
        blit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mip - 1, 0, 1 };
        blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
        blit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mip, 0, 1 };
        blit.dstOffsets[1] = { nextWidth, nextHeight, 1 };
        vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0, 0, nullptr, 0, nullptr, 1, &barrier);
        mipWidth = nextWidth;
        mipHeight = nextHeight;
    }

    // The final mip was written, never read as a blit source (also handles 1x1 textures).
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier);

    // Existing helper waits for completion, so staging can be freed on return.
    EndSingleTimeCommandBuffer_Internal(commandBuffer);
}

void GfxTexture::CreateViewAndSampler()
{
    VkImageViewCreateInfo view{};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.image = image;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = format;
    view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 1 };
    CheckTextureResult(vkCreateImageView(device, &view, nullptr, &imageView), "Creating texture view: " + path);
    const std::string viewName = path + "ImageView";
    DebugUtils::getInstance().SetVulkanObjectName(imageView, viewName.c_str());

    VkSamplerCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    info.magFilter = VK_FILTER_LINEAR;
    info.minFilter = VK_FILTER_LINEAR;
    info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    // Support != enablement: no anisotropy until the device explicitly enables that feature.
    info.anisotropyEnable = VK_FALSE;
    info.maxAnisotropy = 1.0f;
    info.minLod = 0.0f;
    info.maxLod = static_cast<float>(mipLevels - 1);
    info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    CheckTextureResult(vkCreateSampler(device, &info, nullptr, &sampler), "Creating texture sampler: " + path);
}
