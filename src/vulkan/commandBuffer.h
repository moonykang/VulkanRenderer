#pragma once

#include "vulkan/vk_wrapper.h"
#include "vulkan/extension.h"

namespace vk
{
class Transition;
class CommandBuffer
{
public:
    CommandBuffer();

    VkResult init(VkDevice device, VkCommandPool commandPool);

    void destroy(VkDevice device, VkCommandPool commandPool);

    bool reset(VkDevice device, const bool bWait);

    VkCommandBuffer getHandle();
    VkFence getFence();

    void addTransition(Transition* newTransition);
    
    void flushTransitions();
    VkResult begin();
    VkResult end();
public:
    inline void beginRenderPass(const VkRenderPassBeginInfo& beginInfo, VkSubpassContents subpassContents)
    {
        ASSERT(commandBuffer.valid());
        flushTransitions();
        vkCmdBeginRenderPass(commandBuffer.getHandle(), &beginInfo, subpassContents);
    }

    inline void endRenderPass()
    {
        ASSERT(commandBuffer.valid());
        vkCmdEndRenderPass(commandBuffer.getHandle());
    }

    inline void bindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
    {
        ASSERT(commandBuffer.valid());
        vkCmdBindPipeline(commandBuffer.getHandle(), pipelineBindPoint, pipeline);
    }

    inline void setViewport(const VkViewport& viewport)
    {
        ASSERT(commandBuffer.valid());
        vkCmdSetViewport(commandBuffer.getHandle(), 0, 1, &viewport);
    }

    inline void setScissor(const VkRect2D& scissor)
    {
        ASSERT(commandBuffer.valid());
        vkCmdSetScissor(commandBuffer.getHandle(), 0, 1, &scissor);
    }

    inline void bindVertexBuffers(VkBuffer buffer, VkDeviceSize offset)
    {
        ASSERT(commandBuffer.valid());
        VkBuffer buffers[] = { buffer };
        VkDeviceSize offsets[] = { offset };
        vkCmdBindVertexBuffers(commandBuffer.getHandle(), 0, 1, buffers, offsets);
    }

    inline void bindIndexBuffers(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
    {
        ASSERT(commandBuffer.valid());
        vkCmdBindIndexBuffer(commandBuffer.getHandle(), buffer, offset, indexType);
    }

    inline void bindDescriptorSets(VkPipelineBindPoint pipelineBindPoint,
                                   VkPipelineLayout pipelineLayout,
                                   uint32_t firstSet,
                                   uint32_t descriptorSetCount,
                                   const VkDescriptorSet* pDescriptorSets,
                                   uint32_t dynamicOffsetCount,
                                   const uint32_t* pDynamicOffsets)
    {
        ASSERT(commandBuffer.valid());
        vkCmdBindDescriptorSets(commandBuffer.getHandle(), pipelineBindPoint, pipelineLayout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }

    inline void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        ASSERT(commandBuffer.valid());
        vkCmdDraw(commandBuffer.getHandle(), vertexCount, instanceCount, firstVertex, firstInstance);
    }

    inline void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
    {
        ASSERT(commandBuffer.valid());
        vkCmdDrawIndexed(commandBuffer.getHandle(), indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    inline void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        ASSERT(commandBuffer.valid());
        vkCmdDispatch(commandBuffer.getHandle(), groupCountX, groupCountY, groupCountZ);
    }

    inline void dispatchIndirect(VkBuffer buffer, size_t offset)
    {
        ASSERT(commandBuffer.valid());
        vkCmdDispatchIndirect(commandBuffer.getHandle(), buffer, offset);
    }

    inline void setShadingRate(uint32_t width, uint32_t height)
    {
        ASSERT(commandBuffer.valid());

        VkExtent2D fragmentSize;
        fragmentSize.width = width;
        fragmentSize.height = height;
        VkFragmentShadingRateCombinerOpKHR ops[2] = { VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR, VK_FRAGMENT_SHADING_RATE_COMBINER_OP_KEEP_KHR };
#if PLATFORM_ANDROID || USE_WIN_VULKAN_WRAPPER
        vkCmdSetFragmentShadingRateKHR(commandBuffer.getHandle(), &fragmentSize, ops);
#endif
    }

    inline void pipelineBarrier(VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkDependencyFlags dependencyFlags,
        uint32_t memoryBarrierCount,
        const VkMemoryBarrier* memoryBarriers,
        uint32_t bufferMemoryBarrierCount,
        const VkBufferMemoryBarrier* bufferMemoryBarriers,
        uint32_t imageMemoryBarrierCount,
        const VkImageMemoryBarrier* imageMemoryBarriers)
    {
        ASSERT(commandBuffer.valid());
        vkCmdPipelineBarrier(commandBuffer.getHandle(), srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount,
            memoryBarriers, bufferMemoryBarrierCount, bufferMemoryBarriers,
            imageMemoryBarrierCount, imageMemoryBarriers);
    }

    inline void clearColorImage(VkImage image, VkImageLayout imageLayout, const VkClearColorValue& color, const VkImageSubresourceRange& ranges)
    {
        ASSERT(commandBuffer.valid());
        vkCmdClearColorImage(commandBuffer.getHandle(), image, imageLayout, &color, 1, &ranges);
    }

    inline void copyImage(VkImage srcImage, VkImage dstImage, const VkImageCopy& copyRegion)
    {
        ASSERT(commandBuffer.valid());
        vkCmdCopyImage(commandBuffer.getHandle(), srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    }

    inline void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, const VkBufferCopy& copyRegion)
    {
        ASSERT(commandBuffer.valid());
        vkCmdCopyBuffer(commandBuffer.getHandle(), srcBuffer, dstBuffer, 1, &copyRegion);
    }

    inline void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, const VkBufferImageCopy& copyRegion)
    {
        ASSERT(commandBuffer.valid());
        vkCmdCopyBufferToImage(commandBuffer.getHandle(), srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
    }

    inline void buildAccelerationStructures(uint32_t count,
        const VkAccelerationStructureBuildGeometryInfoKHR* accelerationStructureBuildGeometryInfo,
        const VkAccelerationStructureBuildRangeInfoKHR* const* buildRangeInfos)
    {
        ASSERT(commandBuffer.valid());
        vkCmdBuildAccelerationStructuresKHR(commandBuffer.getHandle(), count, accelerationStructureBuildGeometryInfo, buildRangeInfos);
    }
   
    inline void TraceRays(const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                          const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                          const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                          const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable,
                          uint32_t width, uint32_t height, uint32_t depth)
    {
        ASSERT(commandBuffer.valid());

        vkCmdTraceRaysKHR(
            commandBuffer.getHandle(),
            pRaygenShaderBindingTable,
            pMissShaderBindingTable,
            pHitShaderBindingTable,
            pCallableShaderBindingTable,
            width,
            height,
            depth);
    }

private:
	handle::CommandBuffer commandBuffer;
    handle::Fence fence;
    Transition* transition;
};
}