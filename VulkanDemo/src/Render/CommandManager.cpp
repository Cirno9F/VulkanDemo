#include "CommandManager.h"
#include "Context.h"

CommandManager::CommandManager()
{
	vk::CommandPoolCreateInfo createInfo;
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	m_CommandPool = Context::s_Context->m_Device.createCommandPool(createInfo);
}

CommandManager::~CommandManager()
{
	auto& device = Context::s_Context->m_Device;

	device.destroyCommandPool(m_CommandPool);
}

void CommandManager::ResetCommandBuffers()
{
	auto& device = Context::s_Context->m_Device;

	device.resetCommandPool(m_CommandPool);
}

void CommandManager::FreeCommandBuffer(const vk::CommandBuffer& buffer)
{
	auto& device = Context::s_Context->m_Device;

	device.freeCommandBuffers(m_CommandPool, buffer);
}

void CommandManager::ExcuteCommand(vk::Queue queue, RecordCmdFunc func)
{
	auto cmdBuf = CreateCommandBuffer();

	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	cmdBuf.begin(beginInfo);
	if (func) func(cmdBuf);
	cmdBuf.end();

	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(cmdBuf);
	queue.submit(submitInfo);
	queue.waitIdle();
	Context::s_Context->m_Device.waitIdle();
	FreeCommandBuffer(cmdBuf);
}

vk::CommandBuffer CommandManager::CreateCommandBuffer()
{
	return CreateCommandBuffers(1)[0];
}

std::vector<vk::CommandBuffer> CommandManager::CreateCommandBuffers(std::uint32_t count)
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(m_CommandPool)
		.setCommandBufferCount(count)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	return Context::s_Context->m_Device.allocateCommandBuffers(allocInfo);
}
