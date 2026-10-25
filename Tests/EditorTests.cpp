#include "../gfxMaths.h"
#include "../GfxObject.h"
#include "../GfxScene.h"
#include "../GfxEditorLayout.h"
#include "../Imgui/imgui.h"
#include "../Imgui/imgui_internal.h"
#include "../Imgui/ImGuizmo/ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

static void Require(bool condition, const char* message)
{
    if (!condition) throw std::runtime_error(message);
}

static bool Near(const glm::mat4& a, const glm::mat4& b)
{
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            if (std::abs(a[c][r] - b[c][r]) > 0.0005f) return false;
    return true;
}

static void TestTransforms()
{
    for (const auto& scale : { glm::vec3(1), glm::vec3(2, 3, 4), glm::vec3(-2, 3, 4),
        glm::vec3(-2, -3, 4), glm::vec3(-2, -3, -4) })
    {
        GfxTransform source;
        source.position = glm::vec3(10, -20, 30);
        source.scale = scale;
        source.SetEulerAngles(glm::radians(glm::vec3(25, -40, 80)));
        const auto matrix = source.GetModelMatrix();
        GfxTransform restored;
        Require(restored.TrySetModelMatrix(matrix), "Valid TRS rejected");
        Require(Near(matrix, restored.GetModelMatrix()), "TRS reconstruction failed");
        Require(source.TrySetModelMatrix(matrix) && glm::length(source.scale - scale) < 0.0001f,
            "Existing signed scale was not preserved");
    }
    GfxTransform transform;
    const auto initial = transform.GetModelMatrix();
    glm::mat4 invalid(1);
    invalid[1][0] = 0.5f;
    Require(!transform.TrySetModelMatrix(invalid), "Shear accepted");
    invalid = glm::mat4(1);
    invalid[0][0] = 0;
    Require(!transform.TrySetModelMatrix(invalid), "Singular matrix accepted");
    invalid = glm::mat4(1);
    invalid[0][3] = 0.2f;
    Require(!transform.TrySetModelMatrix(invalid), "Perspective accepted");
    invalid = glm::mat4(1);
    invalid[3][0] = std::numeric_limits<float>::quiet_NaN();
    Require(!transform.TrySetModelMatrix(invalid), "NaN accepted");
    Require(Near(initial, transform.GetModelMatrix()), "Rejected edit modified transform");
}

static void TestGizmo()
{
    ImGui::CreateContext();
    struct ContextGuard { ~ContextGuard() { ImGui::DestroyContext(); } } guard;
    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.DisplaySize = ImVec2(800, 600);
    io.DeltaTime = 1.0f / 60.0f;
    unsigned char* pixels = nullptr;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    const auto view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(0, 1, 0));
    const auto projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 500.0f);
    glm::mat4 model(1);
    auto frame = [&](float x, float y, bool down, ImGuizmo::OPERATION operation)
    {
        io.AddMousePosEvent(x, y);
        io.AddMouseButtonEvent(0, down);
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
        ImGuizmo::SetRect(0, 0, 800, 600);
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::Enable(true);
        ImGuizmo::PushID("test-object");
        ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), operation,
            ImGuizmo::LOCAL, glm::value_ptr(model));
        ImGuizmo::PopID();
        ImGui::Render();
        Require(ImGui::GetDrawData()->TotalVtxCount > 0, "Gizmo emitted no draw data");
    };
    // Exercise the actual vendored widget with synthetic mouse input, no GPU/backend.
    frame(400, 300, false, ImGuizmo::TRANSLATE);
    frame(400, 300, true, ImGuizmo::TRANSLATE);
    Require(ImGuizmo::IsUsingAny(), "Gizmo center did not capture drag");
    frame(440, 300, true, ImGuizmo::TRANSLATE);
    Require(model[3].x > 0.1f, "Mouse drag did not translate toward screen right");
    frame(440, 300, false, ImGuizmo::TRANSLATE);
    Require(!ImGuizmo::IsUsingAny(), "Gizmo failed to release drag");
    GfxTransform transform;
    Require(transform.TrySetModelMatrix(model) && Near(model, transform.GetModelMatrix()),
        "Gizmo output did not round-trip to GfxTransform");
    frame(20, 20, false, ImGuizmo::ROTATE);
    frame(20, 20, false, ImGuizmo::SCALE);

    GfxScene scene;
    for (bool collapsed : { false, true, false })
    {
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        ImGui::SetWindowCollapsed("Scene Objects", collapsed);
        ImGui::SetWindowCollapsed("Transform Editor", collapsed);
        scene.DrawImgui(view, projection);
        ImGui::Render(); // Begin/End must balance even for collapsed windows.
    }
}

static void TestDocking()
{
    ImGui::CreateContext();
    struct ContextGuard { ~ContextGuard() { ImGui::DestroyContext(); } } guard;
    auto configure = []()
    {
        auto& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.DisplaySize = ImVec2(1280, 720);
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    };
    configure();
    GfxScene scene;
    auto frame = [&](bool reset = false)
    {
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
        const ImGuiID root = GfxEditorLayout::DrawDockspace(reset);
        scene.DrawImgui(glm::mat4(1), glm::mat4(1));
        ImGui::Begin("GFX options");
        ImGui::TextUnformatted("Docking test");
        ImGui::End();
        ImGui::Render();
        return root;
    };
    auto dockId = [](const char* name)
    {
        const auto* window = ImGui::FindWindowByName(name);
        Require(window != nullptr && window->DockId != 0, "Panel was not docked");
        return window->DockId;
    };
    const ImGuiID root = frame();
    frame();
    const ImGuiID hierarchy = dockId("Scene Objects");
    const ImGuiID inspector = dockId("Transform Editor");
    Require(hierarchy != inspector && dockId("GFX options") != inspector, "Default layout did not split panels");
    const auto* center = ImGui::DockBuilderGetCentralNode(root);
    Require(center != nullptr && center->Windows.empty(), "Scene center is not empty");
    Require((center->MergedFlags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, "Scene center is not transparent");
    Require(!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable), "Native viewports were enabled");

    ImGui::GetIO().DisplaySize = ImVec2(1600, 900);
    frame();
    frame();
    Require(ImGui::DockBuilderGetNode(root)->Size.x > 1500, "Dockspace did not follow window resize");
    Require(dockId("Scene Objects") == hierarchy, "Resize rebuilt the user's layout");

    // Group two panels into tabs, then ensure normal frames do not reset the layout.
    // A real drag first undocks the payload, then resolves the updated target.
    // Undocking an only child may merge its sibling into the parent node.
    ImGuiWindow* payload = ImGui::FindWindowByName("GFX options");
    ImGui::DockContextQueueUndockWindow(ImGui::GetCurrentContext(), payload);
    frame();
    frame();
    Require(payload->DockId == 0, "Panel did not become floating");
    ImGuiWindow* target = ImGui::FindWindowByName("Transform Editor");
    ImGui::DockContextQueueDock(ImGui::GetCurrentContext(), target, target->DockNode,
        payload, ImGuiDir_None, 0.5f, false);
    frame();
    frame();
    Require(dockId("GFX options") == dockId("Transform Editor"), "Tab grouping was lost");
    const std::string saved = ImGui::SaveIniSettingsToMemory();
    Require(saved.find("[Docking][Data]") != std::string::npos, "Docking settings were not serialized");
    ImGui::DestroyContext();
    ImGui::CreateContext();
    configure();
    ImGui::LoadIniSettingsFromMemory(saved.c_str(), saved.size());
    frame();
    frame();
    Require(dockId("GFX options") == dockId("Transform Editor"), "Saved tab layout did not survive context reload");
    frame(true);
    frame();
    Require(dockId("GFX options") != dockId("Transform Editor"), "Reset did not restore the default split");
}

static void TestProjection()
{
    const auto view = glm::lookAt(glm::vec3(0, 0, 5), glm::vec3(0), glm::vec3(0, 1, 0));
    for (const auto& size : { glm::vec2(800, 600), glm::vec2(1280, 720), glm::vec2(640, 360) })
    {
        auto vkProjection = glm::perspective(glm::radians(45.0f), size.x / size.y, 0.1f, 500.0f);
        vkProjection[1][1] *= -1;
        auto gizmoProjection = vkProjection;
        for (int c = 0; c < 4; ++c) gizmoProjection[c][1] *= -1;
        for (const auto& point : { glm::vec3(-1, 1, 0), glm::vec3(1, -1, 0), glm::vec3(0) })
        {
            const auto clip = vkProjection * view * glm::vec4(point, 1);
            const auto screen = (glm::vec2(clip) / clip.w * 0.5f + 0.5f) * size;
            glm::vec3 origin, direction;
            ImGuizmo::ComputeMouseRay(glm::value_ptr(view), glm::value_ptr(gizmoProjection),
                ImVec2(screen.x, screen.y), ImVec2(0, 0), ImVec2(size.x, size.y),
                glm::value_ptr(origin), glm::value_ptr(direction));
            const auto hit = origin + direction * (-origin.z / direction.z);
            Require(glm::length(hit - point) < 0.0001f, "Vulkan projection and gizmo picking disagree");
        }
    }
}

int main()
{
    try
    {
        TestTransforms();
        TestProjection();
        TestGizmo();
        TestDocking();
        std::cout << "PASS: TRS, projection, ImGuizmo, collapsed panels, docking/splits/tabs/resize/ini/reset.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
