#include "vulkan/commandBuffer.h"
#include "vulkan/transition.h"

namespace vk
{
CommandBuffer::CommandBuffer()
    : transition(nullptr)
{

}

VkResult CommandBuffer::init(VkDevice device, VkCommandPool commandPool)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;

    VKCALL(fence.init(device, fenceCreateInfo));

    return commandBuffer.allocate(device, commandBufferAllocateInfo);
}

void CommandBuffer::destroy(VkDevice device, VkCommandPool commandPool)
{
    fence.destroy(device);
    commandBuffer.free(device, commandPool);
}

bool CommandBuffer::reset(VkDevice device, const bool bWait)
{
    if (fence.getStatus(device) == VK_SUCCESS)
    {
        commandBuffer.reset();
        fence.reset(device);
        delete transition;
        transition = nullptr;
        return true;
    }
    else if (bWait)
    {
        LOGD("Wait fence");
        VKCALL(fence.wait(device, UINT64_MAX));
    }
    return false;
}

VkCommandBuffer CommandBuffer::getHandle() { return commandBuffer.getHandle(); }
VkFence CommandBuffer::getFence() { return fence.getHandle(); }

void CommandBuffer::addTransition(Transition* newTransition)
{
    if (newTransition == nullptr)
    {
        return;
    }

    if (transition == nullptr)
    {
        transition = newTransition;
    }
    else
    {
        transition->merge(newTransition);
    }
}

void CommandBuffer::flushTransitions()
{
    if (transition == nullptr)
    {
        return;
    }

    if (transition->build())
    {
        auto memoryBarriers = transition->getMemoryBarriers();
        auto imageMemoryBarriers = transition->getImageMemoryBarriers();
        auto bufferMemoryBarriers = transition->getBufferMemoryBarriers();

        pipelineBarrier(transition->getSrcStageMask(),
            transition->getDstStageMask(),
            0,
            static_cast<uint32_t>(memoryBarriers->size()),
            memoryBarriers->data(),
            static_cast<uint32_t>(bufferMemoryBarriers->size()),
            bufferMemoryBarriers->data(),
            static_cast<uint32_t>(imageMemoryBarriers->size()),
            imageMemoryBarriers->data());
    }

    transition->reset();
}

VkResult CommandBuffer::begin()
{
    VkCommandBufferBeginInfo commandBufferBeginInfo = {};
    commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    commandBufferBeginInfo.pNext = nullptr;
    commandBufferBeginInfo.flags = 0;
    commandBufferBeginInfo.pInheritanceInfo = nullptr;

    return commandBuffer.begin(commandBufferBeginInfo);
}
VkResult CommandBuffer::end()
{
    flushTransitions();
    return commandBuffer.end();
}
}