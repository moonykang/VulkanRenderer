#include "rhi/context.h"
#include "vulkan/buffer.h"
#include "vulkan/texture.h"

namespace vk
{
Texture::Texture(Format format, uint32_t width, uint32_t height, ImageLayout initialLayout, uint32_t usage)
    : rhi::Texture(format, width, height, initialLayout, usage)
    , buffer(nullptr)
    , descriptorImageInfo()
{

}

Texture::Texture(Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels, uint32_t layers, ImageLayout initialLayout, uint32_t usage)
    : rhi::Texture(format, width, height, depth, samples, mipLevels, layers, initialLayout, usage)
    , buffer(nullptr)
    , descriptorImageInfo()
{

}

void Texture::destroy(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    Image::destroy(contextVk->getDevice());

    if (buffer != nullptr)
    {
        buffer->destroy(contextVk);
        delete buffer;
        buffer = nullptr;
    }
}

void Texture::build(rhi::Context* rhiContext)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);
    VkFormat format = convertToVkFormat(rhi::Texture::format);
    VkExtent3D extent = { width, height, depth };
    createImage(context, format, mipLevels, layers, rhi::Texture::samples, extent, usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkComponentMapping components = {};
    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = getImageAspectMask(format);
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = mipLevels;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = layers;

    auto physicalDeviceProps = context->getPhysicalDeviceProperties();
    float maxSamplerAnisotropy = physicalDeviceProps.limits.maxSamplerAnisotropy;

    createSampler(context->getDevice(),
                  convertToVkFilter(samplerInfo.getMagFilter()),
                  convertToVkFilter(samplerInfo.getMinFilter()),
                  convertToVkSamplerMipmapMode(samplerInfo.getMipmapMode()),
                  samplerInfo.getAnisotropyEnable(),
                  maxSamplerAnisotropy,
                  samplerInfo.getUnnormalizedCoordinates());

    createImageView(context->getDevice(), format, components, subresourceRange, getImageViewType(width, height, depth));

    CommandBuffer* commandBuffer = context->getUploadCommandBuffer();

    if (textureLoaded)
    {
        ASSERT(memoryBuffer.size() != 0);
        buffer = BufferFactory::createBuffer(rhi::BufferType::HostCoherent, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 0, memoryBuffer.size());
        buffer->init(context, memoryBuffer.data());

        commandBuffer->addTransition(updateImageLayoutAndBarrier(ImageLayout::TransferDst));
        commandBuffer->flushTransitions();

        if (mipOffsets.empty())
        {
            Copy(context, buffer->getBuffer(), extent);
        }
        else
        {
            for (auto& offset : mipOffsets)
            {
                uint32_t mipLevel = offset.first;
                size_t bufferOffset = offset.second;

                VkExtent3D copyExtent = extent;
                copyExtent.width = extent.width >> mipLevel;
                copyExtent.height = extent.height >> mipLevel;
                copyExtent.depth = extent.depth;

                Copy(context, buffer->getBuffer(), copyExtent, mipLevel, 0, bufferOffset);
            }
        }
        commandBuffer->addTransition(updateImageLayoutAndBarrier(ImageLayout::FragmentShaderReadOnly));
    }
    else
    {        
        commandBuffer->addTransition(updateImageLayoutAndBarrier(initialLayout));
    }

    context->submitUploadCommandBuffer();
    clear();
}

void Texture::Copy(Context* context, VkBuffer srcBuffer, VkExtent3D extent)
{
    Copy(context, srcBuffer, extent, 0, 0, 0);
}

void Texture::Copy(Context* context, VkBuffer srcBuffer, VkExtent3D extent, uint32_t mipLevel, uint32_t layer, size_t bufferOffset)
{
    VkBufferImageCopy copyRegion = {};
    copyRegion.imageSubresource.aspectMask = subresourceRange.aspectMask;
    copyRegion.imageSubresource.mipLevel = mipLevel;
    copyRegion.imageSubresource.baseArrayLayer = layer;
    copyRegion.imageSubresource.layerCount = 1;

    copyRegion.imageOffset = { 0, 0, 0 };
    copyRegion.imageExtent = extent;
    copyRegion.bufferOffset = bufferOffset;

    CommandBuffer* commandBuffer = context->getUploadCommandBuffer();
    commandBuffer->copyBufferToImage(srcBuffer, image.getHandle(), copyRegion);
}

void* Texture::getDescriptorData(rhi::DescriptorType type)
{
    VkFormat format = getFormat();
    auto aspectMask = getImageAspectMask(format);

    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_GENERAL;

    if (type != rhi::DescriptorType::Storage_Image)
    {
        imageLayout = (aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) != 0 ?
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL :
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }

    descriptorImageInfo.imageView = getReadableImageView();
    descriptorImageInfo.sampler = getSampler();

    descriptorImageInfo.imageLayout = imageLayout;

    return &descriptorImageInfo;
}

void Texture::clearColor(rhi::Context* rhiContext, float r, float g, float b, float a)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);

    auto aspectMask = getImageAspectMask(getFormat());

    if (aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
    {
            VkImageSubresourceRange range = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

            VkClearColorValue color;

            color.float32[0] = r;
            color.float32[1] = g;
            color.float32[2] = b;
            color.float32[3] = a;

            CommandBuffer* commandBuffer = context->getActiveCommandBuffer();
            commandBuffer->clearColorImage(getImage(), getVkImageLayout(), color, range);
    }
    else
    {
        LOGE("Invalid clear command: It is not color image");
    }
}

void Texture::CopyTo(rhi::Context* rhiContext, rhi::Texture* rhiDstTexture, uint32_t srcMipLevel, uint32_t dstMipLevel)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);
    Texture* dstTexture = reinterpret_cast<Texture*>(rhiDstTexture);

    ImageLayout srcLayout = imageLayout;
    ImageLayout dstLayout = dstTexture->getImageLayout();

    CommandBuffer* commandBuffer = context->getActiveCommandBuffer();
    commandBuffer->addTransition(updateImageLayoutAndBarrier(ImageLayout::TransferSrc));
    commandBuffer->addTransition(dstTexture->updateImageLayoutAndBarrier(ImageLayout::TransferDst));

    // TODO: assume mip0 > mip0 at this time
    VkImageCopy copyRegion {};
    copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.srcSubresource.layerCount = 1;
    copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.dstSubresource.layerCount = 1;
    copyRegion.extent.width              = extent.width;
    copyRegion.extent.height             = extent.height;
    copyRegion.extent.depth              = extent.depth;

    commandBuffer->copyImage(getImage(), dstTexture->getImage(), copyRegion);

    commandBuffer->addTransition(updateImageLayoutAndBarrier(srcLayout));
    commandBuffer->addTransition(dstTexture->updateImageLayoutAndBarrier(dstLayout));
}
}