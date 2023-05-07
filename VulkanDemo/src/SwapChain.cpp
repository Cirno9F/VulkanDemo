#include "SwapChain.h"
#include "Context.h"

#include <glm/glm.hpp>

SwapChain::SwapChain(uint32_t width, uint32_t height)
{
	QueryInfo(width, height);

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.setClipped(true)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setSurface(Context::s_Context->m_Surface)
		.setImageColorSpace(m_SwapChainInfo.Format.colorSpace)
		.setImageFormat(m_SwapChainInfo.Format.format)
		.setImageExtent(m_SwapChainInfo.ImageExtent)
		.setMinImageCount(m_SwapChainInfo.ImageCount)
		.setPresentMode(m_SwapChainInfo.PresentMode);

	auto& queueIndices = Context::s_Context->m_QueueFamilyIndices;
	if (queueIndices.PresentQueue.value() == queueIndices.GraphicsQueue.value())
	{
		createInfo.setQueueFamilyIndices(queueIndices.GraphicsQueue.value())
			.setImageSharingMode(vk::SharingMode::eExclusive);
	}
	else
	{
		std::array indices = { queueIndices.GraphicsQueue.value(), queueIndices.PresentQueue.value() };
		createInfo.setQueueFamilyIndices(indices)
			.setImageSharingMode(vk::SharingMode::eConcurrent);
	}
	
	m_Swapchain = Context::s_Context->m_Device.createSwapchainKHR(createInfo);
}

SwapChain::~SwapChain()
{
	Context::s_Context->m_Device.destroySwapchainKHR(m_Swapchain);
}

void SwapChain::QueryInfo(uint32_t width, uint32_t height)
{
	auto& physicalDevice = Context::s_Context->m_PhysicalDevice;
	auto& surface = Context::s_Context->m_Surface;
	auto formats = physicalDevice.getSurfaceFormatsKHR(surface);
	m_SwapChainInfo.Format = formats[0];
	for (const auto& format : formats)
	{
		if (format.format == vk::Format::eR8G8B8A8Srgb &&
			format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			m_SwapChainInfo.Format = format;
			break;
		}
	}

	auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
	m_SwapChainInfo.ImageCount = glm::clamp((uint32_t)2, capabilities.minImageCount, capabilities.maxImageCount);

	m_SwapChainInfo.ImageExtent.width = glm::clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	m_SwapChainInfo.ImageExtent.height = glm::clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	m_SwapChainInfo.Transform = capabilities.currentTransform;

	auto presents = physicalDevice.getSurfacePresentModesKHR(surface);
	m_SwapChainInfo.PresentMode = vk::PresentModeKHR::eFifo;
	for (const auto& present : presents)
	{
		if (present == vk::PresentModeKHR::eMailbox)
		{
			m_SwapChainInfo.PresentMode = present;
			break;
		}
	}

}


