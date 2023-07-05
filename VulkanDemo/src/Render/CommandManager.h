#pragma once

#include <vulkan/vulkan.hpp>
#include <functional>

class CommandManager
{
public:
	CommandManager();
	~CommandManager();
	void ResetCommandBuffers();
	void FreeCommandBuffer(const vk::CommandBuffer& buffer);

	using RecordCmdFunc = std::function<void(vk::CommandBuffer&)>;
	void ExcuteCommand(vk::Queue queue, RecordCmdFunc func);
public:
	vk::CommandBuffer CreateCommandBuffer();
	std::vector<vk::CommandBuffer> CreateCommandBuffers(std::uint32_t count);
private:
	vk::CommandPool m_CommandPool;
};