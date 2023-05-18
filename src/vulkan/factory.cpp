#include "vulkan/context.h"
#include "vulkan/descriptor.h"
#include "vulkan/pipeline.h"
#include "vulkan/rendertarget.h"
#include "vulkan/buffer.h"
#include "vulkan/texture.h"
#include "vulkan/accelerationStructure.h"

namespace vk
{
rhi::RenderTarget* Context::createRenderTarget(rhi::RenderTargetType type, uint16_t width, uint16_t height)
{
    switch (type)
    {
    case rhi::RenderTargetType::Graphics:
        return new RenderTarget(width, height);
    case rhi::RenderTargetType::Surface:
        return new SurfaceRenderTarget(width, height);
    case rhi::RenderTargetType::Compute:
    case rhi::RenderTargetType::RayTracing:
        return new NullRenderTarget(width, height);
    default:
        UNREACHABLE();
        return nullptr;
    }
}

rhi::Pipeline* Context::createPipeline(rhi::PipelineType type)
{
    switch (type)
    {
    case rhi::PipelineType::Graphics:
        return new GraphicsPipeline();
    case rhi::PipelineType::RayTracing:
        return new RayTracingPipeline();
    case rhi::PipelineType::Compute:
        return new ComputePipeline();
    default:
        UNREACHABLE();
        return nullptr;
    }
}

rhi::ShaderModuleContainer* Context::createShaderModule()
{
    return new ShaderModuleContainer();
}

rhi::VertexBuffer* Context::createVertexBuffer()
{
    return new VertexBuffer();
}

rhi::IndexBuffer* Context::createIndexBuffer(rhi::IndexSize indexSize)
{
    switch (indexSize)
    {
    case rhi::IndexSize::None:
        return new NullIndexBuffer();
    default:
        return new IndexBuffer(indexSize);
    }
}

rhi::UniformBuffer* Context::createUniformBuffer(rhi::BufferType bufferType)
{
    return new UniformBuffer(bufferType);
}

rhi::StorageBuffer* Context::createStorageBuffer(rhi::BufferType bufferType, rhi::BufferUsageFlags usage)
{
    VkBufferUsageFlags bufferUsageFlags = convertToVkBufferUsageFlag(usage);
    return new StorageBuffer(bufferType, bufferUsageFlags);
}

rhi::DescriptorSet* Context::createDescriptorSet()
{
    return new DescriptorSet();
}

rhi::Texture* Context::createTexture(Format format, uint32_t width, uint32_t height, ImageLayout initialLayout, uint32_t usage)
{
    return new Texture(format, width, height, initialLayout, usage);
}

rhi::Texture* Context::createTexture(Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels,
                                     uint32_t layers, ImageLayout initialLayout, uint32_t usage)
{
    return new Texture(format, width, height, depth, samples, mipLevels, layers, initialLayout, usage);
}

rhi::ScratchBuffer* Context::createScratchBuffer(rhi::BufferUsageFlags bufferUsage)
{
    return new ScratchBuffer(bufferUsage);
}

rhi::AccStructureManager* Context::createAccStructureManager()
{
    return new AccStructureManager();
}

rhi::BottomLevelAccStructure* Context::createBottomLevelAccStructure()
{
    return new BottomLevelAccStructure();
}
}