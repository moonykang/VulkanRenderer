#pragma once

#include <queue>
#include "vulkan/commandBuffer.h"
#include "vulkan/queue.h"
#include "vulkan/vk_wrapper.h"

namespace vk
{
const uint32_t kMaxInFlightCommandBuffers = 5;

class CommandBufferManager
{
public:
    CommandBufferManager();

    bool init(VkDevice device, uint32_t queueFamilyIndex);

    void destory(VkDevice device);

    void allocateCommandBuffers(VkDevice device);

    void prepareActiveCommandBuffer(VkDevice device);

    void prepareUploadCommandBuffer(VkDevice device);

    CommandBuffer* getActiveCommandBuffer(VkDevice device);

    CommandBuffer* getUploadCommandBuffer(VkDevice device);

    void submitActiveCommandBuffer(VkDevice device, Queue* queue, std::vector<VkSemaphore>* waitSemaphores = nullptr, std::vector<VkSemaphore>* signalSemaphores = nullptr);

    void submitUploadCommandBuffer(VkDevice device, Queue* queue);

    void resetCommandBuffers(VkDevice device, bool bIdle = false);

private:
    const uint32_t DEFAULT_NUM_COMMAND_BUFFER = 10;

    handle::CommandPool commandPool;
    CommandBuffer* uploadCommandBuffer;
    CommandBuffer* activeCommandBuffer;
    std::queue<CommandBuffer*> submitCommandBuffers;
    std::queue<CommandBuffer*> readyCommandBuffers;
};
}