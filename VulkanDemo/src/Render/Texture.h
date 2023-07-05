#pragma once

#include <vulkan/vulkan.hpp>

class Texture
{
public:
	Texture(const std::string& path);
	~Texture();

	vk::DescriptorSet& GetDescriptorSet() { return m_DescriptorSet; }
private:
	void Init(void* data, uint32_t width, uint32_t height);
private:
	vk::Image m_Image;
	vk::DeviceMemory m_Memory;
	vk::ImageView m_ImageView;
	vk::DescriptorSet m_DescriptorSet;
	vk::Sampler m_Sampler;
};