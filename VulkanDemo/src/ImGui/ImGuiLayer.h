#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

class ImGuiLayer
{
public:
	ImGuiLayer(GLFWwindow* window);
	~ImGuiLayer();

	using ImGuiUpdateFunc = std::function<void()>;
	void Update(ImGuiUpdateFunc updateFunc);
	void Render(vk::CommandBuffer cmd);
private:
	GLFWwindow* m_GlfwWindow;
	VkDescriptorPool m_DescriptorPool;
};