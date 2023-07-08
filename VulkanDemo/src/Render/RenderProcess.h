#pragma once

#include "vulkan/vulkan.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct VertexInput
{
	glm::vec3 Position;
	glm::vec2 TexCoord;
	glm::vec3 Color;

	bool operator==(const VertexInput& other) const{
		return Position == other.Position && TexCoord == other.TexCoord && Color == other.Color;
	}
};

namespace std {
	template<> struct hash<VertexInput> {
		size_t operator()(VertexInput const& vertex) const {
			return ((hash<glm::vec3>()(vertex.Position) ^
				(hash<glm::vec3>()(vertex.Color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.TexCoord) << 1);
		}
	};
};

class RenderProcess
{
public:
	RenderProcess();
	RenderProcess(uint32_t width, uint32_t height);
	~RenderProcess();

	void InitLayout();
	void InitRenderPass();
	void InitPipeline(uint32_t width, uint32_t height);
	std::vector<vk::DescriptorSetLayout> CreateSetLayout();
public:
	vk::Pipeline m_Pipeline;
	vk::PipelineLayout m_PipelineLayout;
	vk::RenderPass m_RenderPass;

	std::vector<vk::DescriptorSetLayout> m_SetLayout;
};