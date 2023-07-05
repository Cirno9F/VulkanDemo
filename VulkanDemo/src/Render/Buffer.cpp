#include "Buffer.h"
#include "Context.h"

Buffer::Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property)
	:m_Size(size), m_Map(nullptr)
{
	CreateBuffer(size, usage);
	auto requirements = Context::s_Context->m_Device.getBufferMemoryRequirements(m_Buffer);
	uint32_t index = Utils::QueryMemoryTypeIndex(requirements, property, Context::s_Context->m_PhysicalDevice.getMemoryProperties());
	AllocateMemory(index, requirements.size);
	BindingMemoryToBuffer();
}

Buffer::~Buffer()
{
	if (m_Map)
	{
		Context::s_Context->m_Device.unmapMemory(m_Memory);
		m_Map = nullptr;
	}
	Context::s_Context->m_Device.destroyBuffer(m_Buffer);
	Context::s_Context->m_Device.freeMemory(m_Memory);
}

void Buffer::CreateBuffer(size_t size, vk::BufferUsageFlags usage)
{
	vk::BufferCreateInfo createInfo;
	createInfo.setSize(size)
		.setUsage(usage)
		.setSharingMode(vk::SharingMode::eExclusive);

	m_Buffer = Context::s_Context->m_Device.createBuffer(createInfo);
}

void Buffer::AllocateMemory(uint32_t memoryTypeIndex, uint32_t size)
{
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(memoryTypeIndex)
		.setAllocationSize(size);
	m_Memory = Context::s_Context->m_Device.allocateMemory(allocInfo);
}

void Buffer::BindingMemoryToBuffer()
{
	Context::s_Context->m_Device.bindBufferMemory(m_Buffer, m_Memory, 0);
}


