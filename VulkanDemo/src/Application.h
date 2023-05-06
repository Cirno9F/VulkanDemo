#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>

class Application
{
public:
	Application(const std::string& name = "VulkanDemo" ,uint32_t width = 1600, uint32_t height = 900);
	~Application();

	void Run();
private:
	void InitWindow();
private:
	GLFWwindow* m_Window;
	uint32_t m_Width, m_Height;
	std::string m_Name;
};
