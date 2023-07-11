#pragma once

#include <vulkan/vulkan.hpp>

class Texture
{
public:
	Texture(const std::string& path, vk::Format format = vk::Format::eR8G8B8A8Unorm);
	Texture(uint32_t width, uint32_t height, vk::Format format = vk::Format::eR8G8B8A8Unorm);
	~Texture();

	static vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	vk::DescriptorSet& GetDescriptorSet() { return m_DescriptorSet; }
	const vk::ImageView& GetImageView() { return m_ImageView; }
	const vk::Format& GetFormat() { return m_Format; }
private:
	void Init(void* data, uint32_t width, uint32_t height);
	void GenerateMipmaps();
private:
	vk::Image m_Image;
	vk::DeviceMemory m_Memory;
	vk::ImageView m_ImageView;
	vk::DescriptorSet m_DescriptorSet = nullptr;
	vk::Sampler m_Sampler = nullptr;

	vk::Format m_Format;
	uint32_t m_MipLevels;
	uint32_t m_Width;
	uint32_t m_Height;
};