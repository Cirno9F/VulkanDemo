#pragma once

#include "../Base.h"
#include "DepthTexture.h"

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
	std::vector<vk::Framebuffer> m_FrameBuffers;

	Scope<DepthTexture> m_DepthTexture;
public:
	SwapChain(uint32_t width, uint32_t height);
	~SwapChain();

	void QueryInfo(uint32_t width, uint32_t height);
	void GetImages();
	void CreateImageViews();
	void CreateFrameBuffers(uint32_t width, uint32_t height);
private:
};