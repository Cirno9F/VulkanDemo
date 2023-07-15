#pragma once

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

struct VertexInput
{
	glm::vec3 Position;
	glm::vec2 TexCoord;
	glm::vec3 Normal;
	glm::vec3 Color;

	bool operator==(const VertexInput& other) const {
		return Position == other.Position && TexCoord == other.TexCoord && Color == other.Color;
	}

	static std::vector<vk::VertexInputAttributeDescription> GetAttribute()
	{
		std::vector<vk::VertexInputAttributeDescription> attribute;
		attribute.resize(4);
		attribute[0].setBinding(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setLocation(0)
			.setOffset(0);
		attribute[1].setBinding(0)
			.setFormat(vk::Format::eR32G32Sfloat)
			.setLocation(1)
			.setOffset(offsetof(VertexInput, TexCoord));
		attribute[2].setBinding(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setLocation(2)
			.setOffset(0);
		attribute[3].setBinding(0)
			.setFormat(vk::Format::eR32G32B32Sfloat)
			.setLocation(3)
			.setOffset(offsetof(VertexInput, Color));
		return attribute;
	}

	static vk::VertexInputBindingDescription GetBinding()
	{
		vk::VertexInputBindingDescription binding;

		binding.setBinding(0)
			.setInputRate(vk::VertexInputRate::eVertex)
			.setStride(sizeof(VertexInput));

		return binding;
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

class Mesh
{
public:
	Mesh(const std::string& path);
	~Mesh();

	const std::vector<uint32_t>& const GetIndices() { return m_Indices; }
	const std::vector<VertexInput>& const GetVertices() { return m_Vertices; }
private:
	std::vector<VertexInput> m_Vertices;
	std::vector<uint32_t> m_Indices;
};