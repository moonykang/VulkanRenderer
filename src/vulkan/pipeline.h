#pragma once

#include <vector>
#include "rhi/pipeline.h"
#include "vulkan/vk_wrapper.h"

namespace rhi
{
class Context;
class VertexBuffer;
class DescriptorSetLayout;
class RenderTarget;
}

namespace vk
{
class Buffer;
class Context;

class ShaderModuleContainer : public rhi::ShaderModuleContainer
{
public:
    void destroy(rhi::Context* context) override;

    void build(rhi::Context* context) override;

    std::vector<VkPipelineShaderStageCreateInfo>& getPipelineShaderStageCreateInfos() { return pipelineShaderStageCreateInfos; }
private:
    std::vector<handle::ShaderModule> shaderModules;
    std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos;
};

class Pipeline
{
public:
    Pipeline(VkPipelineBindPoint pipelineBindPoint);

    void destroy(Context* context);

    VkPipelineLayout getLayout();

    VkPipeline getHandle();

    VkPipelineBindPoint getBindPoint();
protected:
    handle::Pipeline pipeline;
    handle::PipelineLayout pipelineLayout;
    VkPipelineBindPoint pipelineBindPoint;
};

class GraphicsPipeline : public rhi::GraphicsPipeline, public Pipeline
{
public:
    GraphicsPipeline();

    void destroy(rhi::Context* context) override;

    void buildGraphics(rhi::Context* context, rhi::PipelineState& pipelineState, rhi::ShaderModuleContainer* shaderModule, rhi::VertexBuffer* inVertexbuffer, std::vector<rhi::DescriptorSet*>& descriptorSets, rhi::RenderTarget* renderTarget) override;

    void bind(rhi::Context* context) override;
};

class ComputePipeline : public rhi::ComputePipeline, public Pipeline
{
public:
    ComputePipeline();

    void destroy(rhi::Context* context) override;

    void buildCompute(rhi::Context* context, rhi::ShaderModuleContainer* shaderModule, std::vector<rhi::DescriptorSet*>& descriptorSet) override;

    void bind(rhi::Context* context) override;
};

class RayTracingPipeline : public rhi::RayTracingPipeline, public Pipeline
{
public:
    RayTracingPipeline();

    void destroy(rhi::Context* context) override;

    void buildRayTracing(rhi::Context* context, rhi::ShaderModuleContainer* shaderModule, rhi::DescriptorSet* descriptorSet) override;

    void bind(rhi::Context* context) override;
private:
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

    Buffer* rayGenShaderBindingTable;
    Buffer* rayMissShaderBindingTable;
    Buffer* rayHitShaderBindingTable;
};
}