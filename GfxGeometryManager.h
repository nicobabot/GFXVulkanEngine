#pragma once
#include <map>
#include <memory>
#include <string>

class GfxGeometry;

// Owns shared meshes. Scene objects borrow pointers until Cleanup().
class GfxGeometryManager
{
public:
    GfxGeometryManager();
    ~GfxGeometryManager();
    GfxGeometryManager(const GfxGeometryManager&) = delete;
    GfxGeometryManager& operator=(const GfxGeometryManager&) = delete;

    GfxGeometry* CreateCubeGeometry();
    GfxGeometry* CreatePlaneGeometry();
    GfxGeometry* CreateSphereGeometry();
    // Call only after GPU work is complete, before destroying the device.
    void Cleanup();

private:
    GfxGeometry* GetOrCreate(const std::string& name, void (*generate)(GfxGeometry&));
    std::map<std::string, std::unique_ptr<GfxGeometry>> geometryMap;
};
