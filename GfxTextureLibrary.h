#pragma once
#include "GfxTexture.h"
#include <map>
#include <memory>
#include <utility>

// Owns file textures; callers borrow stable pointers until Clear().
class GfxTextureLibrary
{
public:
    const GfxTexture* GetWhite();
    const GfxTexture* Load(const std::string& path,
        GfxTextureColorSpace colorSpace = GfxTextureColorSpace::SRGB);
    size_t GetLoadedTextureCount() const { return textures.size(); }
    // GPU work and descriptor use must have ended before clearing the library.
    void Clear();

private:
    // The same bytes interpreted as color or linear data are different GPU resources.
    using Key = std::pair<std::string, GfxTextureColorSpace>;
    std::map<Key, std::unique_ptr<GfxTexture>> textures;
};
