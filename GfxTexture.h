#pragma once
#include <string>
#include <vulkan/vulkan_core.h>

enum class GfxTextureColorSpace
{
    SRGB,
    Linear
};

// Owns an immutable 2D texture. Destroy only after GPU use has finished.
class GfxTexture
{
public:
    GfxTexture(const std::string& path, GfxTextureColorSpace colorSpace);
    ~GfxTexture();
    GfxTexture(const GfxTexture&) = delete;
    GfxTexture& operator=(const GfxTexture&) = delete;

    VkImage GetImage() const { return image; }
    VkImageView GetImageView() const { return imageView; }
    VkSampler GetSampler() const { return sampler; }
    VkFormat GetFormat() const { return format; }
    uint32_t GetWidth() const { return width; }
    uint32_t GetHeight() const { return height; }
    uint32_t GetMipLevels() const { return mipLevels; }
    const std::string& GetPath() const { return path; }

private:
    void Load();
    void CreateImage();
    void UploadPixels(const unsigned char* pixels, VkDeviceSize byteCount);
    void CreateViewAndSampler();
    void Destroy();

    std::string path;
    VkFormat format;
    VkDevice device = VK_NULL_HANDLE;
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t mipLevels = 1;
};
