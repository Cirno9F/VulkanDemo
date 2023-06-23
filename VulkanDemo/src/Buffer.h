#pragma once

#include <vulkan/vulkan.hpp>

class Buffer
{
public:
	Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property);
	~Buffer();
public:
	vk::Buffer m_Buffer;
	vk::DeviceMemory m_Memory;
	void* m_Map;
	size_t m_Size;
private:
	struct MemoryInfo final
	{
		size_t size;
		uint32_t index;
	};

	void CreateBuffer(size_t size, vk::BufferUsageFlags usage);
	void AllocateMemory(MemoryInfo info);
	void BindingMemoryToBuffer();
	MemoryInfo QueryMemoryInfo(vk::Buffer buffer, vk::MemoryPropertyFlags property);
};