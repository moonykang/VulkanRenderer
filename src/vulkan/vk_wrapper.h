#pragma once

#include <algorithm>
#include "vk_headers.h"
#include "vulkan/extension.h"

namespace vk
{
namespace handle
{
template <typename DerivedT, typename HandleT>
class WrappedObject : NonCopyable
{
public:
    HandleT getHandle() const { return mHandle; }
    void setHandle(HandleT handle) { mHandle = handle; }
    bool valid() const { return mHandle != VK_NULL_HANDLE; }

    const HandleT* ptr() const { return &mHandle; }

    HandleT release()
    {
        HandleT handle = mHandle;
        mHandle = VK_NULL_HANDLE;
        return handle;
    }

protected:
    WrappedObject() : mHandle(VK_NULL_HANDLE) {}
    ~WrappedObject() { ASSERT(!valid()); }

    WrappedObject(WrappedObject&& other) : mHandle(other.mHandle)
    {
        other.mHandle = VK_NULL_HANDLE;
    }

    WrappedObject& operator=(WrappedObject &&other)
    {
        ASSERT(!valid());
        std::swap(mHandle, other.mHandle);
        return *this;
    }

    HandleT mHandle;
};

class Instance final : public WrappedObject<Instance, VkInstance>
{
public:
    Instance() = default;
    void destroy();

    VkResult init(const VkInstanceCreateInfo& createInfo);
};

class PhysicalDevice final : public WrappedObject<PhysicalDevice, VkPhysicalDevice>
{
public:
    void getProperties2(VkPhysicalDeviceProperties2* properties);
};

class Device final : public WrappedObject<Device, VkDevice>
{
public:
    Device() = default;
    void destroy();

    VkResult init(VkPhysicalDevice device, const VkDeviceCreateInfo& createInfo);
};

class Queue final : public WrappedObject<Queue, VkQueue>
{
public:
    Queue() = default;
    void destroy();

    void getDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex);
    VkResult submit(const VkSubmitInfo& submitInfo, VkFence fence);
    VkResult waitIdle();
    VkResult present(const VkPresentInfoKHR& presentInfo);

};

class RenderPass final : public WrappedObject<RenderPass, VkRenderPass>
{
public:
    RenderPass() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkRenderPassCreateInfo& createInfo);
};

class Framebuffer final : public WrappedObject<Framebuffer, VkFramebuffer>
{
public:
    Framebuffer() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkFramebufferCreateInfo& createInfo);
};

class CommandPool final : public WrappedObject<CommandPool, VkCommandPool>
{
public:
    CommandPool() = default;

    void destroy(VkDevice device);
    VkResult reset(VkDevice device, VkCommandPoolResetFlags flags);
    void freeCommandBuffers(VkDevice device, uint32_t commandBufferCount, const VkCommandBuffer* commandBuffers);
    VkResult init(VkDevice device, const VkCommandPoolCreateInfo& createInfo);
};

class PipelineCache final : public WrappedObject<PipelineCache, VkPipelineCache>
{
public:
    PipelineCache() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkPipelineCacheCreateInfo& createInfo);
    VkResult getCacheData(VkDevice device, size_t* cacheSize, void* cacheData);
    VkResult merge(VkDevice device, uint32_t srcCacheCount, const VkPipelineCache* srcCaches);
};

class Pipeline final : public WrappedObject<Pipeline, VkPipeline>
{
public:
    Pipeline() = default;
    void destroy(VkDevice device);

    VkResult initGraphics(VkDevice device,
                          const VkGraphicsPipelineCreateInfo& createInfo,
                          const VkPipelineCache& pipelineCache);
    VkResult initCompute(VkDevice device,
                         const VkComputePipelineCreateInfo& createInfo,
                         const VkPipelineCache& pipelineCache);
    VkResult initRayTracing(VkDevice device,
        const VkRayTracingPipelineCreateInfoKHR& createInfo,
        const VkPipelineCache& pipelineCache);
};

class ShaderModule final : public WrappedObject<ShaderModule, VkShaderModule>
{
public:
    ShaderModule() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkShaderModuleCreateInfo& createInfo);
};

class PipelineLayout final : public WrappedObject<PipelineLayout, VkPipelineLayout>
{
public:
    PipelineLayout() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkPipelineLayoutCreateInfo& createInfo);
};

class DescriptorSetLayout final : public WrappedObject<DescriptorSetLayout, VkDescriptorSetLayout>
{
public:
    DescriptorSetLayout() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkDescriptorSetLayoutCreateInfo& createInfo);
};

class DescriptorPool final : public WrappedObject<DescriptorPool, VkDescriptorPool>
{
public:
    DescriptorPool() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkDescriptorPoolCreateInfo& createInfo);
    VkResult allocateDescriptorSets(VkDevice device,
                                    const VkDescriptorSetAllocateInfo& allocateInfo,
                                    VkDescriptorSet* pDescriptorSetOut);
    VkResult freeDescriptorSets(VkDevice device,
                                uint32_t descriptorSetCount,
                                const VkDescriptorSet* pDescriptorSets);
};

class DeviceMemory final : public WrappedObject<DeviceMemory, VkDeviceMemory>
{
public:
    DeviceMemory() = default;
    void destroy(VkDevice device);

    VkResult allocate(VkDevice device, const VkMemoryAllocateInfo& allocateInfo);
    VkResult map(VkDevice device, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, uint8_t** data);
    void unmap(VkDevice device) const;
    void flush(VkDevice device, VkMappedMemoryRange& range);
    void invalidate(VkDevice device, VkMappedMemoryRange& range);
};

class Buffer final : public WrappedObject<Buffer, VkBuffer>
{
public:
    Buffer() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkBufferCreateInfo& createInfo);
    VkResult bindMemory(VkDevice device, const DeviceMemory& deviceMemory);
    void getMemoryRequirements(VkDevice device, VkMemoryRequirements* VkMemoryRequirements);
};

class BufferView final : public WrappedObject<BufferView, VkBufferView>
{
public:
    BufferView() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkBufferViewCreateInfo& createInfo);
};

class Image final : public WrappedObject<Image, VkImage>
{
public:
    Image() = default;
    void destroy(VkDevice device);

    void setHandle(VkImage image);
    void reset();
    VkResult init(VkDevice device, const VkImageCreateInfo& createInfo);
    
    void getMemoryRequirements(VkDevice device, VkMemoryRequirements* pRequirementsOut) const;
    VkResult bindMemory(VkDevice device, const DeviceMemory& deviceMemory);

    void getSubresourceLayout(VkDevice device,
                              VkImageAspectFlagBits aspectMask,
                              uint32_t mipLevel,
                              uint32_t arrayLayer,
                              VkSubresourceLayout* pSubresourceLayoutOut) const;
};

class ImageView final : public WrappedObject<ImageView, VkImageView>
{
public:
    ImageView() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device, const VkImageViewCreateInfo& createInfo);
};

class Sampler final : public WrappedObject<Sampler, VkSampler>
{
public:
    Sampler() = default;
    void destroy(VkDevice device);
    VkResult init(VkDevice device, const VkSamplerCreateInfo& createInfo);
};

class Fence final : public WrappedObject<Fence, VkFence>
{
public:
    Fence() = default;
    void destroy(VkDevice device);
    using WrappedObject::operator=;

    VkResult init(VkDevice device, const VkFenceCreateInfo& createInfo);
    VkResult reset(VkDevice device);
    VkResult getStatus(VkDevice device) const;
    VkResult wait(VkDevice device, uint64_t timeout) const;
};

class Semaphore final : public WrappedObject<Semaphore, VkSemaphore>
{
public:
    Semaphore() = default;
    void destroy(VkDevice device);

    VkResult init(VkDevice device);
    VkResult init(VkDevice device, const VkSemaphoreCreateInfo& createInfo);
};

class CommandBuffer final : public WrappedObject<CommandBuffer, VkCommandBuffer>
{
public:
    CommandBuffer() = default;

    void free(VkDevice device, VkCommandPool commandPool);
    VkResult allocate(VkDevice device, const VkCommandBufferAllocateInfo& allocateInfo);
    VkResult reset();
    VkResult begin(const VkCommandBufferBeginInfo& beginInfo);
    VkResult end();
/*
    void beginRenderPass(const VkRenderPassBeginInfo& beginInfo, VkSubpassContents subpassContents);
    void bindDescriptorSets(const PipelineLayout& layout,
        VkPipelineBindPoint pipelineBindPoint,
        uint32_t firstSet,
        uint32_t descriptorSetCount,
        const VkDescriptorSet* descriptorSets,
        uint32_t dynamicOffsetCount,
        const uint32_t* dynamicOffsets);
    void bindGraphicsPipeline(const Pipeline& pipeline);
    void bindComputePipeline(const Pipeline& pipeline);
    void bindPipeline(VkPipelineBindPoint pipelineBindPoint, const Pipeline& pipeline);
    void bindIndexBuffer(const Buffer& buffer, VkDeviceSize offset, VkIndexType indexType);
    void bindVertexBuffers(uint32_t firstBinding,
        uint32_t bindingCount,
        const VkBuffer* buffers,
        const VkDeviceSize* offsets);
    void clearColorImage(const Image& image,
        VkImageLayout imageLayout,
        const VkClearColorValue& color,
        uint32_t rangeCount,
        const VkImageSubresourceRange* ranges);
    void clearDepthStencilImage(const Image& image,
        VkImageLayout imageLayout,
        const VkClearDepthStencilValue& depthStencil,
        uint32_t rangeCount,
        const VkImageSubresourceRange* ranges);
    void clearAttachments(uint32_t attachmentCount,
        const VkClearAttachment* attachments,
        uint32_t rectCount,
        const VkClearRect* rects);
    void copyBuffer(const Buffer& srcBuffer,
        const Buffer& destBuffer,
        uint32_t regionCount,
        const VkBufferCopy* regions);
    void copyBufferToImage(VkBuffer srcBuffer,
        const Image& dstImage,
        VkImageLayout dstImageLayout,
        uint32_t regionCount,
        const VkBufferImageCopy* regions);
    void copyImageToBuffer(const Image& srcImage,
        VkImageLayout srcImageLayout,
        VkBuffer dstBuffer,
        uint32_t regionCount,
        const VkBufferImageCopy* regions);
    void copyImage(const Image& srcImage,
        VkImageLayout srcImageLayout,
        const Image& dstImage,
        VkImageLayout dstImageLayout,
        uint32_t regionCount,
        const VkImageCopy* regions);
    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
    void dispatchIndirect(const Buffer& buffer, VkDeviceSize offset);
    void draw(uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t firstVertex,
        uint32_t firstInstance);
    void draw(uint32_t vertexCount, uint32_t firstVertex);
    void drawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex);
    void drawInstancedBaseInstance(uint32_t vertexCount,
        uint32_t instanceCount,
        uint32_t firstVertex,
        uint32_t firstInstance);
    void drawIndexed(uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        int32_t vertexOffset,
        uint32_t firstInstance);
    void drawIndexed(uint32_t indexCount);
    void drawIndexedBaseVertex(uint32_t indexCount, uint32_t vertexOffset);
    void drawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount);
    void drawIndexedInstancedBaseVertex(uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t vertexOffset);
    void drawIndexedInstancedBaseVertexBaseInstance(uint32_t indexCount,
        uint32_t instanceCount,
        uint32_t firstIndex,
        int32_t vertexOffset,
        uint32_t firstInstance);
    void drawIndexedIndirect(const Buffer& buffer,
        VkDeviceSize offset,
        uint32_t drawCount,
        uint32_t stride);
    void drawIndirect(const Buffer& buffer,
        VkDeviceSize offset,
        uint32_t drawCount,
        uint32_t stride);
    void endRenderPass();
    void executeCommands(uint32_t commandBufferCount, const CommandBuffer* commandBuffers);
    void getMemoryUsageStats(size_t* usedMemoryOut, size_t* allocatedMemoryOut) const;
    void executionBarrier(VkPipelineStageFlags stageMask);
    void fillBuffer(const Buffer& dstBuffer,
        VkDeviceSize dstOffset,
        VkDeviceSize size,
        uint32_t data);
    void bufferBarrier(VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        const VkBufferMemoryBarrier* bufferMemoryBarrier);
    void imageBarrier(VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        const VkImageMemoryBarrier& imageMemoryBarrier);
    void memoryBarrier(VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        const VkMemoryBarrier* memoryBarrier);
    void nextSubpass(VkSubpassContents subpassContents);
    void pipelineBarrier(VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        uint32_t memoryBarrierCount,
        const VkMemoryBarrier* memoryBarriers,
        uint32_t bufferMemoryBarrierCount,
        const VkBufferMemoryBarrier* bufferMemoryBarriers,
        uint32_t imageMemoryBarrierCount,
        const VkImageMemoryBarrier* imageMemoryBarriers);
    void pushConstants(const PipelineLayout& layout,
        VkShaderStageFlags flag,
        uint32_t offset,
        uint32_t size,
        const void* data);
    void setEvent(VkEvent event, VkPipelineStageFlags stageMask);
    void setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D* scissors);
    VkResult reset();
    void resetEvent(VkEvent event, VkPipelineStageFlags stageMask);
    void resolveImage(const Image& srcImage,
        VkImageLayout srcImageLayout,
        const Image& dstImage,
        VkImageLayout dstImageLayout,
        uint32_t regionCount,
        const VkImageResolve* regions);
    void waitEvents(uint32_t eventCount,
        const VkEvent* events,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        uint32_t memoryBarrierCount,
        const VkMemoryBarrier* memoryBarriers,
        uint32_t bufferMemoryBarrierCount,
        const VkBufferMemoryBarrier* bufferMemoryBarriers,
        uint32_t imageMemoryBarrierCount,
        const VkImageMemoryBarrier* imageMemoryBarriers);
*/
};

inline void Instance::destroy()
{
    if (valid())
    {
        vkDestroyInstance(mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult Instance::init(const VkInstanceCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateInstance(&createInfo, nullptr, &mHandle);
}

inline void PhysicalDevice::getProperties2(VkPhysicalDeviceProperties2* properties)
{
    ASSERT(!valid());

    vkGetPhysicalDeviceProperties2KHR(mHandle, properties);
}

inline void Device::destroy()
{
    if (valid())
    {
        vkDestroyDevice(mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult Device::init(VkPhysicalDevice device, const VkDeviceCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateDevice(device, &createInfo, nullptr, &mHandle);
}

inline void RenderPass::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyRenderPass(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult RenderPass::init(VkDevice device, const VkRenderPassCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateRenderPass(device, &createInfo, nullptr, &mHandle);
}

inline void Queue::getDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex)
{
    ASSERT(!valid());
    vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, &mHandle);
}

inline VkResult Queue::submit(const VkSubmitInfo& submitInfo, VkFence fence)
{
    ASSERT(!valid());
    return vkQueueSubmit(mHandle, 1, &submitInfo, fence);
}

inline VkResult Queue::waitIdle()
{
    ASSERT(!valid());
    return vkQueueWaitIdle(mHandle);
}

inline VkResult Queue::present(const VkPresentInfoKHR& presentInfo)
{
    ASSERT(!valid());
    return vkQueuePresentKHR(mHandle, &presentInfo);
}

inline void Framebuffer::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyFramebuffer(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult Framebuffer::init(VkDevice device, const VkFramebufferCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateFramebuffer(device, &createInfo, nullptr, &mHandle);
}

inline void CommandPool::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyCommandPool(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult CommandPool::reset(VkDevice device, VkCommandPoolResetFlags flags)
{
    ASSERT(valid());
    return vkResetCommandPool(device, mHandle, flags);
}

inline void CommandPool::freeCommandBuffers(VkDevice device,
                                                  uint32_t commandBufferCount,
                                                  const VkCommandBuffer *commandBuffers)
{
    ASSERT(valid());
    vkFreeCommandBuffers(device, mHandle, commandBufferCount, commandBuffers);
}

inline VkResult CommandPool::init(VkDevice device, const VkCommandPoolCreateInfo &createInfo)
{
    ASSERT(!valid());
    return vkCreateCommandPool(device, &createInfo, nullptr, &mHandle);
}

inline void PipelineCache::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyPipelineCache(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult PipelineCache::init(VkDevice device, const VkPipelineCacheCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreatePipelineCache(device, &createInfo, nullptr, &mHandle);
}

inline VkResult PipelineCache::getCacheData(VkDevice device, size_t* pCacheSize, void* cacheData)
{
    ASSERT(!valid());
    return vkGetPipelineCacheData(device, mHandle, pCacheSize, cacheData);
}

inline VkResult PipelineCache::merge(VkDevice device, uint32_t srcCacheCount, const VkPipelineCache* pSrcCaches)
{
    ASSERT(!valid());
    return vkMergePipelineCaches(device, mHandle, srcCacheCount, pSrcCaches);
}

inline void Pipeline::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyPipeline(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult Pipeline::initGraphics(VkDevice device,
                                const VkGraphicsPipelineCreateInfo& createInfo,
                                const VkPipelineCache& pipelineCache)
{
    ASSERT(!valid());
    return vkCreateGraphicsPipelines(device, pipelineCache, 1, &createInfo, nullptr, &mHandle);
}

inline VkResult Pipeline::initCompute(VkDevice device,
                               const VkComputePipelineCreateInfo& createInfo,
                               const VkPipelineCache& pipelineCache)
{
    ASSERT(!valid());
    return vkCreateComputePipelines(device, pipelineCache, 1, &createInfo, nullptr, &mHandle);
}

inline VkResult Pipeline::initRayTracing(VkDevice device,
                               const VkRayTracingPipelineCreateInfoKHR& createInfo,
                               const VkPipelineCache& pipelineCache)
{
    ASSERT(!valid());
    return vkCreateRayTracingPipelinesKHR(device, VK_NULL_HANDLE, pipelineCache, 1, &createInfo, nullptr, &mHandle);
}

inline void ShaderModule::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyShaderModule(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult ShaderModule::init(VkDevice device, const VkShaderModuleCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateShaderModule(device, &createInfo, nullptr, &mHandle);
}

inline void PipelineLayout::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyPipelineLayout(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult PipelineLayout::init(VkDevice device, const VkPipelineLayoutCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreatePipelineLayout(device, &createInfo, nullptr, &mHandle);
}

inline void DescriptorSetLayout::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyDescriptorSetLayout(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult DescriptorSetLayout::init(VkDevice device, const VkDescriptorSetLayoutCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &mHandle);
}

inline void DescriptorPool::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyDescriptorPool(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult DescriptorPool::init(VkDevice device, const VkDescriptorPoolCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateDescriptorPool(device, &createInfo, nullptr, &mHandle);
}

inline VkResult DescriptorPool::allocateDescriptorSets(VkDevice device,
                                                       const VkDescriptorSetAllocateInfo& allocateInfo,
                                                       VkDescriptorSet* pDescriptorSetOut)
{
    ASSERT(valid());
    return vkAllocateDescriptorSets(device, &allocateInfo, pDescriptorSetOut);
}

inline VkResult DescriptorPool::freeDescriptorSets(VkDevice device,
                                                   uint32_t descriptorSetCount,
                                                   const VkDescriptorSet* pDescriptorSets)
{
    ASSERT(valid());
    return vkFreeDescriptorSets(device, mHandle, descriptorSetCount, pDescriptorSets);
}

inline void DeviceMemory::destroy(VkDevice device)
{
    if (valid())
    {
        vkFreeMemory(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult DeviceMemory::allocate(VkDevice device, const VkMemoryAllocateInfo& allocateInfo)
{
    ASSERT(!valid());
    return vkAllocateMemory(device, &allocateInfo, nullptr, &mHandle);
}

inline VkResult DeviceMemory::map(VkDevice device,
                                  VkDeviceSize offset,
                                  VkDeviceSize size,
                                  VkMemoryMapFlags flags,
                                  uint8_t** ppData)
{
    ASSERT(valid());
    return vkMapMemory(device, mHandle, offset, size, flags, reinterpret_cast<void**>(ppData));
}

inline void DeviceMemory::unmap(VkDevice device) const
{
    ASSERT(valid());
    vkUnmapMemory(device, mHandle);
}

inline void DeviceMemory::flush(VkDevice device, VkMappedMemoryRange& range)
{
    vkFlushMappedMemoryRanges(device, 1, &range);
}

inline void DeviceMemory::invalidate(VkDevice device, VkMappedMemoryRange& range)
{
    vkInvalidateMappedMemoryRanges(device, 1, &range);
}

inline void Buffer::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyBuffer(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult Buffer::init(VkDevice device, const VkBufferCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateBuffer(device, &createInfo, nullptr, &mHandle);
}

inline VkResult Buffer::bindMemory(VkDevice device, const DeviceMemory& deviceMemory)
{
    ASSERT(valid() && deviceMemory.valid());
    return vkBindBufferMemory(device, mHandle, deviceMemory.getHandle(), 0);
}

inline void Buffer::getMemoryRequirements(VkDevice device, VkMemoryRequirements* pMemoryRequirementsOut)
{
    ASSERT(valid());
    vkGetBufferMemoryRequirements(device, mHandle, pMemoryRequirementsOut);
}

inline void BufferView::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyBufferView(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult BufferView::init(VkDevice device, const VkBufferViewCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateBufferView(device, &createInfo, nullptr, &mHandle);
}

inline void Image::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyImage(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline void Image::setHandle(VkImage image)
{
    mHandle = image;
}

inline void Image::reset()
{
    ASSERT(valid());
    mHandle = VK_NULL_HANDLE;
}

inline VkResult Image::init(VkDevice device, const VkImageCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateImage(device, &createInfo, nullptr, &mHandle);
}

inline void Image::getMemoryRequirements(VkDevice device, VkMemoryRequirements* pRequirementsOut) const
{
    ASSERT(valid());
    vkGetImageMemoryRequirements(device, mHandle, pRequirementsOut);
}

inline VkResult Image::bindMemory(VkDevice device, const DeviceMemory& deviceMemory)
{
    ASSERT(valid() && deviceMemory.valid());
    return vkBindImageMemory(device, mHandle, deviceMemory.getHandle(), 0);
}

inline void Image::getSubresourceLayout(VkDevice device,
                                        VkImageAspectFlagBits aspectMask,
                                        uint32_t mipLevel,
                                        uint32_t arrayLayer,
                                        VkSubresourceLayout* pSubresourceLayoutOut) const
{
    VkImageSubresource subresource = {};
    subresource.aspectMask = aspectMask;
    subresource.mipLevel = mipLevel;
    subresource.arrayLayer = arrayLayer;

    vkGetImageSubresourceLayout(device, mHandle, &subresource, pSubresourceLayoutOut);
}

inline void ImageView::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyImageView(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult ImageView::init(VkDevice device, const VkImageViewCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateImageView(device, &createInfo, nullptr, &mHandle);
}

// Sampler implementation.
inline void Sampler::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroySampler(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult Sampler::init(VkDevice device, const VkSamplerCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateSampler(device, &createInfo, nullptr, &mHandle);
}

inline void Fence::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroyFence(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult Fence::init(VkDevice device, const VkFenceCreateInfo& createInfo)
{
    ASSERT(!valid());
    return vkCreateFence(device, &createInfo, nullptr, &mHandle);
}

inline VkResult Fence::reset(VkDevice device)
{
    ASSERT(valid());
    return vkResetFences(device, 1, &mHandle);
}

inline VkResult Fence::getStatus(VkDevice device) const
{
    ASSERT(valid());
    return vkGetFenceStatus(device, mHandle);
}

inline VkResult Fence::wait(VkDevice device, uint64_t timeout) const
{
    ASSERT(valid());
    return vkWaitForFences(device, 1, &mHandle, true, timeout);

}

inline void Semaphore::destroy(VkDevice device)
{
    if (valid())
    {
        vkDestroySemaphore(device, mHandle, nullptr);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult Semaphore::init(VkDevice device)
{
    ASSERT(!valid());

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.flags = 0;

    return vkCreateSemaphore(device, &semaphoreInfo, nullptr, &mHandle);
}

inline VkResult Semaphore::init(VkDevice device, const VkSemaphoreCreateInfo& createInfo)
{
    ASSERT(valid());
    return vkCreateSemaphore(device, &createInfo, nullptr, &mHandle);
}

// Command buffer
inline void CommandBuffer::free(VkDevice device, VkCommandPool commandPool)
{
    if (valid())
    {
        vkFreeCommandBuffers(device, commandPool, 1, &mHandle);
        mHandle = VK_NULL_HANDLE;
    }
}

inline VkResult CommandBuffer::allocate(VkDevice device, const VkCommandBufferAllocateInfo& allocateInfo)
{
    ASSERT(!valid());
    return vkAllocateCommandBuffers(device, &allocateInfo, &mHandle);
}

inline VkResult CommandBuffer::reset()
{
    ASSERT(valid());
    return vkResetCommandBuffer(mHandle, 0);
}

inline VkResult CommandBuffer::begin(const VkCommandBufferBeginInfo& info)
{
    ASSERT(valid());
    return vkBeginCommandBuffer(mHandle, &info);
}

inline VkResult CommandBuffer::end()
{
    ASSERT(valid());
    return vkEndCommandBuffer(mHandle);
}

/*
inline VkResult CommandBuffer::reset()
{
    ASSERT(valid());
    return vkResetCommandBuffer(mHandle, 0);
}

inline void CommandBuffer::memoryBarrier(VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    const VkMemoryBarrier* memoryBarrier)
{
    ASSERT(valid());
    vkCmdPipelineBarrier(mHandle, srcStageMask, dstStageMask, 0, 1, memoryBarrier, 0, nullptr, 0,
        nullptr);
}

inline void CommandBuffer::nextSubpass(VkSubpassContents subpassContents)
{
    ASSERT(valid());
    vkCmdNextSubpass(mHandle, subpassContents);
}

inline void CommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkDependencyFlags dependencyFlags,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* memoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* bufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* imageMemoryBarriers)
{
    ASSERT(valid());
    vkCmdPipelineBarrier(mHandle, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
        memoryBarriers, bufferMemoryBarrierCount, bufferMemoryBarriers,
        imageMemoryBarrierCount, imageMemoryBarriers);
}

inline void CommandBuffer::executionBarrier(VkPipelineStageFlags stageMask)
{
    ASSERT(valid());
    vkCmdPipelineBarrier(mHandle, stageMask, stageMask, 0, 0, nullptr, 0, nullptr, 0, nullptr);
}

inline void CommandBuffer::bufferBarrier(VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    const VkBufferMemoryBarrier* bufferMemoryBarrier)
{
    ASSERT(valid());
    vkCmdPipelineBarrier(mHandle, srcStageMask, dstStageMask, 0, 0, nullptr, 1, bufferMemoryBarrier,
        0, nullptr);
}

inline void CommandBuffer::imageBarrier(VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    const VkImageMemoryBarrier& imageMemoryBarrier)
{
    ASSERT(valid());
    vkCmdPipelineBarrier(mHandle, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1,
        &imageMemoryBarrier);
}

inline void CommandBuffer::copyBuffer(const Buffer& srcBuffer,
    const Buffer& destBuffer,
    uint32_t regionCount,
    const VkBufferCopy* regions)
{
    ASSERT(valid() && srcBuffer.valid() && destBuffer.valid());
    vkCmdCopyBuffer(mHandle, srcBuffer.getHandle(), destBuffer.getHandle(), regionCount, regions);
}

inline void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer,
    const Image& dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkBufferImageCopy* regions)
{
    ASSERT(valid() && dstImage.valid());
    ASSERT(srcBuffer != VK_NULL_HANDLE);
    ASSERT(regionCount == 1);
    vkCmdCopyBufferToImage(mHandle, srcBuffer, dstImage.getHandle(), dstImageLayout, 1, regions);
}

inline void CommandBuffer::copyImageToBuffer(const Image& srcImage,
    VkImageLayout srcImageLayout,
    VkBuffer dstBuffer,
    uint32_t regionCount,
    const VkBufferImageCopy* regions)
{
    ASSERT(valid() && srcImage.valid());
    ASSERT(dstBuffer != VK_NULL_HANDLE);
    ASSERT(regionCount == 1);
    vkCmdCopyImageToBuffer(mHandle, srcImage.getHandle(), srcImageLayout, dstBuffer, 1, regions);
}

inline void CommandBuffer::clearColorImage(const Image& image,
    VkImageLayout imageLayout,
    const VkClearColorValue& color,
    uint32_t rangeCount,
    const VkImageSubresourceRange* ranges)
{
    ASSERT(valid());
    ASSERT(rangeCount == 1);
    vkCmdClearColorImage(mHandle, image.getHandle(), imageLayout, &color, 1, ranges);
}

inline void CommandBuffer::clearDepthStencilImage(
    const Image& image,
    VkImageLayout imageLayout,
    const VkClearDepthStencilValue& depthStencil,
    uint32_t rangeCount,
    const VkImageSubresourceRange* ranges)
{
    ASSERT(valid());
    ASSERT(rangeCount == 1);
    vkCmdClearDepthStencilImage(mHandle, image.getHandle(), imageLayout, &depthStencil, 1, ranges);
}

inline void CommandBuffer::clearAttachments(uint32_t attachmentCount,
    const VkClearAttachment* attachments,
    uint32_t rectCount,
    const VkClearRect* rects)
{
    ASSERT(valid());
    vkCmdClearAttachments(mHandle, attachmentCount, attachments, rectCount, rects);
}

inline void CommandBuffer::copyImage(const Image& srcImage,
    VkImageLayout srcImageLayout,
    const Image& dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkImageCopy* regions)
{
    ASSERT(valid() && srcImage.valid() && dstImage.valid());
#ifdef SVDT_ENABLE_VMA_DEFRAGMENTATION
    vkCmdCopyImage(mHandle, srcImage.getHandle(), srcImageLayout, dstImage.getHandle(),
        dstImageLayout, regionCount, regions);
#else
    ASSERT(regionCount == 1);
    vkCmdCopyImage(mHandle, srcImage.getHandle(), srcImageLayout, dstImage.getHandle(),
        dstImageLayout, 1, regions);
#endif
}

inline void CommandBuffer::bindIndexBuffer(const Buffer& buffer,
    VkDeviceSize offset,
    VkIndexType indexType)
{
    ASSERT(valid());
    vkCmdBindIndexBuffer(mHandle, buffer.getHandle(), offset, indexType);
}

inline void CommandBuffer::bindDescriptorSets(const PipelineLayout& layout,
    VkPipelineBindPoint pipelineBindPoint,
    uint32_t firstSet,
    uint32_t descriptorSetCount,
    const VkDescriptorSet* descriptorSets,
    uint32_t dynamicOffsetCount,
    const uint32_t* dynamicOffsets)
{
    ASSERT(valid() && layout.valid());
    vkCmdBindDescriptorSets(mHandle, pipelineBindPoint, layout.getHandle(), firstSet,
        descriptorSetCount, descriptorSets, dynamicOffsetCount, dynamicOffsets);
}

inline void CommandBuffer::executeCommands(uint32_t commandBufferCount,
    const CommandBuffer* commandBuffers)
{
    ASSERT(valid());
    vkCmdExecuteCommands(mHandle, commandBufferCount, commandBuffers[0].ptr());
}

inline void CommandBuffer::getMemoryUsageStats(size_t* usedMemoryOut,
    size_t* allocatedMemoryOut) const
{
    // No data available.
    *usedMemoryOut = 0;
    *allocatedMemoryOut = 1;
}

inline void CommandBuffer::fillBuffer(const Buffer& dstBuffer,
    VkDeviceSize dstOffset,
    VkDeviceSize size,
    uint32_t data)
{
    ASSERT(valid());
    vkCmdFillBuffer(mHandle, dstBuffer.getHandle(), dstOffset, size, data);
}

inline void CommandBuffer::pushConstants(const PipelineLayout& layout,
    VkShaderStageFlags flag,
    uint32_t offset,
    uint32_t size,
    const void* data)
{
    ASSERT(valid() && layout.valid());
    ASSERT(offset == 0);
    vkCmdPushConstants(mHandle, layout.getHandle(), flag, 0, size, data);
}

inline void CommandBuffer::setEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
    ASSERT(valid() && event != VK_NULL_HANDLE);
    vkCmdSetEvent(mHandle, event, stageMask);
}

inline void CommandBuffer::setScissor(uint32_t firstScissor,
    uint32_t scissorCount,
    const VkRect2D* scissors)
{
    ASSERT(valid() && scissors != nullptr);
    vkCmdSetScissor(mHandle, firstScissor, scissorCount, scissors);
}

inline void CommandBuffer::resetEvent(VkEvent event, VkPipelineStageFlags stageMask)
{
    ASSERT(valid() && event != VK_NULL_HANDLE);
    vkCmdResetEvent(mHandle, event, stageMask);
}

inline void CommandBuffer::waitEvents(uint32_t eventCount,
    const VkEvent* events,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    uint32_t memoryBarrierCount,
    const VkMemoryBarrier* memoryBarriers,
    uint32_t bufferMemoryBarrierCount,
    const VkBufferMemoryBarrier* bufferMemoryBarriers,
    uint32_t imageMemoryBarrierCount,
    const VkImageMemoryBarrier* imageMemoryBarriers)
{
    ASSERT(valid());
    vkCmdWaitEvents(mHandle, eventCount, events, srcStageMask, dstStageMask, memoryBarrierCount,
        memoryBarriers, bufferMemoryBarrierCount, bufferMemoryBarriers,
        imageMemoryBarrierCount, imageMemoryBarriers);
}

inline void CommandBuffer::resolveImage(const Image& srcImage,
    VkImageLayout srcImageLayout,
    const Image& dstImage,
    VkImageLayout dstImageLayout,
    uint32_t regionCount,
    const VkImageResolve* regions)
{
    ASSERT(valid() && srcImage.valid() && dstImage.valid());
    vkCmdResolveImage(mHandle, srcImage.getHandle(), srcImageLayout, dstImage.getHandle(),
        dstImageLayout, regionCount, regions);
}

inline void CommandBuffer::draw(uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance)
{
    ASSERT(valid());
    vkCmdDraw(mHandle, vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void CommandBuffer::draw(uint32_t vertexCount, uint32_t firstVertex)
{
    ASSERT(valid());
    vkCmdDraw(mHandle, vertexCount, 1, firstVertex, 0);
}

inline void CommandBuffer::drawInstanced(uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex)
{
    ASSERT(valid());
    vkCmdDraw(mHandle, vertexCount, instanceCount, firstVertex, 0);
}

inline void CommandBuffer::drawInstancedBaseInstance(uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance)
{
    ASSERT(valid());
    vkCmdDraw(mHandle, vertexCount, instanceCount, firstVertex, firstInstance);
}

inline void CommandBuffer::drawIndexed(uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance)
{
    ASSERT(valid());
    vkCmdDrawIndexed(mHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

inline void CommandBuffer::drawIndexed(uint32_t indexCount)
{
    ASSERT(valid());
    vkCmdDrawIndexed(mHandle, indexCount, 1, 0, 0, 0);
}

inline void CommandBuffer::drawIndexedBaseVertex(uint32_t indexCount, uint32_t vertexOffset)
{
    ASSERT(valid());
    vkCmdDrawIndexed(mHandle, indexCount, 1, 0, vertexOffset, 0);
}

inline void CommandBuffer::drawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount)
{
    ASSERT(valid());
    vkCmdDrawIndexed(mHandle, indexCount, instanceCount, 0, 0, 0);
}

inline void CommandBuffer::drawIndexedInstancedBaseVertex(uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t vertexOffset)
{
    ASSERT(valid());
    vkCmdDrawIndexed(mHandle, indexCount, instanceCount, 0, vertexOffset, 0);
}

inline void CommandBuffer::drawIndexedInstancedBaseVertexBaseInstance(uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t vertexOffset,
    uint32_t firstInstance)
{
    ASSERT(valid());
    vkCmdDrawIndexed(mHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

inline void CommandBuffer::drawIndexedIndirect(const Buffer& buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride)
{
    ASSERT(valid());
    vkCmdDrawIndexedIndirect(mHandle, buffer.getHandle(), offset, drawCount, stride);
}

inline void CommandBuffer::drawIndirect(const Buffer& buffer,
    VkDeviceSize offset,
    uint32_t drawCount,
    uint32_t stride)
{
    ASSERT(valid());
    vkCmdDrawIndirect(mHandle, buffer.getHandle(), offset, drawCount, stride);
}

inline void CommandBuffer::dispatch(uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ)
{
    ASSERT(valid());
    vkCmdDispatch(mHandle, groupCountX, groupCountY, groupCountZ);
}

inline void CommandBuffer::dispatchIndirect(const Buffer& buffer, VkDeviceSize offset)
{
    ASSERT(valid());
    vkCmdDispatchIndirect(mHandle, buffer.getHandle(), offset);
}

inline void CommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint,
    const Pipeline& pipeline)
{
    ASSERT(valid() && pipeline.valid());
    vkCmdBindPipeline(mHandle, pipelineBindPoint, pipeline.getHandle());
}

inline void CommandBuffer::bindGraphicsPipeline(const Pipeline& pipeline)
{
    ASSERT(valid() && pipeline.valid());
    vkCmdBindPipeline(mHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
}

inline void CommandBuffer::bindComputePipeline(const Pipeline& pipeline)
{
    ASSERT(valid() && pipeline.valid());
    vkCmdBindPipeline(mHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.getHandle());
}

inline void CommandBuffer::bindVertexBuffers(uint32_t firstBinding,
    uint32_t bindingCount,
    const VkBuffer* buffers,
    const VkDeviceSize* offsets)
{
    ASSERT(valid());
    vkCmdBindVertexBuffers(mHandle, firstBinding, bindingCount, buffers, offsets);
}
*/
// Command buffer
}
} // namespace vulkan