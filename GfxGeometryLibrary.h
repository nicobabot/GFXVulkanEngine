#pragma once
#include <map>
#include <memory>
#include <string>

class GfxGeometry;

// Owns reusable geometry and its GPU buffers independently from scene instances.
class GfxGeometryLibrary
{
public:
    GfxGeometryLibrary();
    ~GfxGeometryLibrary();
    GfxGeometryLibrary(const GfxGeometryLibrary&) = delete;
    GfxGeometryLibrary& operator=(const GfxGeometryLibrary&) = delete;

    GfxGeometry* GetOrCreateCube();
    GfxGeometry* GetOrCreatePlane();
    GfxGeometry* GetOrCreateSphere();
    void Clear();

private:
    GfxGeometry* GetOrCreate(const std::string& name, void (*generate)(GfxGeometry&));
    std::map<std::string, std::unique_ptr<GfxGeometry>> geometries;
};
