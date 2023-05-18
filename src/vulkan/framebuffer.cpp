#include "vulkan/framebuffer.h"
#include "vulkan/image.h"

namespace vk
{
void Framebuffer::init(VkDevice device, VkRenderPass renderpass, std::vector<VkImageView>& imageViews, VkExtent2D extent)
{
	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = renderpass;
	frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
	frameBufferCreateInfo.pAttachments = imageViews.data();
	frameBufferCreateInfo.width = extent.width;
	frameBufferCreateInfo.height = extent.height;
	frameBufferCreateInfo.layers = 1;

	framebuffer.init(device, frameBufferCreateInfo);
}

void Framebuffer::destroy(VkDevice device)
{
	framebuffer.destroy(device);
}

VkFramebuffer Framebuffer::getFramebuffer() { return framebuffer.getHandle(); }
}