#include "vulkan/commandBufferManager.h"

namespace vk
{
CommandBufferManager::CommandBufferManager()
    : uploadCommandBuffer(nullptr)
    , activeCommandBuffer(nullptr)
{
}

bool CommandBufferManager::init(VkDevice device, uint32_t queueFamilyIndex)
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VKCALL(commandPool.init(device, commandPoolCreateInfo));

    allocateCommandBuffers(device);
    return true;
}

void CommandBufferManager::destory(VkDevice device)
{
    // TODO: Add checks if command buffer is finished
    while (!submitCommandBuffers.empty())
    {
        auto& commandBuffer = submitCommandBuffers.front();
        submitCommandBuffers.pop();

        commandBuffer->destroy(device, commandPool.getHandle());
        delete commandBuffer;
    }

    while (!readyCommandBuffers.empty())
    {
        auto& commandBuffer = readyCommandBuffers.front();
        readyCommandBuffers.pop();

        commandBuffer->destroy(device, commandPool.getHandle());
        delete commandBuffer;
    }

    commandPool.destroy(device);
}

void CommandBufferManager::allocateCommandBuffers(VkDevice device)
{
    for (uint32_t i = 0; i < DEFAULT_NUM_COMMAND_BUFFER; i++)
    {
        CommandBuffer* commandBuffer = new CommandBuffer();
        commandBuffer->init(device, commandPool.getHandle());
        readyCommandBuffers.push(commandBuffer);
    }
}

void CommandBufferManager::prepareActiveCommandBuffer(VkDevice device)
{
    if (activeCommandBuffer == nullptr)
    {
        if (readyCommandBuffers.empty())
        {
            allocateCommandBuffers(device);
        }

        activeCommandBuffer = readyCommandBuffers.front();
        readyCommandBuffers.pop();
    }
}

void CommandBufferManager::prepareUploadCommandBuffer(VkDevice device)
{
    if (uploadCommandBuffer == nullptr)
    {
        if (readyCommandBuffers.empty())
        {
            allocateCommandBuffers(device);
        }

        uploadCommandBuffer = readyCommandBuffers.front();
        readyCommandBuffers.pop();
    }
}

CommandBuffer* CommandBufferManager::getActiveCommandBuffer(VkDevice device)
{
    ASSERT(commandPool.valid());
    if (activeCommandBuffer == nullptr)
    {
        prepareActiveCommandBuffer(device);
        activeCommandBuffer->begin();
    }
    return activeCommandBuffer;
}

CommandBuffer* CommandBufferManager::getUploadCommandBuffer(VkDevice device)
{
    ASSERT(commandPool.valid());
    if (uploadCommandBuffer == nullptr)
    {
        prepareUploadCommandBuffer(device);
        uploadCommandBuffer->begin();
    }
    return uploadCommandBuffer;
}

void CommandBufferManager::submitActiveCommandBuffer(VkDevice device, Queue* queue, std::vector<VkSemaphore>* waitSemaphores, std::vector<VkSemaphore>* signalSemaphores)
{
    ASSERT(activeCommandBuffer);
    if (uploadCommandBuffer)
    {
        submitUploadCommandBuffer(device, queue);
    }
    activeCommandBuffer->end();
    queue->submit(activeCommandBuffer, waitSemaphores, signalSemaphores);
    resetCommandBuffers(device);
    submitCommandBuffers.push(activeCommandBuffer);
    activeCommandBuffer = nullptr;
}

void CommandBufferManager::submitUploadCommandBuffer(VkDevice device, Queue* queue)
{
    ASSERT(uploadCommandBuffer);
    uploadCommandBuffer->end();
    queue->submit(uploadCommandBuffer);
    resetCommandBuffers(device);
    submitCommandBuffers.push(uploadCommandBuffer);
    uploadCommandBuffer = nullptr;
}

void CommandBufferManager::resetCommandBuffers(VkDevice device, bool bIdle)
{
    while (!submitCommandBuffers.empty())
    {
        auto commandBuffer = submitCommandBuffers.front();

        const bool forceWait = submitCommandBuffers.size() > kMaxInFlightCommandBuffers || bIdle;

        if (commandBuffer->reset(device, forceWait))
        {
            submitCommandBuffers.pop();
            readyCommandBuffers.push(commandBuffer);
        }
        else
        {
            break;
        }
    }
}
}