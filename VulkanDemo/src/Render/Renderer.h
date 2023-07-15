#pragma once

#include "Texture.h"
#include "Buffer.h"
#include "RenderProcess.h"
#include "Mesh.h"
#include "../Base.h"

#include <vulkan/vulkan.hpp>
#include <vector>

struct ViewProj
{
	glm::mat4 view;
	glm::mat4 proj;
};

class Renderer
{
public:
	Renderer(uint32_t maxFlightCount = 2);
	~Renderer();
	void Begin();
	void End();
	void DrawTriangle();
	vk::CommandBuffer GetFrameCmd() { return m_CommandBuffers[m_CurFrame]; }
	uint32_t GetImageIndex() { return m_ImageIndex; }
private:
	void CreateCommandBuffers();
	void CreateSemaphores();
	void CreateFences();
	void CreateVertexBuffer();
	void BufferVertexData();
	void CreateUniformBuffer();
	void BufferUniformData();
	void AllocateSets();
	void UpdateSets();

	void UpdateMVP();

	void CopyBuffer(vk::Buffer& src, vk::Buffer& dst, uint32_t size, uint32_t srcOffset, uint32_t dstOffset);
private:
    std::vector<vk::CommandBuffer> m_CommandBuffers;
	std::vector<vk::Semaphore> m_SemaphoreImageAvaliables;
	std::vector<vk::Semaphore> m_SemaphoreImageDrawFinishs;
	std::vector<vk::Fence> m_Fences;

	Scope<Buffer> m_HostVertexBuffer;
	Scope<Buffer> m_DeviceVertexBuffer;
	Scope<Buffer> m_HostIndexBuffer;
	Scope<Buffer> m_DeviceIndexBuffer;

	std::vector<Scope<Buffer>> m_HostUniformBuffer;
	std::vector<Scope<Buffer>> m_DeviceUniformBuffer;
	std::vector<Scope<Buffer>> m_HostMVPBuffer;
	std::vector<vk::DescriptorSet> m_DescriptorSets;

	uint32_t m_MaxFlightCount;
	uint32_t m_CurFrame;
	uint32_t m_ImageIndex;

	ViewProj m_ViewProj;
	glm::mat4 m_Model;

	Scope<Texture> m_DefaultTexture;

	//viking room
	Scope<Texture> m_VikingRoomTexture;
	Ref<Mesh> m_DefaultMesh;
};