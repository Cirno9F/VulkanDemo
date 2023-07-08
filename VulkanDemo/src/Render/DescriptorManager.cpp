#include "DescriptorManager.h"
#include "Context.h"

DescriptorManager::DescriptorManager(uint32_t maxFlightCount) : m_MaxFlightCount(maxFlightCount)
{
	vk::DescriptorPoolCreateInfo createInfo;
	vk::DescriptorPoolSize poolSize;
	poolSize.setType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(m_MaxFlightCount);
	createInfo.setMaxSets(m_MaxFlightCount)
		.setPoolSizes(poolSize);
	m_UniformPool = Context::s_Context->m_Device.createDescriptorPool(createInfo);


	poolSize.setType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(2);
	createInfo.setMaxSets(2)
		.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
		.setPoolSizes(poolSize);
	m_ImagePool = Context::s_Context->m_Device.createDescriptorPool(createInfo);
}

DescriptorManager::~DescriptorManager()
{
	Context::s_Context->m_Device.destroyDescriptorPool(m_UniformPool);
	Context::s_Context->m_Device.destroyDescriptorPool(m_ImagePool);
}

std::vector<vk::DescriptorSet> DescriptorManager::AllocateUniformSets()
{
	std::vector<vk::DescriptorSetLayout> layouts(m_MaxFlightCount, Context::s_Context->m_RenderProcess->m_SetLayout[0]);
	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(m_UniformPool)
		.setDescriptorSetCount(m_MaxFlightCount)
		.setSetLayouts(layouts);

	return Context::s_Context->m_Device.allocateDescriptorSets(allocInfo);
}

vk::DescriptorSet DescriptorManager::AllocateImageSet()
{
	std::vector<vk::DescriptorSetLayout> layouts{ Context::s_Context->m_RenderProcess->m_SetLayout[1] };
	vk::DescriptorSetAllocateInfo allocInfo;
	allocInfo.setDescriptorPool(m_ImagePool)
	.setDescriptorSetCount(1)
	.setSetLayouts(layouts);
	auto sets = Context::s_Context->m_Device.allocateDescriptorSets(allocInfo);

	return sets[0];
}

void DescriptorManager::FreeImageSet(vk::DescriptorSet imageSet)
{
	if(m_ImagePool)
		Context::s_Context->m_Device.freeDescriptorSets(m_ImagePool, imageSet);
}
