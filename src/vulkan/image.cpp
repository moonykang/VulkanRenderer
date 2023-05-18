#include "vulkan/context.h"
#include "vulkan/image.h"
#include "vulkan/transition.h"

namespace vk
{
inline VkAccessFlags getMemoryAccessFlagsMask(const VkImageLayout& layout)
{
    VkAccessFlags flags = 0;
    switch (layout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        flags = VK_ACCESS_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        flags = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        flags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        flags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
        flags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
        flags = VK_ACCESS_MEMORY_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_GENERAL:
    case VK_IMAGE_LAYOUT_UNDEFINED:
        flags = 0;
        break;
    default:
        ASSERT(false, "undefined layout");
        break;
    }
    return flags;
};

#ifndef VK_IMAGE_LAYOUT_RANGE_SIZE
#define VK_IMAGE_LAYOUT_RANGE_SIZE (VK_IMAGE_LAYOUT_PREINITIALIZED - VK_IMAGE_LAYOUT_UNDEFINED + 1)
#endif

static const VkPipelineStageFlagBits GVulkanPipelineStage[VK_IMAGE_LAYOUT_RANGE_SIZE] =
{
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // VK_IMAGE_LAYOUT_UNDEFINED = 0,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // VK_IMAGE_LAYOUT_GENERAL = 1,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2,
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, // VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4,
    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5,
    VK_PIPELINE_STAGE_TRANSFER_BIT, // VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6,
    VK_PIPELINE_STAGE_TRANSFER_BIT, // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7,
    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,// VK_IMAGE_LAYOUT_PREINITIALIZED = 8,
};

Image::Image()
    : extent({})
    , format(VK_FORMAT_UNDEFINED)
    , subresourceRange({})
    , imageLayout(ImageLayout::Undefined)
{

}

void Image::destroy(VkDevice device)
{
    memory.destroy(device);
    sampler.destroy(device);
    destroyImageView(device);
    destroyImage(device);
}

bool Image::createImageView(VkDevice device, VkFormat format, VkImageAspectFlags imageAspectFlags)
{
    ASSERT(image.valid());

    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;

    VkComponentMapping components = {
        VK_COMPONENT_SWIZZLE_R,
        VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B,
        VK_COMPONENT_SWIZZLE_A
    };

    VkImageSubresourceRange subresourceRange = {
        imageAspectFlags, 0, 1, 0, 1
    };

    return createImageView(device, format, components, subresourceRange, viewType);
}

bool Image::createImageView(VkDevice device, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange, VkImageViewType viewType)
{
    ASSERT(image.valid());

    VkImageViewCreateInfo imageViewCreateInfo = {};
    imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCreateInfo.pNext = NULL;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.components = components;
    imageViewCreateInfo.subresourceRange = subresourceRange;
    imageViewCreateInfo.viewType = viewType;
    imageViewCreateInfo.flags = 0;
    imageViewCreateInfo.image = image.getHandle();

    VKCALL(view.init(device, imageViewCreateInfo));

    this->subresourceRange = subresourceRange;

    // TODO: Stencil readable view is needed
    if (subresourceRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
    {
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        VKCALL(readView.init(device, imageViewCreateInfo));
    }

    return true;
}

bool Image::createImage(Context* context,
                        VkFormat format,
                        uint32_t mipLevels,
                        uint32_t layers,
                        uint32_t samples,
                        VkExtent3D extent,
                        VkImageUsageFlags imageUsage,
                        VkMemoryPropertyFlags memoryProperty)
{
	this->format = format;
    this->samples = samples;

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = layers;
	imageCreateInfo.samples = VkSampleCountFlagBits(samples);
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { extent.width, extent.height, extent.depth };
	imageCreateInfo.usage = imageUsage;

	VKCALL(image.init(context->getDevice(), imageCreateInfo));

	VkMemoryRequirements memRequirements;
	image.getMemoryRequirements(context->getDevice(), &memRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(context->getPhysicalDevice(),memRequirements.memoryTypeBits, memoryProperty);

	VKCALL(memory.allocate(context->getDevice(), memoryAllocateInfo));
	VKCALL(image.bindMemory(context->getDevice(), memory));

	return true;
}

uint32_t Image::getMemoryTypeIndex(VkPhysicalDevice physicalDevice, const uint32_t memoryTypeBits, const VkMemoryPropertyFlags memoryProperty)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & memoryProperty) == memoryProperty)
        {
            return i;
        }
    }

    UNREACHABLE();
    return 0;
}

void Image::destroyImage(VkDevice device)
{
	ASSERT(image.valid());
	image.destroy(device);
}

void Image::setHandle(VkImage inImage, VkFormat format, VkExtent2D extent)
{
	ASSERT(!image.valid());
	image.setHandle(inImage);
	this->format = format;
	this->extent = { extent.width, extent.height , 1 };
	this->subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    this->samples = 1;
}

void Image::release()
{
	ASSERT(image.valid());
	image.release();
}

void Image::destroyImageView(VkDevice device)
{
	ASSERT(view.valid());
	view.destroy(device);
    readView.destroy(device);
}

void Image::updateImageLayout(ImageLayout newImageLayout)
{
	imageLayout = newImageLayout;
}

Transition* Image::updateImageLayoutAndBarrier(ImageLayout newLayout)
{
	if (newLayout == imageLayout)
	{
		return nullptr;
	}

	ImageLayout oldLayout = imageLayout;

	imageLayout = newLayout;
	return new Transition(oldLayout, newLayout, image.getHandle(), subresourceRange);
}

VkImage Image::getImage()
{
	ASSERT(image.valid());
	return image.getHandle();
}

VkImageView Image::getImageView()
{
	ASSERT(view.valid());
	return view.getHandle();
}

VkImageView Image::getReadableImageView()
{
    if (readView.valid())
    {
        return readView.getHandle();
    }
    else
    {
        return getImageView();
    }
}

VkSampler Image::getSampler()
{
    ASSERT(sampler.valid());
    return sampler.getHandle();
}

VkFormat Image::getFormat()
{
    return format;
}

uint32_t Image::getSamples()
{
    return samples;
}

VkImageAspectFlags Image::getImageAspect() { return subresourceRange.aspectMask; }

VkImageSubresourceRange Image::getSubresourceRange()
{
	return subresourceRange;
}
}