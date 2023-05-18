#pragma once

#include "rhi/texture.h"
#include "vulkan/image.h"
#include "vulkan/resources.h"

namespace rhi
{
class Context;
};

namespace vk
{
const VkImageUsageFlags kTextureImageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
const VkImageUsageFlags kRenderTargetImageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;


class Context;
class Image;
class Buffer;

class Texture : public rhi::Texture, public Image 
{
public:
    Texture(Format format, uint32_t width, uint32_t height, ImageLayout initialLayout, uint32_t usage);

    Texture(Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels, uint32_t layers, ImageLayout initialLayout, uint32_t usage);

    void destroy(rhi::Context* context) override;

    void build(rhi::Context* context) override;

    void* getDescriptorData(rhi::DescriptorType type) override final;

    void clearColor(rhi::Context* context, float r, float g, float b, float a) override;

    void CopyTo(rhi::Context* context, rhi::Texture* dstTexture, uint32_t srcMipLevel, uint32_t dstMipLevel) override;
public:
    void Copy(Context* context, VkBuffer srcBuffer, VkExtent3D extent);

    void Copy(Context* context, VkBuffer srcBuffer, VkExtent3D extent, uint32_t mipLevel, uint32_t layer, size_t bufferOffset);

protected:
    vk::Buffer* buffer;

    VkDescriptorImageInfo descriptorImageInfo;
};
}