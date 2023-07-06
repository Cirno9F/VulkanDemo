#pragma once


#include <vulkan/vulkan.hpp>
//todo: 之后要合并到Texture中去

class DepthTexture
{
public:
	DepthTexture(uint32_t width, uint32_t height);
	~DepthTexture();
	vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	const vk::ImageView& GetImageView() { return m_ImageView; }
	const vk::Format& GetFormat() { return m_Format; }
private:
	vk::Image m_Image;
	vk::DeviceMemory m_Memory;
	vk::ImageView m_ImageView;
	vk::Format m_Format;
};