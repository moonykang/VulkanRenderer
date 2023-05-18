#pragma once

#include "vulkan/vk_wrapper.h"
#include "vulkan/resources.h"
#include "vulkan/transition.h"

namespace vk
{
class Context;
class Image
{
public:
	Image();

	void destroy(VkDevice device);

	bool createImage(Context* context, VkFormat format, uint32_t mipLevels, uint32_t layers, uint32_t samples, VkExtent3D extent, VkImageUsageFlags imageUsage, VkMemoryPropertyFlags memoryProperty);

	void destroyImage(VkDevice device);

	void setHandle(VkImage inImage, VkFormat format, VkExtent2D extent);

	void release();

	uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, const uint32_t memoryTypeBits, const VkMemoryPropertyFlags memoryProperty);

	bool createImageView(VkDevice device, VkFormat format, VkImageAspectFlags aspectFlags);

	bool createImageView(VkDevice device, VkFormat format, VkComponentMapping components, VkImageSubresourceRange subresourceRange, VkImageViewType viewType);

	void createSampler(VkDevice device, VkFilter magFilter, VkFilter minFilter, VkSamplerMipmapMode mipmapMode, bool anisotropyEnable, float maxAnisotropy, bool unnormalizedCoordinates)
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.pNext = nullptr;
		samplerCreateInfo.magFilter = magFilter;
		samplerCreateInfo.minFilter = minFilter;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.anisotropyEnable = anisotropyEnable && maxAnisotropy > 1.0f;
		samplerCreateInfo.maxAnisotropy = maxAnisotropy;
		samplerCreateInfo.mipmapMode = mipmapMode;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.minLod = 0.0f;
		samplerCreateInfo.maxLod = 0.0f;
		samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates = unnormalizedCoordinates;

		VKCALL(sampler.init(device, samplerCreateInfo));
	}

	void destroyImageView(VkDevice device);

	void updateImageLayout(ImageLayout newImageLayout);

	Transition* updateImageLayoutAndBarrier(ImageLayout newLayout);
public:
	VkImage getImage();
	  
	virtual VkImageView getImageView();

	VkImageView getReadableImageView();

	virtual VkSampler getSampler();

	VkFormat getFormat();

	uint32_t getSamples();

	VkImageAspectFlags getImageAspect();

	VkImageSubresourceRange getSubresourceRange();

	VkImageLayout getVkImageLayout() { return kImageMemoryBarrierData[imageLayout].layout; }

	ImageLayout getImageLayout() { return imageLayout; }

protected:
	handle::Image image;
	handle::ImageView view;
	handle::ImageView readView;
	handle::Sampler sampler;
	handle::DeviceMemory memory;	
	VkImageSubresourceRange subresourceRange;

	VkFormat format;
	uint32_t samples;
	VkExtent3D extent;
	ImageLayout imageLayout;
};
}