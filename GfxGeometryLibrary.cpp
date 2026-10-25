#include "GfxGeometryLibrary.h"
#include "GfxGeometry.h"
#include "BasicPolygons.h"
#include "ModelLoader.h"
#include <filesystem>
#include <iostream>
#include <stdexcept>

GfxGeometryLibrary::GfxGeometryLibrary() = default;
GfxGeometryLibrary::~GfxGeometryLibrary() = default;

GfxGeometry* GfxGeometryLibrary::GetOrCreate(const std::string& name, void (*generate)(GfxGeometry&))
{
    const auto found = geometries.find(name);
    if (found != geometries.end())
        return found->second.get();

    auto geometry = std::make_unique<GfxGeometry>(name);
    generate(*geometry);
    geometry->Upload();
    GfxGeometry* result = geometry.get();
    geometries.emplace(name, std::move(geometry));
    return result;
}

GfxGeometry* GfxGeometryLibrary::GetOrCreateCube()
{
    return GetOrCreate("cube", GenerateCubeGeometry);
}

GfxGeometry* GfxGeometryLibrary::GetOrCreatePlane()
{
    return GetOrCreate("plane", GeneratePlaneGeometry);
}

GfxGeometry* GfxGeometryLibrary::GetOrCreateSphere()
{
    return GetOrCreate("sphere", GenerateSphereGeometry);
}

GfxGeometry* GfxGeometryLibrary::LoadObj(const std::string& path, bool flipV)
{
    if (path.empty())
        throw std::runtime_error("OBJ path must not be empty");
    const std::string resolvedPath = std::filesystem::weakly_canonical(
        std::filesystem::absolute(path)).generic_string();
    const std::string key = "obj:" + resolvedPath + (flipV ? ":flipV" : ":keepV");
    const auto found = geometries.find(key);
    if (found != geometries.end())
        return found->second.get();

    GfxMeshData data = GfxLoader::LoadObj(resolvedPath, flipV);
    auto geometry = std::make_unique<GfxGeometry>(resolvedPath);
    geometry->vertices = std::move(data.vertices);
    geometry->indices = std::move(data.indices);
    geometry->submeshes = std::move(data.submeshes);
    geometry->sourceMaterials = std::move(data.materials);
    geometry->Upload();
    GfxGeometry* result = geometry.get();
    geometries.emplace(key, std::move(geometry));
    std::cout << "Loaded OBJ: " << resolvedPath << " (" << result->vertices.size()
        << " vertices, " << result->indexCount / 3 << " triangles)\n";
    if (!data.warnings.empty())
        std::cerr << "OBJ import notes for " << resolvedPath << ":\n" << data.warnings;
    return result;
}

void GfxGeometryLibrary::Clear()
{
    geometries.clear();
}
