#include "DepthTexture.h"
#include "Context.h"

DepthTexture::DepthTexture(uint32_t width, uint32_t height)
{
    auto device = Context::s_Context->m_Device;
    auto phyDevice = Context::s_Context->m_PhysicalDevice;
    auto& cmdMgr = Context::s_Context->m_CommandManager;

    vk::Format format = FindSupportedFormat(
        { vk::Format::eD32Sfloat,vk::Format::eD32SfloatS8Uint,vk::Format::eD24UnormS8Uint },
        vk::ImageTiling::eOptimal,
        vk::FormatFeatureFlagBits::eDepthStencilAttachment
    );
    m_Format = format;

    //create image
    vk::ImageCreateInfo createInfo;
    createInfo.setImageType(vk::ImageType::e2D)
        .setArrayLayers(1)
        .setMipLevels(1)
        .setExtent({ width, height, 1 })
        .setFormat(format)
        .setTiling(vk::ImageTiling::eOptimal)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
        .setSamples(vk::SampleCountFlagBits::e1);
    m_Image = device.createImage(createInfo);

    //allocate device memory
    vk::MemoryAllocateInfo allocInfo;
    auto requirements = device.getImageMemoryRequirements(m_Image);
    allocInfo.setAllocationSize(requirements.size);
    auto index = Utils::QueryMemoryTypeIndex(requirements, vk::MemoryPropertyFlagBits::eDeviceLocal, phyDevice.getMemoryProperties());
    allocInfo.setMemoryTypeIndex(index);
    m_Memory = device.allocateMemory(allocInfo);

    //bind image and memory
    device.bindImageMemory(m_Image, m_Memory, 0);

    //create image view
    vk::ImageViewCreateInfo imCreateInfo;
    vk::ComponentMapping mapping;
    vk::ImageSubresourceRange range;
    range.setAspectMask(vk::ImageAspectFlagBits::eDepth)
        .setBaseArrayLayer(0)
        .setLayerCount(1)
        .setLevelCount(1)
        .setBaseMipLevel(0);
    imCreateInfo.setImage(m_Image)
        .setViewType(vk::ImageViewType::e2D)
        .setComponents(mapping)
        .setFormat(format)
        .setSubresourceRange(range);
    m_ImageView = device.createImageView(imCreateInfo);

    //set immage layout from undefine to dst
    cmdMgr->ExcuteCommand(Context::s_Context->m_GraphicsQueue,
        [&](vk::CommandBuffer cmdBuf) {
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;
            range.setLayerCount(1)
                .setBaseArrayLayer(0)
                .setLevelCount(1)
                .setBaseMipLevel(0)
                .setAspectMask(vk::ImageAspectFlagBits::eDepth);
            barrier.setImage(m_Image)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstAccessMask((vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite))
                .setSubresourceRange(range);
            cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eEarlyFragmentTests,
                {}, {}, nullptr, barrier);
        });
}

DepthTexture::~DepthTexture()
{
    auto device = Context::s_Context->m_Device;
    device.destroyImageView(m_ImageView);
    device.freeMemory(m_Memory);
    device.destroyImage(m_Image);
}

vk::Format DepthTexture::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    auto phyDevice = Context::s_Context->m_PhysicalDevice;

    for (vk::Format format : candidates) {
        vk::FormatProperties props = phyDevice.getFormatProperties(format);

        if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    ASSERT("failed to find supported format!");
    return vk::Format::eD16Unorm;
}
