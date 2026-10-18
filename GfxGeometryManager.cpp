#include "GfxGeometryManager.h"
#include "GfxGeometry.h"
#include "BasicPolygons.h"

GfxGeometryManager::GfxGeometryManager() = default;
GfxGeometryManager::~GfxGeometryManager() = default;

GfxGeometry* GfxGeometryManager::GetOrCreate(const std::string& name, void (*generate)(GfxGeometry&))
{
    const auto found = geometryMap.find(name);
    if (found != geometryMap.end())
        return found->second.get();

    auto geometry = std::make_unique<GfxGeometry>(name);
    generate(*geometry);
    geometry->Upload();
    GfxGeometry* result = geometry.get();
    // Publish only complete meshes; unique_ptr also cleans up failed uploads.
    geometryMap.emplace(name, std::move(geometry));
    return result;
}

GfxGeometry* GfxGeometryManager::CreateCubeGeometry()
{
    return GetOrCreate("cube", GenerateCubeGeometry);
}

GfxGeometry* GfxGeometryManager::CreatePlaneGeometry()
{
    return GetOrCreate("plane", GeneratePlaneGeometry);
}

GfxGeometry* GfxGeometryManager::CreateSphereGeometry()
{
    return GetOrCreate("sphere", GenerateSphereGeometry);
}

void GfxGeometryManager::Cleanup()
{
    // Each unique geometry destroys its GPU buffers exactly once.
    geometryMap.clear();
}
