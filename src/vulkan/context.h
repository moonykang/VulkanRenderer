#pragma once

#include <string>
#include <map>
#include "rhi/context.h"
#include "vulkan/vk_wrapper.h"
#include "vulkan/descriptorPool.h"

namespace platform
{
class Window;
}

namespace vk
{
class DescriptorPool;
class Surface;
class CommandBuffer;
class CommandBufferManager;
class Queue;
class DeviceExtension;

class Context : public rhi::Context
{
public:
    Context();

    bool init(platform::Window* window) override;

    bool terminate() override;

    bool present() override;

    bool submit() override;

    void wait() override;

    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

    void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;

    void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;

    void dispatchIndirect(rhi::StorageBuffer* buffer) override;

    inline const std::string& getGpuName() override { return gpuName; }

// Factory
public:
    rhi::RenderTarget* createRenderTarget(rhi::RenderTargetType type, uint16_t width, uint16_t height) override;

    rhi::Pipeline* createPipeline(rhi::PipelineType type) override;

    rhi::ShaderModuleContainer* createShaderModule() override;

    rhi::VertexBuffer* createVertexBuffer() override;

    rhi::IndexBuffer* createIndexBuffer(rhi::IndexSize indexSize = rhi::IndexSize::UINT32) override;

    rhi::UniformBuffer* createUniformBuffer(rhi::BufferType bufferType) override;

    rhi::StorageBuffer* createStorageBuffer(rhi::BufferType bufferType, rhi::BufferUsageFlags usage) override;

    rhi::DescriptorSet* createDescriptorSet() override;

    rhi::Texture* createTexture(rhi::Format format, uint32_t width, uint32_t height, rhi::ImageLayout initialLayout, uint32_t usage) override;

    rhi::Texture* createTexture(rhi::Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels,
        uint32_t layers, rhi::ImageLayout initialLayout, uint32_t usage) override;

    rhi::ScratchBuffer* createScratchBuffer(rhi::BufferUsageFlags bufferUsage) override;

    rhi::AccStructureManager* createAccStructureManager() override;

    rhi::BottomLevelAccStructure* createBottomLevelAccStructure() override;
private:
    bool initInstance();

    bool initPhysicalDevice();

    bool initLogicalDevice();

    VkDeviceQueueCreateInfo getQueueCreateInfo(VkQueueFlags queueFlags, uint32_t* queueIndex, float queuePriority = 0);

    uint32_t getQueueFamilyIndex(VkQueueFlagBits queueFlags) const;

public:
    VkDevice getDevice();

    VkPhysicalDevice getPhysicalDevice();

    Surface* getSurface();

    CommandBufferManager* getCommandBufferManager();

    DescriptorPool* getDescriptorPool();

public:
    CommandBuffer* getActiveCommandBuffer();

    CommandBuffer* getUploadCommandBuffer();

    void submitUploadCommandBuffer();

    uint32_t getNextImageIndex();

    inline uint32_t getQueueFamilyIndex() { return queueFamilyIndex; }

    inline VkPhysicalDeviceProperties& getPhysicalDeviceProperties() { return physicalDeviceProperties; }

    inline VkPhysicalDeviceRayTracingPipelinePropertiesKHR& getRayTracingPipelineProperties()
    {
        return *reinterpret_cast<VkPhysicalDeviceRayTracingPipelinePropertiesKHR*>(devicePropertyMap[VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR]);
    }

    inline VkPhysicalDeviceAccelerationStructurePropertiesKHR& getAccelerationStructureProperties()
    {
        return *reinterpret_cast<VkPhysicalDeviceAccelerationStructurePropertiesKHR*>(devicePropertyMap[VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR]);
    }

    VkPipelineCache getPipelineCache() { return pipelineCache.getHandle(); }

private:
    bool enableValidationLayer = true;
    std::string name;
    handle::Instance instance;
    handle::Device device;
    handle::PhysicalDevice physicalDevice;
    handle::PipelineCache pipelineCache;

    uint32_t queueFamilyIndex;
    VkPhysicalDeviceProperties2 physicalDeviceProperties2;
    VkPhysicalDeviceProperties physicalDeviceProperties;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2;

    Surface* surface;
    CommandBufferManager* commandBufferManager;
    Queue* queue;
    DescriptorPool* descriptorPool;

    std::vector<InstanceExtension*> instanceExtensions;
    std::vector<DeviceExtension*> deviceExtensions;
    
    std::map<VkStructureType, void*> devicePropertyMap;

    std::string gpuName = "Unknown";
};
}