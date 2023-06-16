#include "Renderer.h"
#include "Context.h"
#include "SwapChain.h"

Renderer::Renderer()
{
	InitCommandPool();
	AllocCommandBuffer();
	CreateSemaphores();
	CreateFence();
}

Renderer::~Renderer()
{
	auto& device = Context::s_Context->m_Device;
	device.freeCommandBuffers(m_CommandPool, m_CommandBuffer);
	device.destroyCommandPool(m_CommandPool);
	device.destroySemaphore(m_SemaphoreImageAvaliable);
	device.destroySemaphore(m_SemaphoreImageDrawFinish);
	device.destroyFence(m_AvailableFence);
}

void Renderer::Render()
{
	auto& device = Context::s_Context->m_Device;
	auto& renderProcess = Context::s_Context->m_RenderProcess;
	auto& swapChain = Context::s_Context->m_SwapChain;
	auto result = device.acquireNextImageKHR(Context::s_Context->m_SwapChain->m_Swapchain,
		std::numeric_limits<uint64_t>::max(), m_SemaphoreImageAvaliable);

	ASSERT_IFNOT(result.result == vk::Result::eSuccess, "Acquire next image failed!");

	auto imageIndex = result.value;

	m_CommandBuffer.reset();
	
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	m_CommandBuffer.begin(beginInfo);
	{
		vk::RenderPassBeginInfo rpBeginInfo;
		vk::Rect2D area;
		vk::ClearValue clearValue;
		clearValue.color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
		area.setOffset({ 0, 0 })
			.setExtent(swapChain->m_SwapChainInfo.ImageExtent);
		rpBeginInfo.setRenderPass(renderProcess->m_RenderPass)
			.setRenderArea(area)
			.setFramebuffer(swapChain->m_FrameBuffers[imageIndex])
			.setClearValues(clearValue);
		m_CommandBuffer.beginRenderPass(rpBeginInfo, {});
		{
			m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, Context::s_Context->m_RenderProcess->m_Pipeline);
			m_CommandBuffer.draw(3, 1, 0, 0);
		}
		m_CommandBuffer.endRenderPass();
	}
	m_CommandBuffer.end();

	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(m_CommandBuffer)
		.setWaitSemaphores(m_SemaphoreImageAvaliable)
		.setSignalSemaphores(m_SemaphoreImageDrawFinish);
	Context::s_Context->m_GraphicsQueue.submit(submitInfo, m_AvailableFence);


	vk::PresentInfoKHR presentInfo;
	presentInfo.setImageIndices(imageIndex)
		.setSwapchains(swapChain->m_Swapchain)
		.setWaitSemaphores(m_SemaphoreImageDrawFinish);
	ASSERT_IFNOT(Context::s_Context->m_PresentQueue.presentKHR(presentInfo) == vk::Result::eSuccess, "Image present failed!");

	ASSERT_IFNOT(device.waitForFences(m_AvailableFence, true, std::numeric_limits<uint64_t>::max()) == vk::Result::eSuccess,
		"Wait for fence failed!");
	device.resetFences(m_AvailableFence);
}

void Renderer::InitCommandPool()
{
	vk::CommandPoolCreateInfo createInfo;
	createInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
	
	m_CommandPool = Context::s_Context->m_Device.createCommandPool(createInfo);
}

void Renderer::AllocCommandBuffer()
{
	vk::CommandBufferAllocateInfo allocInfo;
	allocInfo.setCommandPool(m_CommandPool)
		.setCommandBufferCount(1)
		.setLevel(vk::CommandBufferLevel::ePrimary);

	m_CommandBuffer = Context::s_Context->m_Device.allocateCommandBuffers(allocInfo)[0];
;}

void Renderer::CreateSemaphores()
{
	vk::SemaphoreCreateInfo createInfo;

	m_SemaphoreImageAvaliable = Context::s_Context->m_Device.createSemaphore(createInfo);
	m_SemaphoreImageDrawFinish = Context::s_Context->m_Device.createSemaphore(createInfo);
}

void Renderer::CreateFence()
{
	vk::FenceCreateInfo createInfo;

	m_AvailableFence = Context::s_Context->m_Device.createFence(createInfo);
}
