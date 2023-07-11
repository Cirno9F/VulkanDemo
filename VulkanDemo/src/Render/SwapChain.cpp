#include "SwapChain.h"
#include "Context.h"

#include <glm/glm.hpp>

SwapChain::SwapChain(uint32_t width, uint32_t height)
{
	vk::Format format = Texture::FindSupportedFormat(
		{ vk::Format::eD32Sfloat,vk::Format::eD32SfloatS8Uint,vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment
	);
	m_DepthTexture = CreateScope<Texture>(width, height, format);


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

	GetImages();
	CreateImageViews();
}

SwapChain::~SwapChain()
{
	for (auto& frameBuffer : m_FrameBuffers)
	{
		Context::s_Context->m_Device.destroyFramebuffer(frameBuffer);
	}
	for (auto& view : m_ImageViews)
	{
		Context::s_Context->m_Device.destroyImageView(view);
	}
	Context::s_Context->m_Device.destroySwapchainKHR(m_Swapchain);
	m_DepthTexture = nullptr;
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

void SwapChain::GetImages()
{
	m_Images = Context::s_Context->m_Device.getSwapchainImagesKHR(m_Swapchain);
}

void SwapChain::CreateImageViews()
{
	m_ImageViews.resize(m_Images.size());
	for (int i = 0;i < m_Images.size();i++)
	{
		vk::ImageViewCreateInfo createInfo;
		vk::ComponentMapping mapping;
		vk::ImageSubresourceRange range;
		range.setBaseMipLevel(0)
			.setLevelCount(1)
			.setBaseArrayLayer(0)
			.setLayerCount(1)
			.setAspectMask(vk::ImageAspectFlagBits::eColor);
		createInfo.setImage(m_Images[i])
			.setViewType(vk::ImageViewType::e2D)
			.setComponents(mapping)
			.setFormat(m_SwapChainInfo.Format.format)
			.setSubresourceRange(range);
		m_ImageViews[i] = Context::s_Context->m_Device.createImageView(createInfo);
	}
}

void SwapChain::CreateFrameBuffers(uint32_t width, uint32_t height)
{
	m_FrameBuffers.resize(m_Images.size());
	for (int i = 0;i < m_FrameBuffers.size();i++)
	{
		vk::FramebufferCreateInfo createInfo;
		std::array<vk::ImageView, 2> imageViews{ m_ImageViews[i], m_DepthTexture->GetImageView() };
		createInfo.setAttachments(imageViews)
			.setWidth(width)
			.setHeight(height)
			.setRenderPass(Context::s_Context->m_RenderProcess->m_RenderPass)
			.setLayers(1);
		m_FrameBuffers[i] = Context::s_Context->m_Device.createFramebuffer(createInfo);
	}
}


