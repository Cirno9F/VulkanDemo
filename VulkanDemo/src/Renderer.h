#pragma once

#include "vulkan/vulkan.hpp"

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Render();
private:
	vk::CommandPool m_CommandPool;
	vk::CommandBuffer m_CommandBuffer;

	vk::Semaphore m_SemaphoreImageAvaliable;
	vk::Semaphore m_SemaphoreImageDrawFinish;
	vk::Fence m_AvailableFence;

	void InitCommandPool();
	void AllocCommandBuffer();
	void CreateSemaphores();
	void CreateFence();
};