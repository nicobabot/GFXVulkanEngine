# Dear ImGui docking

Official upstream: https://github.com/ocornut/imgui
Tag: `v1.92.8-docking`
Commit: `b61e56346a92cfcaf1f43a545ca37b0b32239654`
License: `LICENSE.txt` (MIT).

The core, internal header, bundled stb headers, demo, and GLFW/Vulkan backends
come from this tag. `imconfig.h` is preserved (identical to the tag at integration).
The ImGuizmo subdirectory is a separate pinned dependency.
Local vendor delta: one whitespace-only cleanup in imgui_impl_vulkan.cpp.

## Engine integration

- Enable `ImGuiConfigFlags_DockingEnable` at context creation.
- Keep `ImGuiConfigFlags_ViewportsEnable` disabled: only the existing GLFW window
  and Vulkan swapchain are used. Floating panels stay inside the client area.
- Submit `GfxEditorLayout::DrawDockspace()` before the editor panels every frame.
- First-use layout: Scene Objects left, Transform Editor right, GFX options below it.
- The central node is transparent and reserved for the existing full-frame scene.
  This is not a Scene texture widget: docking does not resize the scene render area,
  change camera aspect, or create additional render targets.
- Drag a panel title/tab to a docking target to split or tab it; drag it away to float.
- ImGui saves docking in the existing `imgui.ini`. Do not regenerate the layout each
  frame. `Layout > Reset panels` explicitly rebuilds only the editor dock tree.
- Default layout uses internal DockBuilder APIs only in GfxEditorLayout.cpp.

Build with the existing Debug x64 task. `Test editor and gizmo (CPU)` covers dock
assignments, tab grouping, resize, in-memory ini round-trip, reset, and ImGuizmo.
No GPU/backend is required for these CPU tests.
