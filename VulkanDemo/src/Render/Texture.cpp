#include "Texture.h"
#include "Buffer.h"
#include "Context.h"
#include "stb_image.h"
#include "../Base.h"
#include "../Utils.h"

Texture::Texture(const std::string& path, vk::Format format) : m_Format(format)
{
    int w, h, channel;
    stbi_uc* pixels = stbi_load(path.c_str(), &w, &h, &channel, STBI_rgb_alpha);
    size_t size = w * h * 4;
    ASSERT_IFNOT(pixels, "image load failed: {0}", path);
    Init(pixels, w, h);
    stbi_image_free(pixels);
}

Texture::Texture(uint32_t width, uint32_t height, vk::Format format)
{
    auto device = Context::s_Context->m_Device;
    auto phyDevice = Context::s_Context->m_PhysicalDevice;
    auto& cmdMgr = Context::s_Context->m_CommandManager;

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

Texture::~Texture()
{
    auto device = Context::s_Context->m_Device;
    device.destroyImageView(m_ImageView);
    device.freeMemory(m_Memory);
    device.destroyImage(m_Image);

    if(m_DescriptorSet)
        Context::s_Context->m_DescriptorManager->FreeImageSet(m_DescriptorSet);
    if(m_Sampler)
        device.destroySampler(m_Sampler);
}

void Texture::Init(void* data, uint32_t width, uint32_t height)
{
    auto device = Context::s_Context->m_Device;
    auto phyDevice = Context::s_Context->m_PhysicalDevice;
    auto& cmdMgr = Context::s_Context->m_CommandManager;

    //caculate miplevels;
    m_MipLevels = 1;//static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
    m_Width = width;
    m_Height = height;

    //stage buffer
    const uint32_t size = width * height * 4;
    Scope<Buffer> buffer = CreateScope<Buffer>(size, 
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
    memcpy(buffer->m_Map, data, size);

    //create image
    vk::ImageCreateInfo createInfo;
    createInfo.setImageType(vk::ImageType::e2D)
        .setArrayLayers(1)
        .setMipLevels(m_MipLevels)
        .setExtent({ width, height, 1 })
        .setFormat(m_Format)
        .setTiling(vk::ImageTiling::eOptimal)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
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

    //set immage layout from undefine to dst
    cmdMgr->ExcuteCommand(Context::s_Context->m_GraphicsQueue,
        [&](vk::CommandBuffer cmdBuf) {
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;
            range.setLayerCount(1)
                .setBaseArrayLayer(0)
                .setLevelCount(m_MipLevels)
                .setBaseMipLevel(0)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
            barrier.setImage(m_Image)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstAccessMask((vk::AccessFlagBits::eTransferWrite))
                .setSubresourceRange(range);
            cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer,
                {}, {}, nullptr, barrier);
        });

    //transfer data from stage buffer to image
    cmdMgr->ExcuteCommand(Context::s_Context->m_GraphicsQueue,
        [&](vk::CommandBuffer cmdBuf) {
            vk::BufferImageCopy region;
            vk::ImageSubresourceLayers subsource;
            subsource.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setMipLevel(0)
                .setLayerCount(1);
            region.setBufferImageHeight(0)
                .setBufferOffset(0)
                .setImageOffset(0)
                .setImageExtent({ width, height, 1 })
                .setBufferRowLength(0)
                .setImageSubresource(subsource);
            cmdBuf.copyBufferToImage(buffer->m_Buffer, m_Image,
                vk::ImageLayout::eTransferDstOptimal,
                region);
        });

    //set immage layout from dst to optimal
    cmdMgr->ExcuteCommand(Context::s_Context->m_GraphicsQueue,
        [&](vk::CommandBuffer cmdBuf) {
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;
            range.setLayerCount(1)
                .setBaseArrayLayer(0)
                .setLevelCount(m_MipLevels)
                .setBaseMipLevel(0)
                .setAspectMask(vk::ImageAspectFlagBits::eColor);
            barrier.setImage(m_Image)
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSrcAccessMask((vk::AccessFlagBits::eTransferWrite))
                .setDstAccessMask((vk::AccessFlagBits::eShaderRead))
                .setSubresourceRange(range);
            cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                {}, {}, nullptr, barrier);
        });

    //generate mipmaps
    //GenerateMipmaps();

    //create image view
    vk::ImageViewCreateInfo imCreateInfo;
    vk::ComponentMapping mapping;
    vk::ImageSubresourceRange range;
    range.setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setBaseArrayLayer(0)
        .setLayerCount(1)
        .setLevelCount(m_MipLevels)
        .setBaseMipLevel(0);
    imCreateInfo.setImage(m_Image)
        .setViewType(vk::ImageViewType::e2D)
        .setComponents(mapping)
        .setFormat(m_Format)
        .setSubresourceRange(range);
    m_ImageView = device.createImageView(imCreateInfo);

    //create sampler
    vk::SamplerCreateInfo samplerCreateInfo;
    samplerCreateInfo.setMagFilter(vk::Filter::eLinear)
        .setMinFilter(vk::Filter::eLinear)
        .setAddressModeU(vk::SamplerAddressMode::eRepeat)
        .setAddressModeV(vk::SamplerAddressMode::eRepeat)
        .setAddressModeW(vk::SamplerAddressMode::eRepeat)
        .setAnisotropyEnable(false)
        .setBorderColor(vk::BorderColor::eIntOpaqueBlack)
        .setUnnormalizedCoordinates(false)
        .setCompareEnable(false)
        .setMipmapMode(vk::SamplerMipmapMode::eLinear);
    m_Sampler = device.createSampler(samplerCreateInfo);

    //allocate image descriptor set
    m_DescriptorSet = Context::s_Context->m_DescriptorManager->AllocateImageSet();

    //update descriptor set
    vk::WriteDescriptorSet writer;
    vk::DescriptorImageInfo imageInfo;
    imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
        .setImageView(m_ImageView)
        .setSampler(m_Sampler);
    writer.setImageInfo(imageInfo)
        .setDstBinding(0)
        .setDstArrayElement(0)
        .setDstSet(m_DescriptorSet)
        .setDescriptorCount(1)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
    device.updateDescriptorSets(writer, {});
}

void Texture::GenerateMipmaps()
{
    auto& cmdMgr = Context::s_Context->m_CommandManager;
    cmdMgr->ExcuteCommand(Context::s_Context->m_GraphicsQueue,
        [&](vk::CommandBuffer cmdBuf) {
            vk::ImageMemoryBarrier barrier;
            vk::ImageSubresourceRange range;
            range.setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setBaseArrayLayer(0)
                .setLayerCount(1)       
                .setLevelCount(1);
            barrier.setImage(m_Image)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setSubresourceRange(range);


            int32_t mipWidth = m_Width;
            int32_t mipHeight = m_Height;

            for (uint32_t i = 1; i < m_MipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
                barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
                barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

                cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer,
                    {}, {}, nullptr, barrier);
            }


            cmdBuf.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                {}, {}, nullptr, barrier);
        });
}


vk::Format Texture::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
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
