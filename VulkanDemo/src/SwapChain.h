#pragma once

#include <vulkan/vulkan.hpp>

class SwapChain
{
public:
	vk::SwapchainKHR m_Swapchain;

	struct SwapChainInfo
	{
		vk::Extent2D ImageExtent;
		uint32_t ImageCount;
		vk::SurfaceFormatKHR Format;
		vk::SurfaceTransformFlagsKHR Transform;
		vk::PresentModeKHR PresentMode;
	};
	SwapChainInfo m_SwapChainInfo;
	std::vector<vk::Image> m_Images;
	std::vector<vk::ImageView> m_ImageViews;
public:
	SwapChain(uint32_t width, uint32_t height);
	~SwapChain();

	void QueryInfo(uint32_t width, uint32_t height);
	void GetImages();
	void CreateImageViews();
private:
};