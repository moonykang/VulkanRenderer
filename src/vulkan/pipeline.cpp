#include "rhi/context.h"
#include "vulkan/commandBuffer.h"
#include "vulkan/pipeline.h"
#include "vulkan/buffer.h"
#include "vulkan/rendertarget.h"
#include "vulkan/descriptor.h"
#include "vulkan/extension.h"

namespace vk
{
void ShaderModuleContainer::destroy(rhi::Context* rhiContext)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);

    for (auto& shaderModule : shaderModules)
    {
        shaderModule.destroy(context->getDevice());
    }
    shaderModules.clear();
}

void ShaderModuleContainer::build(rhi::Context* rhiContext)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);

    VkShaderModuleCreateInfo shaderModuleCrateInfo = {};
    shaderModuleCrateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCrateInfo.pNext = nullptr;

    for (auto& shader : shaders)
    {
        auto& shaderModule = shaderModules.emplace_back();
        shaderModuleCrateInfo.codeSize = static_cast<uint32_t>(shader.code.size());
        shaderModuleCrateInfo.pCode = reinterpret_cast<const uint32_t*>(shader.code.data());
        VKCALL(shaderModule.init(context->getDevice(), shaderModuleCrateInfo));

        auto& pipelineShaderStageCreateInfo = pipelineShaderStageCreateInfos.emplace_back();
        pipelineShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        pipelineShaderStageCreateInfo.stage = convertToVkShaderStage(shader.shaderStage);
        pipelineShaderStageCreateInfo.module = shaderModule.getHandle();
        pipelineShaderStageCreateInfo.pName = "main";
    }
}

Pipeline::Pipeline(VkPipelineBindPoint pipelineBindPoint)
    : pipelineBindPoint(pipelineBindPoint)
{
}

void Pipeline::destroy(Context* context)
{
    pipelineLayout.destroy(context->getDevice());
    pipeline.destroy(context->getDevice());
}

VkPipelineLayout Pipeline::getLayout()
{
    return pipelineLayout.getHandle();
}

VkPipeline Pipeline::getHandle()
{
    return pipeline.getHandle();
}

VkPipelineBindPoint Pipeline::getBindPoint()
{
    return pipelineBindPoint;
}

GraphicsPipeline::GraphicsPipeline()
    : vk::Pipeline(VK_PIPELINE_BIND_POINT_GRAPHICS)
{
}

void GraphicsPipeline::destroy(rhi::Context* rhiContext)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);
    
    vk::Pipeline::destroy(context);
}

void GraphicsPipeline::buildGraphics(rhi::Context* rhiContext, rhi::PipelineState& pipelineState, rhi::ShaderModuleContainer* rhiShaderModule, rhi::VertexBuffer* inVertexbuffer, std::vector<rhi::DescriptorSet*>& descriptorSets, rhi::RenderTarget* rhiRenderTarget)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);

    vk::RenderTarget* renderTarget = reinterpret_cast<vk::RenderTarget*>(rhiRenderTarget);

    ShaderModuleContainer* shaderModule = reinterpret_cast<ShaderModuleContainer*>(rhiShaderModule);

    auto& shaderStageInfos = shaderModule->getPipelineShaderStageCreateInfos();

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    if (inVertexbuffer != nullptr)
    {
        VertexBuffer* vertexBuffer = reinterpret_cast<VertexBuffer*>(inVertexbuffer);
        vertexBuffer->updateVertexInputState(&vertexInputState);
    }

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = convertToVkPrimitiveTopology(pipelineState.getTopology());
    inputAssemblyState.primitiveRestartEnable = VK_FALSE;


    std::vector<VkPipelineTessellationStateCreateInfo> tessellationState;
    if (pipelineState.getTessellationPatchControl() != 0)
    {
        VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo = {};
        tessellationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
        tessellationStateCreateInfo.pNext = nullptr;
        tessellationStateCreateInfo.patchControlPoints = pipelineState.getTessellationPatchControl();
        inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
        tessellationState.push_back(tessellationStateCreateInfo);
    }

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.polygonMode = convertToVkPolygonMode(pipelineState.getPolygonMode());
    rasterizationState.lineWidth = 1.0f;
    rasterizationState.cullMode = convertToVkCullMode(pipelineState.getCullMode());
    rasterizationState.frontFace = convertToVkFrontFace(pipelineState.getFrontFace());
    rasterizationState.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.sampleShadingEnable = VK_FALSE;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
    auto& colorBlendMasks = pipelineState.getColorBlendMasks();

    for (auto& colorBlendMasks : colorBlendMasks)
    {
        VkPipelineColorBlendAttachmentState& colorBlendAttachmentState = colorBlendAttachmentStates.emplace_back();
        colorBlendAttachmentState.colorWriteMask = convertToColorComponentFlags(colorBlendMasks);
        colorBlendAttachmentState.blendEnable = VK_FALSE;
    }

    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.logicOpEnable = VK_FALSE;
    colorBlendState.logicOp = VK_LOGIC_OP_COPY;
    colorBlendState.attachmentCount = static_cast<uint32_t>(colorBlendAttachmentStates.size());
    colorBlendState.pAttachments = colorBlendAttachmentStates.data();;
    colorBlendState.blendConstants[0] = 0.0f;
    colorBlendState.blendConstants[1] = 0.0f;
    colorBlendState.blendConstants[2] = 0.0f;
    colorBlendState.blendConstants[3] = 0.0f;

    std::vector<VkDynamicState> dynamicStateList;
    {
        dynamicStateList.push_back(VK_DYNAMIC_STATE_VIEWPORT);
        dynamicStateList.push_back(VK_DYNAMIC_STATE_SCISSOR);
    }

    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateList.size());
    dynamicState.pDynamicStates = dynamicStateList.data();


    std::vector<VkDescriptorSetLayout> setLayouts;
    ASSERT(!descriptorSets.empty());
    for (auto& descriptorSet : descriptorSets)
    {
        setLayouts.push_back(reinterpret_cast<DescriptorSet*>(descriptorSet)->getLayout());
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    VKCALL(pipelineLayout.init(context->getDevice(), pipelineLayoutInfo));

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
    graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStageInfos.size());
    graphicsPipelineCreateInfo.pStages = shaderStageInfos.data();
    graphicsPipelineCreateInfo.pVertexInputState = &vertexInputState;
    graphicsPipelineCreateInfo.pTessellationState = tessellationState.data();
    graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    graphicsPipelineCreateInfo.pViewportState = &viewportState;
    graphicsPipelineCreateInfo.pRasterizationState = &rasterizationState;
    graphicsPipelineCreateInfo.pMultisampleState = &multisampleState;
    graphicsPipelineCreateInfo.pColorBlendState = &colorBlendState;
    graphicsPipelineCreateInfo.pDynamicState = &dynamicState;
    graphicsPipelineCreateInfo.layout = pipelineLayout.getHandle();
    graphicsPipelineCreateInfo.renderPass = renderTarget->getRenderpass();
    graphicsPipelineCreateInfo.subpass = 0;
    graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilState = {};    
    {
        rhi::PipelineState::DepthStencilState& depthStencilState = pipelineState.getDepthStencilState();
        pipelineDepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipelineDepthStencilState.depthTestEnable = depthStencilState.depthTestEnable;
        pipelineDepthStencilState.depthWriteEnable = depthStencilState.depthWriteEnable;
        pipelineDepthStencilState.depthCompareOp = convertToVkCompareOp(depthStencilState.depthCompareOp);
        pipelineDepthStencilState.depthBoundsTestEnable = depthStencilState.depthBoundsTestEnable;
        pipelineDepthStencilState.stencilTestEnable = depthStencilState.stencilTestEnable;

        pipelineDepthStencilState.front.failOp = convertToVkStencilOp(depthStencilState.front.failOp);
        pipelineDepthStencilState.front.passOp = convertToVkStencilOp(depthStencilState.front.passOp);
        pipelineDepthStencilState.front.depthFailOp = convertToVkStencilOp(depthStencilState.front.depthFailOp);
        pipelineDepthStencilState.front.compareOp = convertToVkCompareOp(depthStencilState.front.compareOp);
        pipelineDepthStencilState.front.compareMask = depthStencilState.front.compareMask;
        pipelineDepthStencilState.front.writeMask = depthStencilState.front.writeMask;
        pipelineDepthStencilState.front.reference = depthStencilState.front.reference;

        pipelineDepthStencilState.back.failOp = convertToVkStencilOp(depthStencilState.back.failOp);
        pipelineDepthStencilState.back.passOp = convertToVkStencilOp(depthStencilState.back.passOp);
        pipelineDepthStencilState.back.depthFailOp = convertToVkStencilOp(depthStencilState.back.depthFailOp);
        pipelineDepthStencilState.back.compareOp = convertToVkCompareOp(depthStencilState.back.compareOp);
        pipelineDepthStencilState.back.compareMask = depthStencilState.back.compareMask;
        pipelineDepthStencilState.back.writeMask = depthStencilState.back.writeMask;
        pipelineDepthStencilState.back.reference = depthStencilState.back.reference;

        pipelineDepthStencilState.minDepthBounds = depthStencilState.minDepthBounds;
        pipelineDepthStencilState.maxDepthBounds = depthStencilState.maxDepthBounds;

    }
    graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilState;


    VKCALL(pipeline.initGraphics(context->getDevice(), graphicsPipelineCreateInfo, context->getPipelineCache()));
}

void GraphicsPipeline::bind(rhi::Context* context)
{
    Context* contextVk = reinterpret_cast<Context*>(context);
    CommandBuffer* commandBuffer = contextVk->getActiveCommandBuffer();
    commandBuffer->bindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getHandle());
}

ComputePipeline::ComputePipeline()
    : vk::Pipeline(VK_PIPELINE_BIND_POINT_COMPUTE)
{

}

void ComputePipeline::destroy(rhi::Context* rhiContext)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);
    vk::Pipeline::destroy(context);
}

void ComputePipeline::buildCompute(rhi::Context* rhiContext, rhi::ShaderModuleContainer* rhiShaderModule, std::vector<rhi::DescriptorSet*>& descriptorSets)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);
    ShaderModuleContainer* shaderModule = reinterpret_cast<ShaderModuleContainer*>(rhiShaderModule);
    std::vector<VkDescriptorSetLayout> setLayouts;
    ASSERT(!descriptorSets.empty());
    for (auto& descriptorSet : descriptorSets)
    {
        setLayouts.push_back(reinterpret_cast<DescriptorSet*>(descriptorSet)->getLayout());
    }

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = setLayouts.data();

    VKCALL(pipelineLayout.init(context->getDevice(), pipelineLayoutInfo));

    auto& shaderStageInfos = shaderModule->getPipelineShaderStageCreateInfos();

    VkComputePipelineCreateInfo computePipelineCreateInfo = {};
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.layout = pipelineLayout.getHandle();
    computePipelineCreateInfo.stage = shaderStageInfos[0];

    VKCALL(pipeline.initCompute(context->getDevice(), computePipelineCreateInfo, nullptr));
}

void ComputePipeline::bind(rhi::Context* rhiContext)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);
    CommandBuffer* commandBuffer = context->getActiveCommandBuffer();
    commandBuffer->bindPipeline(pipelineBindPoint, pipeline.getHandle());
}

RayTracingPipeline::RayTracingPipeline()
    : vk::Pipeline(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
    , rayGenShaderBindingTable(nullptr)
    , rayMissShaderBindingTable(nullptr)
    , rayHitShaderBindingTable(nullptr)
{
}

void RayTracingPipeline::destroy(rhi::Context* rhiContext)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);

    vk::Pipeline::destroy(context);

    if (rayGenShaderBindingTable != nullptr)
    {
        rayGenShaderBindingTable->destroy(context);
        delete rayGenShaderBindingTable;
        rayGenShaderBindingTable = nullptr;
    }

    if (rayMissShaderBindingTable != nullptr)
    {
        rayMissShaderBindingTable->destroy(context);
        delete rayMissShaderBindingTable;
        rayMissShaderBindingTable = nullptr;
    }

    if (rayHitShaderBindingTable != nullptr)
    {
        rayHitShaderBindingTable->destroy(context);
        delete rayHitShaderBindingTable;
        rayHitShaderBindingTable = nullptr;
    }
}

void RayTracingPipeline::buildRayTracing(rhi::Context* rhiContext, rhi::ShaderModuleContainer* rhiShaderModule, rhi::DescriptorSet* descriptorSet)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);

    ShaderModuleContainer* shaderModule = reinterpret_cast<ShaderModuleContainer*>(rhiShaderModule);

    VkSpecializationMapEntry specializationMapEntry = {};
    specializationMapEntry.constantID = 0;
    specializationMapEntry.offset = 0;
    specializationMapEntry.size = sizeof(uint32_t);

    uint32_t maxRecursion = 2;
    VkSpecializationInfo specializationInfo = {};
    specializationInfo.mapEntryCount = 1;
    specializationInfo.pMapEntries = &specializationMapEntry;
    specializationInfo.dataSize = sizeof(maxRecursion);
    specializationInfo.pData = &maxRecursion;

    auto& shaderStages = shaderModule->getPipelineShaderStageCreateInfos();
    {
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup = {};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader = 0;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
        shaderStages[0].pSpecializationInfo = &specializationInfo;
    }

    {
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup = {};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shaderGroup.generalShader = 1;
        shaderGroup.closestHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
        shaderGroup.generalShader = 2;
        shaderGroups.push_back(shaderGroup);
    }

    {
        VkRayTracingShaderGroupCreateInfoKHR shaderGroup = {};
        shaderGroup.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shaderGroup.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shaderGroup.generalShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.closestHitShader = 3;
        shaderGroup.anyHitShader = VK_SHADER_UNUSED_KHR;
        shaderGroup.intersectionShader = VK_SHADER_UNUSED_KHR;
        shaderGroups.push_back(shaderGroup);
    }

    std::vector<VkDescriptorSetLayout> setLayouts;
    if (descriptorSet)
    {
        setLayouts.push_back(reinterpret_cast<DescriptorSet*>(descriptorSet)->getLayout());
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
    pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();

    VKCALL(pipelineLayout.init(context->getDevice(), pipelineLayoutCreateInfo));

    VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {};
    rayTracingPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    rayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
    rayTracingPipelineCreateInfo.pStages = shaderStages.data();
    rayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
    rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
    rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = maxRecursion;
    rayTracingPipelineCreateInfo.layout = pipelineLayout.getHandle();

    VKCALL(pipeline.initRayTracing(context->getDevice(), rayTracingPipelineCreateInfo, nullptr));

    const auto& rayTracingPipelineProps = context->getRayTracingPipelineProperties();
    const uint32_t handleSize = rayTracingPipelineProps.shaderGroupHandleSize;
    const uint32_t handleSizeAligned = Util::align(rayTracingPipelineProps.shaderGroupHandleSize, rayTracingPipelineProps.shaderGroupHandleAlignment);
    const uint32_t groupCount = static_cast<uint32_t>(shaderGroups.size());
    const uint32_t sbtSize = groupCount * handleSizeAligned;

    std::vector<uint8_t> shaderHandleStorage(sbtSize);
    VKCALL(vkGetRayTracingShaderGroupHandlesKHR(context->getDevice(), pipeline.getHandle(), 0, groupCount, sbtSize, shaderHandleStorage.data()));

    const VkBufferUsageFlags bufferUsageFlags = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
    const VkMemoryPropertyFlags memoryUsageFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    rayGenShaderBindingTable = BufferFactory::createBuffer(rhi::BufferType::HostCoherent, bufferUsageFlags, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, handleSize);
    rayGenShaderBindingTable->initBuffer(context, bufferUsageFlags);

    rayMissShaderBindingTable = BufferFactory::createBuffer(rhi::BufferType::HostCoherent, bufferUsageFlags, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, handleSize * 2);
    rayMissShaderBindingTable->initBuffer(context, bufferUsageFlags);

    rayHitShaderBindingTable = BufferFactory::createBuffer(rhi::BufferType::HostCoherent, bufferUsageFlags, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, handleSize);
    rayHitShaderBindingTable->initBuffer(context, bufferUsageFlags);

    rayGenShaderBindingTable->mapMemory(context, handleSize, shaderHandleStorage.data());
    rayMissShaderBindingTable->mapMemory(context, handleSize * 2, shaderHandleStorage.data() + handleSizeAligned);
    rayHitShaderBindingTable->mapMemory(context, handleSize, shaderHandleStorage.data() + (handleSizeAligned * 3));
}

void RayTracingPipeline::bind(rhi::Context* rhiContext)//, rhi::DescriptorSet* rhiDescriptorSet)
{
    Context* context = reinterpret_cast<Context*>(rhiContext);

    CommandBuffer* commandBuffer = context->getActiveCommandBuffer();

    /*
    DescriptorSet* descriptorSet = reinterpret_cast<DescriptorSet*>(rhiDescriptorSet);

    commandBuffer->bindDescriptorSets(
        getBindPoint(), getLayout(), 0, 1, &(descriptorSet->getHandle()), 0, nullptr);
        */

    const uint32_t handleSizeAligned = context->getRayTracingPipelineProperties().shaderGroupHandleAlignment;

    VkStridedDeviceAddressRegionKHR raygenShaderSbtEntry{};
    raygenShaderSbtEntry.deviceAddress = rayGenShaderBindingTable->getDeviceAddress(context->getDevice());
    raygenShaderSbtEntry.stride = handleSizeAligned;
    raygenShaderSbtEntry.size = handleSizeAligned;

    VkStridedDeviceAddressRegionKHR missShaderSbtEntry{};
    missShaderSbtEntry.deviceAddress = rayMissShaderBindingTable->getDeviceAddress(context->getDevice());
    missShaderSbtEntry.stride = handleSizeAligned;
    missShaderSbtEntry.size = handleSizeAligned;

    VkStridedDeviceAddressRegionKHR hitShaderSbtEntry{};
    hitShaderSbtEntry.deviceAddress = rayHitShaderBindingTable->getDeviceAddress(context->getDevice());
    hitShaderSbtEntry.stride = handleSizeAligned;
    hitShaderSbtEntry.size = handleSizeAligned;

    VkStridedDeviceAddressRegionKHR callableShaderSbtEntry{};

    commandBuffer->bindPipeline(getBindPoint(), pipeline.getHandle());
    commandBuffer->TraceRays(
        &raygenShaderSbtEntry,
        &missShaderSbtEntry,
        &hitShaderSbtEntry,
        &callableShaderSbtEntry,
        1024,
        1024,
        1);
}

} // namespace vk