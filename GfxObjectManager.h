#pragma once
#include <vector>

class GfxObject;

class GfxObjectManager
{
public:
    static GfxObjectManager& get()
    {
        static GfxObjectManager instance;
        return instance;
    }

    GfxObjectManager(const GfxObjectManager&) = delete;
    GfxObjectManager& operator=(const GfxObjectManager&) = delete;

    void DrawImgui();

private:
    GfxObjectManager() = default;

public:
    std::vector<GfxObject*> objects;
    GfxObject* selectedObject = nullptr;
};

