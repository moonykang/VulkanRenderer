#pragma once

#include <vector>
#include "vulkan/vk_wrapper.h"

namespace vk
{
class Image;

class Framebuffer
{
public:
	void init(VkDevice device, VkRenderPass renderpass, std::vector<VkImageView>& imageViews, VkExtent2D extent);

	void destroy(VkDevice device);

	VkFramebuffer getFramebuffer();
private:
	handle::Framebuffer framebuffer;
};
}