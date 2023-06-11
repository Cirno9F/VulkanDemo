#pragma once

#include "vulkan/vulkan.hpp"

class RenderProcess
{
public:
	RenderProcess();
	RenderProcess(uint32_t width, uint32_t height);
	~RenderProcess();

	void InitPipeline(uint32_t width, uint32_t height);
	void DestroyPipeline();
public:
	vk::Pipeline m_Pipeline;
};