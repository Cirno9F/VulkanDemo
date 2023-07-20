#pragma once

#include "Texture.h"
#include "Buffer.h"
#include "RenderProcess.h"
#include "Mesh.h"
#include "../Base.h"

#include <vulkan/vulkan.hpp>
#include <vector>


struct Camera
{
	glm::vec3 EulerAngle = { -0.44f,0.0f,0.0f };
	float Distance = 5.9f;
};

struct Material
{
	glm::vec3 Albedo = { 1.0f,1.0f,1.0f };
	float Roughness = 1.0f;
	float Metallic = 0.0f;
	float Reflectance = 0.04f;
};

struct CommonBufferInfo
{
	glm::mat4 View = glm::mat4(1.0f);
	glm::mat4 Proj = glm::mat4(1.0f);
	glm::vec3 LightDir = glm::vec3{ 2.0f,-2.0f,-2.0f };
	glm::vec3 CameraPosition = glm::vec3{ 0.0f,0.0f,0.0f };
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
public:
	CommonBufferInfo m_CommonBufferInfo;
	Material m_PBRMaterial;
	Camera m_Camera;
private:
	void CreateCommandBuffers();
	void CreateSemaphores();
	void CreateFences();
	void CreateVertexBuffer();
	void BufferVertexData();
	void CreateUniformBuffer();
	void AllocateSets();
	void UpdateSets();

	void UpdateCommonBuffer();
	void UpdateMaterial();

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

	std::vector<Scope<Buffer>> m_HostMaterialBuffer;
	std::vector<Scope<Buffer>> m_DeviceMaterialBuffer;
	std::vector<Scope<Buffer>> m_HostCommonBuffer;
	std::vector<vk::DescriptorSet> m_DescriptorSets;

	uint32_t m_MaxFlightCount;
	uint32_t m_CurFrame;
	uint32_t m_ImageIndex;

	Scope<Texture> m_DefaultTexture;

	//viking room
	Scope<Texture> m_VikingRoomTexture;
	Ref<Mesh> m_DefaultMesh;
};