#pragma once

#include "../Utils.h"
#include <vulkan/vulkan.hpp>

class Buffer
{
public:
	Buffer(size_t size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags property);
	~Buffer();
public:
	vk::Buffer m_Buffer;
	vk::DeviceMemory m_Memory;
	void* m_Map = nullptr;
	size_t m_Size;
private:
	void CreateBuffer(size_t size, vk::BufferUsageFlags usage);
	void AllocateMemory(uint32_t memoryTypeIndex, uint32_t size);
	void BindingMemoryToBuffer();
};