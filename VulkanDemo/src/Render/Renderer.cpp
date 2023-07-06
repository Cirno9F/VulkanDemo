#include "Renderer.h"
#include "Context.h"
#include "SwapChain.h"
#include "RenderProcess.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

static std::array<VertexInput, 8> vertices =
{
	VertexInput{glm::vec3{-0.5f, -0.5f, 0.0f}, glm::vec2{0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f}},
	VertexInput{glm::vec3{ 0.5f, -0.5f, 0.0f}, glm::vec2{1.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}},
	VertexInput{glm::vec3{ 0.5f,  0.5f, 0.0f}, glm::vec2{1.0f, 1.0f}, glm::vec3{1.0f, 1.0f, 1.0f}},
	VertexInput{glm::vec3{-0.5f,  0.5f, 0.0f}, glm::vec2{0.0f, 1.0f}, glm::vec3{1.0f, 1.0f, 1.0f}},


	VertexInput{glm::vec3{-0.2f, -0.2f, 1.0f}, glm::vec2{0.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}},
	VertexInput{glm::vec3{ 0.2f, -0.2f, 1.0f}, glm::vec2{1.0f, 0.0f}, glm::vec3{1.0f, 1.0f, 1.0f}},
	VertexInput{glm::vec3{ 0.2f,  0.2f, 1.0f}, glm::vec2{1.0f, 1.0f}, glm::vec3{1.0f, 1.0f, 1.0f}},
	VertexInput{glm::vec3{-0.2f,  0.2f, 1.0f}, glm::vec2{0.0f, 1.0f}, glm::vec3{1.0f, 1.0f, 1.0f}},
};

static std::array<int, 12> indicies = { 0, 1, 2, 0, 2, 3 , 4, 5, 6, 4, 6, 7};

static glm::vec3 color = { 0.8f,0.6f,0.2f };

Renderer::Renderer(uint32_t maxFlightCount) : m_MaxFlightCount(maxFlightCount) , m_CurFrame(0)
{
	m_MVP.model = glm::mat4(1.0f);
	m_MVP.view = glm::mat4(1.0f);
	m_MVP.proj = glm::mat4(1.0f);

	CreateCommandBuffers();
	CreateSemaphores();
	CreateFences();
	CreateVertexBuffer();
	BufferVertexData();
	CreateUniformBuffer();
	BufferUniformData();
	AllocateSets();
	UpdateSets();

	m_DefaultTexture = CreateScope<Texture>("assets/texture/avatar.jpg");
}

Renderer::~Renderer()
{
	m_DefaultTexture = nullptr;

	m_DeviceIndexBuffer = nullptr;
	m_DeviceVertexBuffer = nullptr;
	for (uint32_t i = 0;i < m_MaxFlightCount;i++)
	{
		m_HostUniformBuffer[i] = nullptr;
		m_DeviceUniformBuffer[i] = nullptr;
	}

	auto& device = Context::s_Context->m_Device;
	auto& cmdMgr = Context::s_Context->m_CommandManager;
	
	for (uint32_t i = 0;i < m_MaxFlightCount;i++)
	{
		cmdMgr->FreeCommandBuffer(m_CommandBuffers[i]);
		device.destroySemaphore(m_SemaphoreImageAvaliables[i]);
		device.destroySemaphore(m_SemaphoreImageDrawFinishs[i]);
		device.destroyFence(m_Fences[i]);
	}
}

void Renderer::DrawTriangle()
{
	UpdateMVP();

	auto& device = Context::s_Context->m_Device;
	auto& renderProcess = Context::s_Context->m_RenderProcess;
	auto& swapChain = Context::s_Context->m_SwapChain;

	//Wait for current frame fence
	ASSERT_IFNOT(device.waitForFences(m_Fences[m_CurFrame], true, std::numeric_limits<uint64_t>::max()) == vk::Result::eSuccess,
		"Wait for fence failed!");
	device.resetFences(m_Fences[m_CurFrame]);

	//Acquire next image index
	auto result = device.acquireNextImageKHR(Context::s_Context->m_SwapChain->m_Swapchain,
		std::numeric_limits<uint64_t>::max(), m_SemaphoreImageAvaliables[m_CurFrame]);
	ASSERT_IFNOT(result.result == vk::Result::eSuccess, "Acquire next image failed!");
	auto imageIndex = result.value;

	m_CommandBuffers[m_CurFrame].reset();
	
	vk::CommandBufferBeginInfo beginInfo;
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.1f, 0.1f, 0.1f, 1.0f});
	clearValues[1].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };
	vk::RenderPassBeginInfo rpBeginInfo;
	rpBeginInfo.setRenderPass(renderProcess->m_RenderPass)
		.setRenderArea(vk::Rect2D({ 0,0 }, swapChain->m_SwapChainInfo.ImageExtent))
		.setFramebuffer(swapChain->m_FrameBuffers[imageIndex])
		.setClearValues(clearValues);

	m_CommandBuffers[m_CurFrame].begin(beginInfo);
	m_CommandBuffers[m_CurFrame].beginRenderPass(rpBeginInfo, {});
	m_CommandBuffers[m_CurFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, Context::s_Context->m_RenderProcess->m_Pipeline);
	vk::DeviceSize offset = 0;
	m_CommandBuffers[m_CurFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, Context::s_Context->m_RenderProcess->m_PipelineLayout, 0, { m_DescriptorSets[m_CurFrame], m_DefaultTexture->GetDescriptorSet() }, {});
	m_CommandBuffers[m_CurFrame].bindVertexBuffers(0, m_DeviceVertexBuffer->m_Buffer, offset);
	m_CommandBuffers[m_CurFrame].bindIndexBuffer(m_DeviceIndexBuffer->m_Buffer, 0, vk::IndexType::eUint32);
	m_CommandBuffers[m_CurFrame].drawIndexed(indicies.size(), 1, 0, 0, 0);
	m_CommandBuffers[m_CurFrame].endRenderPass();
	m_CommandBuffers[m_CurFrame].end();

	vk::SubmitInfo submitInfo;
	submitInfo.setCommandBuffers(m_CommandBuffers[m_CurFrame])
		.setWaitSemaphores(m_SemaphoreImageAvaliables[m_CurFrame])
		//.setWaitDstStageMask()
		.setSignalSemaphores(m_SemaphoreImageDrawFinishs[m_CurFrame]);
	Context::s_Context->m_GraphicsQueue.submit(submitInfo, m_Fences[m_CurFrame]);


	vk::PresentInfoKHR presentInfo;
	presentInfo.setImageIndices(imageIndex)
		.setSwapchains(swapChain->m_Swapchain)
		.setWaitSemaphores(m_SemaphoreImageDrawFinishs[m_CurFrame]);
	ASSERT_IFNOT(Context::s_Context->m_PresentQueue.presentKHR(presentInfo) == vk::Result::eSuccess, "Image present failed!");

	m_CurFrame = (m_CurFrame + 1) % m_MaxFlightCount;
}

void Renderer::CreateCommandBuffers()
{
	m_CommandBuffers = Context::s_Context->m_CommandManager->CreateCommandBuffers(m_MaxFlightCount);
;}

void Renderer::CreateSemaphores()
{
	auto& device = Context::s_Context->m_Device;
	vk::SemaphoreCreateInfo createInfo;

	m_SemaphoreImageAvaliables.resize(m_MaxFlightCount);
	m_SemaphoreImageDrawFinishs.resize(m_MaxFlightCount);

	for (int i = 0;i < m_MaxFlightCount;i++)
	{
		m_SemaphoreImageAvaliables[i] = Context::s_Context->m_Device.createSemaphore(createInfo);
		m_SemaphoreImageDrawFinishs[i] = Context::s_Context->m_Device.createSemaphore(createInfo);
	}
}

void Renderer::CreateFences()
{
	m_Fences.resize(m_MaxFlightCount, nullptr);

	for (auto& fence : m_Fences)
	{
		vk::FenceCreateInfo createInfo;
		createInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);     //创建Fence后，立刻唤醒该Fence，便可直接使用waitForFences等待
		fence = Context::s_Context->m_Device.createFence(createInfo);
	}

}

void Renderer::CreateVertexBuffer()
{
	//CPU本地存储的vertex buffer， 用来传输数据
	m_HostVertexBuffer = CreateScope<Buffer>(sizeof(VertexInput) * vertices.size(),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	//GPU存储的vertex buffer
	m_DeviceVertexBuffer = CreateScope<Buffer>(sizeof(VertexInput) * vertices.size(),
		vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	//Index Buffer
	m_HostIndexBuffer = CreateScope<Buffer>(sizeof(int) * indicies.size(),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	m_DeviceIndexBuffer = CreateScope<Buffer>(sizeof(int) * indicies.size(),
		vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eDeviceLocal);
}

void Renderer::BufferVertexData()
{
	//VertexBuffer
	void* ptr = Context::s_Context->m_Device.mapMemory(m_HostVertexBuffer->m_Memory, 0, m_HostVertexBuffer->m_Size);
	memcpy(ptr, vertices.data(), sizeof(vertices));
	Context::s_Context->m_Device.unmapMemory(m_HostVertexBuffer->m_Memory);
	CopyBuffer(m_HostVertexBuffer->m_Buffer, m_DeviceVertexBuffer->m_Buffer, m_HostVertexBuffer->m_Size, 0, 0);
	//这里因为HostBuffer的数据已经传给了DeviceBuffer，所以HostBuffer可以删掉了
	m_HostVertexBuffer = nullptr;

	//IndexBuffer
	ptr = Context::s_Context->m_Device.mapMemory(m_HostIndexBuffer->m_Memory, 0, m_HostIndexBuffer->m_Size);
	memcpy(ptr, indicies.data(), sizeof(indicies));
	Context::s_Context->m_Device.unmapMemory(m_HostIndexBuffer->m_Memory);
	CopyBuffer(m_HostIndexBuffer->m_Buffer, m_DeviceIndexBuffer->m_Buffer, m_HostIndexBuffer->m_Size, 0, 0);
	//这里因为HostBuffer的数据已经传给了DeviceBuffer，所以HostBuffer可以删掉了
	m_HostIndexBuffer = nullptr;
}

void Renderer::CreateUniformBuffer()
{
	m_HostUniformBuffer.resize(m_MaxFlightCount);
	m_DeviceUniformBuffer.resize(m_MaxFlightCount);
	for (auto& buffer : m_HostUniformBuffer)
	{
		buffer = CreateScope<Buffer>(sizeof(color),
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	}
	for (auto& buffer : m_DeviceUniformBuffer)
	{
		buffer = CreateScope<Buffer>(sizeof(color),
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal);
	}


	m_HostMVPBuffer.resize(m_MaxFlightCount);
	for (auto& buffer : m_HostMVPBuffer)
	{
		buffer = CreateScope<Buffer>(sizeof(m_MVP),
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	}
}

void Renderer::BufferUniformData()
{
	for (int i = 0;i < m_HostUniformBuffer.size();i++)
	{
		auto& hostBuffer = m_HostUniformBuffer[i];
		auto& deviceBuffer = m_DeviceUniformBuffer[i];
		void* ptr = Context::s_Context->m_Device.mapMemory(hostBuffer->m_Memory, 0, hostBuffer->m_Size);
		memcpy(ptr, glm::value_ptr(color), sizeof(color));
		Context::s_Context->m_Device.unmapMemory(hostBuffer->m_Memory);

		CopyBuffer(hostBuffer->m_Buffer, deviceBuffer->m_Buffer, hostBuffer->m_Size, 0, 0);
	}



	for (int i = 0;i < m_HostMVPBuffer.size();i++)
	{
		auto& hostBuffer = m_HostMVPBuffer[i];
		hostBuffer->m_Map = Context::s_Context->m_Device.mapMemory(hostBuffer->m_Memory, 0, hostBuffer->m_Size);
		memcpy(hostBuffer->m_Map, &m_MVP, sizeof(m_MVP));
	}
}

void Renderer::AllocateSets()
{
	m_DescriptorSets = Context::s_Context->m_DescriptorManager->AllocateUniformSets();
}

void Renderer::UpdateSets()
{
	for (int i = 0;i < m_DescriptorSets.size();i++)
	{
		auto set = m_DescriptorSets[i];

		std::array<vk::DescriptorBufferInfo, 2> bufferInfo;
		bufferInfo[0].setBuffer(m_DeviceUniformBuffer[i]->m_Buffer)
			.setOffset(0)
			.setRange(m_DeviceUniformBuffer[i]->m_Size);
		bufferInfo[1].setBuffer(m_HostMVPBuffer[i]->m_Buffer)
			.setOffset(0)
			.setRange(m_HostMVPBuffer[i]->m_Size);

		std::array<vk::WriteDescriptorSet, 2> writer;
		writer[0].setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(bufferInfo[0])
			.setDstBinding(0)
			.setDstSet(set)
			.setDstArrayElement(0)
			.setDescriptorCount(1);
		writer[1].setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(bufferInfo[1])
			.setDstBinding(1)
			.setDstSet(set)
			.setDstArrayElement(0)
			.setDescriptorCount(1);
		Context::s_Context->m_Device.updateDescriptorSets(writer, {});
	}
}

void Renderer::UpdateMVP()
{
	auto swapChainExtent = Context::s_Context->m_SwapChain->m_SwapChainInfo.ImageExtent;

	m_MVP.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * glm::scale(glm::mat4(1.0f), {3.0f, 3.0f, 3.0f});
	m_MVP.view = glm::translate(glm::mat4(1.0f), { 0.0, 0.0f, -10.0f });
	m_MVP.proj  = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 100.0f);
	m_MVP.proj[1][1] *= -1;

	memcpy(m_HostMVPBuffer[m_CurFrame]->m_Map, &m_MVP, sizeof(m_MVP));
}

void Renderer::CopyBuffer(vk::Buffer& src, vk::Buffer& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset)
{
	auto cmdBuf = Context::s_Context->m_CommandManager->CreateCommandBuffer();
	vk::CommandBufferBeginInfo begin;
	begin.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	cmdBuf.begin(begin);
	vk::BufferCopy region;
	region.setSize(size)
		.setSrcOffset(srcOffset)
		.setDstOffset(dstOffset);
	cmdBuf.copyBuffer(src, dst, region);
	cmdBuf.end();
	vk::SubmitInfo submit;
	submit.setCommandBuffers(cmdBuf);
	Context::s_Context->m_GraphicsQueue.submit(submit);

	Context::s_Context->m_Device.waitIdle();

	Context::s_Context->m_CommandManager->FreeCommandBuffer(cmdBuf);
}
