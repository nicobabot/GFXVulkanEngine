# ImGuizmo integration

Upstream: https://github.com/CedricGuillemet/ImGuizmo
Revision: `105e4fde45e4e74aae8973d98750820ef7aca7e2`
Files: `src/ImGuizmo.h`, `src/ImGuizmo.cpp`, and `LICENSE` (MIT).
Only the transform widget is vendored; no optional widgets or build dependencies.

Select an object in Scene Objects. Transform Editor exposes translate (T),
rotate (E), scale (R), local/world axes, and optional snapping. Scale is local
to avoid shear, which GfxTransform cannot represent. Numeric controls remain.
The whole object (all submeshes) moves; shared geometry is unchanged.

The editor uses untransposed GLM model/view matrices. It undoes the Vulkan
projection's clip-Y flip because ImGuizmo maps NDC Y to top-left screen space.
The gizmo covers the full logical client area, matching the full framebuffer
scene even on HiDPI displays. ImGui's Vulkan backend handles framebuffer scale.

UI transforms are applied before recording color/shadow draws for that frame.
The gizmo adds ImGui draw data, not Vulkan pipelines or scene geometry buffers.
Singular transforms cannot be manipulated; restore nonzero scale numerically.
No viewport picking, undo/redo, persistence, or per-submesh transforms yet.
