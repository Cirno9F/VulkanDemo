#pragma once

#include "Log.h"
#include <string>
#include <fstream>
#include <vulkan/vulkan.hpp>

static class Utils
{
public:
	static std::string ReadFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);

		ASSERT_IFNOT(file.is_open(), "can not read file!");

		auto size = file.tellg();
		std::string content;
		content.resize(size);
		file.seekg(0);

		file.read(content.data(), content.size());

		return content;
	}

	static uint32_t QueryMemoryTypeIndex(vk::MemoryRequirements requirements, vk::MemoryPropertyFlags property, vk::PhysicalDeviceMemoryProperties properties)
	{
		uint32_t index = 0;

		for (int i = 0;i < properties.memoryTypeCount;i++)
		{
			if (((1 << i) & requirements.memoryTypeBits) &&
				(properties.memoryTypes[i].propertyFlags & property))
			{
				index = i;
				break;
			}
		}

		return index;
	}
};


