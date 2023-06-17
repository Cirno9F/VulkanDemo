#pragma once

#include "vulkan/vulkan.hpp"
#include <vector>

class Renderer
{
public:
	Renderer(uint32_t maxFlightCount = 2);
	~Renderer();

	void DrawTriangle();
private:
    std::vector<vk::CommandBuffer> m_CommandBuffers;
	std::vector<vk::Semaphore> m_SemaphoreImageAvaliables;
	std::vector<vk::Semaphore> m_SemaphoreImageDrawFinishs;
	std::vector<vk::Fence> m_Fences;

	void CreateCommandBuffers();
	void CreateSemaphores();
	void CreateFences();

	uint32_t m_MaxFlightCount;
	uint32_t m_CurFrame;
};