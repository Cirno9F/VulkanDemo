#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class DescriptorManager
{
public:
	DescriptorManager(uint32_t maxFlightCount);
	~DescriptorManager();

	std::vector<vk::DescriptorSet> AllocateUniformSets();
	vk::DescriptorSet AllocateImageSet();
	void FreeImageSet(vk::DescriptorSet imageSet);
private:
	uint32_t m_MaxFlightCount = 0;
	vk::DescriptorPool m_UniformPool;
	vk::DescriptorPool m_ImagePool;
};