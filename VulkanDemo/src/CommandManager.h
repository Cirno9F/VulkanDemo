#pragma once

#include <vulkan/vulkan.hpp>

class CommandManager
{
public:
	CommandManager();
	~CommandManager();
	void ResetCommandBuffers();
	void FreeCommandBuffer(const vk::CommandBuffer& buffer);
public:
	vk::CommandBuffer CreateCommandBuffer();
	std::vector<vk::CommandBuffer> CreateCommandBuffers(std::uint32_t count);
private:
	vk::CommandPool m_CommandPool;
};