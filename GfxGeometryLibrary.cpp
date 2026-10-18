#include "GfxGeometryLibrary.h"
#include "GfxGeometry.h"
#include "BasicPolygons.h"

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

void GfxGeometryLibrary::Clear()
{
    geometries.clear();
}
