#include "GfxTextureLibrary.h"
#include <filesystem>
#include <stdexcept>

const GfxTexture* GfxTextureLibrary::GetWhite()
{
    const Key key{ "builtin:white", GfxTextureColorSpace::SRGB };
    const auto found = textures.find(key);
    if (found != textures.end())
        return found->second.get();
    auto texture = std::make_unique<GfxTexture>();
    const GfxTexture* result = texture.get();
    textures.emplace(key, std::move(texture));
    return result;
}

const GfxTexture* GfxTextureLibrary::Load(const std::string& path, GfxTextureColorSpace colorSpace)
{
    if (path.empty())
        throw std::runtime_error("Texture path must not be empty");

    // Resolve relative paths and '..' so common aliases reuse one texture.
    const std::string resolvedPath = std::filesystem::weakly_canonical(
        std::filesystem::absolute(path)).generic_string();
    const Key key{ resolvedPath, colorSpace };
    const auto found = textures.find(key);
    if (found != textures.end())
        return found->second.get();

    auto texture = std::make_unique<GfxTexture>(resolvedPath, colorSpace);
    const GfxTexture* result = texture.get();
    // Failed loads never publish a partially initialized resource into the cache.
    textures.emplace(key, std::move(texture));
    return result;
}

void GfxTextureLibrary::Clear()
{
    textures.clear();
}
