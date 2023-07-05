#pragma once

#include "../Base.h"
#include "vulkan/vulkan.hpp"

class Shader
{
public:
	static Ref<Shader> Create(const std::string& vertexSource, const std::string& fragSource);
	Shader(const std::string& vertexSource, const std::string& fragSource);
	~Shader();
	void InitStages();
public:
	std::vector<vk::PipelineShaderStageCreateInfo> m_Stages;
	vk::ShaderModule m_VertexShader;
	vk::ShaderModule m_FragmentShader;

private:
};