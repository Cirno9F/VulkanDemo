#include "Application.h"

#include <imgui.h>


Application::Application(const std::string& name, uint32_t width, uint32_t height)
	:m_Name(name), m_Width(width), m_Height(height)
{
	InitWindow();

	//glfw
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> requiredExtensions;
	for (uint32_t i = 0; i < glfwExtensionCount; i++)
	{
		requiredExtensions.emplace_back(glfwExtensions[i]);
		LOG_TRACE("Extension: {0}" , glfwExtensions[i]);
	}

	//context
	Context::Init(m_Width,
		m_Height,
		requiredExtensions,
		[&](vk::Instance instance)
		{
			VkSurfaceKHR surface;
			ASSERT_IFNOT(glfwCreateWindowSurface(instance, m_Window, nullptr, &surface) == VK_SUCCESS, "glfwCreateWindowSurface failed!");
			return surface;
		});

	m_ImGuiLayer = CreateRef<ImGuiLayer>(m_Window);
}

Application::~Application()
{ 
	//因为imgui layer里面有个descriptorPool，所以waitIdle暂时放这
	Context::s_Context->m_Device.waitIdle();

	m_ImGuiLayer = nullptr;

	//context
	Context::Close();

	//glfw
	glfwDestroyWindow(m_Window);
	glfwTerminate();

	m_Window = nullptr;
}

void Application::Run()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();

		auto& renderer = Context::s_Context->m_Renderer;

		m_ImGuiLayer->Update([&]() 
			{
				ImGui::Begin("Setting");
				ImGui::Text("----Camera----");
				ImGui::DragFloat3("EulerAngle", glm::value_ptr(renderer->m_Camera.EulerAngle), 0.02f, -2.0f, 2.0f, "%.3f");
				ImGui::DragFloat("Distance", &renderer->m_Camera.Distance, 0.02f, 0.0f, 10.0f, "%.3f");
				ImGui::Text("----Light----");
				ImGui::DragFloat3("LightDir", glm::value_ptr(renderer->m_CommonBufferInfo.LightDir), 0.02f, -2.0f, 2.0f, "%.3f");
				ImGui::Text("----Material----");
				ImGui::ColorEdit3("Albedo", glm::value_ptr(renderer->m_PBRMaterial.Albedo));
				ImGui::DragFloat("Roughness", &renderer->m_PBRMaterial.Roughness, 0.02f, 0.0f, 1.0f, "%.3f");
				ImGui::DragFloat("Metallic", &renderer->m_PBRMaterial.Metallic, 0.02f, 0.0f, 1.0f, "%.3f");
				ImGui::DragFloat("Reflectance", &renderer->m_PBRMaterial.Reflectance, 0.02f, 0.0f, 1.0f, "%.3f");
				ImGui::End();
			});


		renderer->Begin();
		renderer->DrawTriangle();
		m_ImGuiLayer->Render(renderer->GetFrameCmd());
		renderer->End();
	}
}

void Application::InitWindow()
{
	glfwInit();

	//we need to tell it to not create an OpenGL context with a subsequent call :
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	//disable window resize for now with another window hint call :
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_Window = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), nullptr, nullptr);
}
