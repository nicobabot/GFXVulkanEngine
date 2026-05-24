#include "ImguiHandler.h"
#include <stdexcept>
#include "HelloTriangleApp.h"

ImguiVulkanObjects ImguiHandler::vkObjects{};
VkDescriptorPool ImguiHandler::imguiDescriptorPool = VK_NULL_HANDLE;
bool ImguiHandler::isOpen = true;
bool ImguiHandler::lastBlurSetting = true;
bool ImguiHandler::isBlurEnabled = true;

void ImguiHandler::ImguiVulkanResultLogger(VkResult err)
{
    if (err == 0)
        return;
    //if (err < 0)
        throw std::runtime_error(std::string("[vulkan] Error: VkResult = %d\n", err));
}

void ImguiHandler::InitWindow(GLFWwindow* window)
{
    if (window == nullptr) 
    {
        throw std::runtime_error("Error initializing imgui, window is nullptr!");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    ImGui_ImplGlfw_InitForVulkan(window, true);
}

void ImguiHandler::InitVulkan(ImguiVulkanObjects vulkanObj)
{
    vkObjects = vulkanObj;

    VkDescriptorPoolSize pool_size = { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 };
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 10;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;
    vkCreateDescriptorPool(vkObjects.logicalDevice, &pool_info, nullptr, &imguiDescriptorPool);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance = vkObjects.instance;
    init_info.PhysicalDevice = vkObjects.physicalDevice;
    init_info.Device = vkObjects.logicalDevice;
    init_info.QueueFamily = vkObjects.queueFamilyIndices->presentationFamily.value();
    init_info.Queue = vkObjects.queue;
    init_info.DescriptorPool = imguiDescriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = vkObjects.imageCount;
    init_info.PipelineInfoMain.RenderPass = vkObjects.renderPass;
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = vkObjects.MSAASamples;
    init_info.CheckVkResultFn = ImguiVulkanResultLogger;
    init_info.Allocator = nullptr;

    ImGui_ImplVulkan_Init(&init_info);
}

void ImguiHandler::NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImguiHandler::Draw(const VkCommandBuffer& commandBuffer)
{
    DebugUtils::getInstance().BeginDebugLabel(commandBuffer, "IMGUI");

    // --- Your ImGui UI code here ---
    //ImGui::ShowDemoWindow();  // useful for testing

    if (ImGui::Begin("GFX options", &isOpen, 0))
    {
        ImGui::Checkbox("Enable Blur", &isBlurEnabled);
    }

    ImGui::End();

    // --- Record to command buffer ---
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);

    DebugUtils::getInstance().EndDebugLabel(commandBuffer);
}

void ImguiHandler::Cleanup()
{
    // Shutdown ImGui Vulkan backend first so it can free its descriptor sets / resources
    ImGui_ImplVulkan_Shutdown();
    if (imguiDescriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(vkObjects.logicalDevice, imguiDescriptorPool, nullptr);
        imguiDescriptorPool = VK_NULL_HANDLE;
    }
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}