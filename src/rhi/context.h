#pragma once

#include "platform/utils.h"
#include "rhi/resources.h"

namespace platform
{
class Window;
}

namespace rhi
{
class RenderTarget;
class Pipeline;
class ShaderModuleContainer;
class VertexBuffer;
class IndexBuffer;
class UniformBuffer;
class StorageBuffer;
class ScratchBuffer;
class DescriptorSet;
class DescriptorSetLayout;
class Texture;
class AccStructureManager;
class BottomLevelAccStructure;

class Context
{
public:
    ~Context() = default;

    virtual bool init(platform::Window* window) = 0;

    virtual bool terminate() = 0;

    virtual const std::string& getGpuName() = 0;

    virtual bool present() = 0;

    virtual bool submit() = 0;

    virtual void wait() = 0;

    virtual void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;

    virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;

    virtual void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;

    virtual void dispatchIndirect(StorageBuffer* buffer) = 0;
// Factory
public:
    virtual RenderTarget* createRenderTarget(RenderTargetType type, uint16_t width, uint16_t height) = 0;

    virtual Pipeline* createPipeline(PipelineType type) = 0;

    virtual ShaderModuleContainer* createShaderModule() = 0;

    virtual VertexBuffer* createVertexBuffer() = 0;

    virtual IndexBuffer* createIndexBuffer(IndexSize indexSize = IndexSize::UINT32) = 0;

    virtual UniformBuffer* createUniformBuffer(BufferType bufferType) = 0;

    virtual StorageBuffer* createStorageBuffer(BufferType bufferType, BufferUsageFlags usage) = 0;

    virtual DescriptorSet* createDescriptorSet() = 0;

    virtual Texture* createTexture(Format format, uint32_t width, uint32_t height, ImageLayout initialLayout, uint32_t usage) = 0;

    virtual Texture* createTexture(Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels,
        uint32_t layers, ImageLayout initialLayout, uint32_t usage) = 0;

    virtual ScratchBuffer* createScratchBuffer(BufferUsageFlags bufferUsage) = 0;

    virtual AccStructureManager* createAccStructureManager() = 0;

    virtual BottomLevelAccStructure* createBottomLevelAccStructure() = 0;

    uint32_t getWidth() { return renderTargetWidth; }
    uint32_t getHeight() { return renderTargetHeight; }
protected:
    uint32_t renderTargetWidth;
    uint32_t renderTargetHeight;
};
}