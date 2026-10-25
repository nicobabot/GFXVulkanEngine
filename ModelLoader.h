#pragma once
#include "GfxMeshData.h"
#include <string>
#include <vector>

const std::string MODEL_PATH = "Models/viking_room.obj";
const std::string TEXTURE_PATH = "Textures/viking_room.png";

class GfxLoader
{
public:
    // Retain shape/material runs as index ranges in shared buffers.
    // Preserve source positions and winding. Apply orientation/scale per instance.
    static GfxMeshData LoadObj(const std::string& path, bool flipV = true);
};

