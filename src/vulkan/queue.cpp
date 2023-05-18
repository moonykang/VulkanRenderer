#include <vector>
#include "vulkan/queue.h"
#include "vulkan/commandBuffer.h"

namespace vk
{
bool Queue::init(VkDevice device, uint32_t queueFamilyIndex)
{
	queue.getDeviceQueue(device, queueFamilyIndex, 0);
	return true;
}

void Queue::destroy(VkDevice device)
{

}

bool Queue::submit(CommandBuffer* commandBuffer, std::vector<VkSemaphore>* waitSemaphores, std::vector<VkSemaphore>* signalSemaphores)
{
	const uint32_t commandBufferCount = 1;
	VkCommandBuffer commandBuffers[commandBufferCount] = { commandBuffer->getHandle() };

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = commandBuffers;

	if (waitSemaphores)
	{
		VkPipelineStageFlags waitDstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submitInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores->size());
		submitInfo.pWaitSemaphores = waitSemaphores->data();
		submitInfo.pWaitDstStageMask = &waitDstStage;
	}

	if (signalSemaphores)
	{
		submitInfo.signalSemaphoreCount = static_cast<uint32_t>(signalSemaphores->size());
		submitInfo.pSignalSemaphores = signalSemaphores->data();
	}

	VKCALL(queue.submit(submitInfo, commandBuffer->getFence()));

	return true;
}


VkResult Queue::present(VkSwapchainKHR swapchain, uint32_t imageIndex, std::vector<VkSemaphore>* waitSemaphores)
{
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;
	if (waitSemaphores)
	{
		presentInfo.waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores->size());
		presentInfo.pWaitSemaphores = waitSemaphores->data();
	}
	return queue.present(presentInfo);
}

void Queue::waitIdle()
{
	queue.waitIdle();
}
}