#include "Buffer.h"
#include "Context.h"

Buffer::Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property)
	:m_Size(size)
{
	CreateBuffer(size, usage);
	auto info = QueryMemoryInfo(m_Buffer, property);
	AllocateMemory(info);
	BindingMemoryToBuffer();
}

Buffer::~Buffer()
{
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

void Buffer::AllocateMemory(MemoryInfo info)
{
	vk::MemoryAllocateInfo allocInfo;
	allocInfo.setMemoryTypeIndex(info.index)
		.setAllocationSize(info.size);
	m_Memory = Context::s_Context->m_Device.allocateMemory(allocInfo);
}

void Buffer::BindingMemoryToBuffer()
{
	Context::s_Context->m_Device.bindBufferMemory(m_Buffer, m_Memory, 0);
}

Buffer::MemoryInfo Buffer::QueryMemoryInfo(vk::Buffer buffer, vk::MemoryPropertyFlags property)
{
	MemoryInfo info;
	auto requirements = Context::s_Context->m_Device.getBufferMemoryRequirements(buffer);
	info.size = requirements.size;

	auto properties = Context::s_Context->m_PhysicalDevice.getMemoryProperties();
	for (int i = 0;i < properties.memoryTypeCount;i++)
	{
		if (((1 << i) & requirements.memoryTypeBits) &&
			(properties.memoryTypes[i].propertyFlags & property))
		{
			info.index = i;
			break;
		}
	}

	return info;
}
