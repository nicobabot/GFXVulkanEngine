#pragma once
#include "Imgui/imgui.h"

namespace GfxEditorLayout
{
    // Call after NewFrame and before all dockable panels. Existing ini layouts win.
    ImGuiID DrawDockspace(bool resetLayout = false);
}
