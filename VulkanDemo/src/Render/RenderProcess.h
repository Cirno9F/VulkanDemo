#pragma once

#include "vulkan/vulkan.hpp"

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