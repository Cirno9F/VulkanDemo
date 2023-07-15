#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "../Base.h"

#include "ImGuiLayer.h"
#include "../Render/Context.h"

ImGuiLayer::ImGuiLayer(GLFWwindow* window) : m_GlfwWindow(window)
{
    auto device = Context::s_Context->m_Device;
    auto instance = Context::s_Context->m_Instance;
    auto phyDevice = Context::s_Context->m_PhysicalDevice;
    auto graphicsQueue = Context::s_Context->m_GraphicsQueue;
    auto renderPass = Context::s_Context->m_RenderProcess->m_RenderPass;
    auto& cmdMgr = Context::s_Context->m_CommandManager;

    //1: create descriptor pool for IMGUI
    // the size of the pool is very oversize, but it's copied from imgui demo itself.
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    ASSERT_IFNOT(vkCreateDescriptorPool(device, &pool_info, nullptr, &m_DescriptorPool) == VK_SUCCESS, "create imgui descriptor pool failed");


    // 2: initialize imgui library

    //this initializes the core structures of imgui
    auto context = ImGui::CreateContext();
    ImGui::SetCurrentContext(context);
    //this initializes imgui for glfw
    ImGui_ImplGlfw_InitForVulkan(m_GlfwWindow, true);

    //this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = phyDevice;
    init_info.Device = device;
    init_info.Queue = graphicsQueue;
    init_info.DescriptorPool = m_DescriptorPool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, renderPass);

    //execute a gpu command to upload imgui font textures
    cmdMgr->ExcuteCommand(graphicsQueue, 
        [&](VkCommandBuffer cmd) {
            ImGui_ImplVulkan_CreateFontsTexture(cmd);
        }
    );
    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

ImGuiLayer::~ImGuiLayer()
{
    auto device = Context::s_Context->m_Device;

    vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
    ImGui_ImplVulkan_Shutdown();
}

void ImGuiLayer::Render(vk::CommandBuffer cmd)
{
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

void ImGuiLayer::Update(ImGuiUpdateFunc updateFunc)
{
    //imgui new frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    updateFunc();

    ImGui::Render();
}
