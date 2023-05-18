#pragma once

#include "vulkan/vk_wrapper.h"

namespace vk
{
class CommandBuffer;

class Queue
{
public:
	bool init(VkDevice device, uint32_t queueFamilyIndex);

	void destroy(VkDevice device);

	bool submit(CommandBuffer* commandBuffer, std::vector<VkSemaphore>* waitSemaphores = nullptr, std::vector<VkSemaphore>* signalSemaphores = nullptr);


	VkResult present(VkSwapchainKHR swapchain, uint32_t imageIndex, std::vector<VkSemaphore>* waitSemaphores = nullptr);

	void waitIdle();

private:
	handle::Queue queue;
	
};
}