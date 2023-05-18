#pragma once

#include <string>
#include "rhi/buffer.h"
#include "platform/assetManager.h"
#include "rhi/rendertarget.h"
#include "vulkan/context.h"
#include "rhi/resources.h"

namespace rhi
{
class Context;
class DescriptorSet;

class PipelineState
{
public:
	struct StencilState
	{
		StencilOp failOp;
		StencilOp passOp;
		StencilOp depthFailOp;
		CompareOp compareOp;
		uint32_t compareMask;
		uint32_t writeMask;
		uint32_t reference;
		
		StencilState()
			: failOp(StencilOp::KEEP)
			, passOp(StencilOp::KEEP)
			, depthFailOp(StencilOp::KEEP)
			, compareOp(CompareOp::NEVER)
			, compareMask(0)
			, writeMask(0)
			, reference(0)
		{
		}
	};

	struct DepthStencilState
	{
		bool depthTestEnable;
		bool depthWriteEnable;
		CompareOp depthCompareOp;
		bool depthBoundsTestEnable;
		bool stencilTestEnable;
		StencilState front;
		StencilState back;
		float minDepthBounds;
		float maxDepthBounds;

		DepthStencilState(bool depthTestEnable = false
				, bool depthWriteEnable = false
				, CompareOp depthCompareOp = CompareOp::NEVER
				, bool depthBoundsTestEnable = false
				, bool stencilTestEnable = false
				, StencilState front = {}
				, StencilState back = {}
				, float minDepthBounds = 0.f
				, float maxDepthBounds = 0.f
			)
			: depthTestEnable(depthTestEnable)
			, depthWriteEnable(depthWriteEnable)
			, depthCompareOp(depthCompareOp)
			, depthBoundsTestEnable(depthBoundsTestEnable)
			, stencilTestEnable(stencilTestEnable)
			, front(front)
			, back(back)
			, minDepthBounds(minDepthBounds)
			, maxDepthBounds(maxDepthBounds)
		{
		}
	};
public:
	PipelineState(FrontFace frontFace = FrontFace::COUNTER_CLOCKWISE
				, CullMode cullMode = CullMode::BACK_BIT
				, PolygonMode polygonMode = PolygonMode::FILL
				, uint32_t tessellationPatchControl = 0
				, std::vector<ColorBlendMaskFlags> colorBlendMaskFlags = { COLOR_COMPONENT_ALL_BIT }
				, Topology topology = Topology::TRIANGLE_LIST
				, DepthStencilState depthStencilState = {});

	FrontFace getFrontFace();
	CullMode getCullMode();
	PolygonMode getPolygonMode();
	uint32_t getTessellationPatchControl();
	std::vector<ColorBlendMaskFlags>& getColorBlendMasks();
	Topology getTopology();
	DepthStencilState& getDepthStencilState();

	FrontFace frontFace;
	CullMode cullMode;
	PolygonMode polygonMode;
	uint32_t tessellationPatchControl;
	std::vector<ColorBlendMaskFlags> colorBlendMasks;
	Topology topology;
	DepthStencilState depthStencilState;
};

struct ShaderCode
{
	rhi::ShaderStage shaderStage;
	util::MemoryBuffer code;
};

class ShaderModuleContainer
{
public:
	virtual ~ShaderModuleContainer() = default;

	void init(platform::AssetManager* assetManager, std::string path);

	void updateShaderCode(platform::AssetManager* assetManager, rhi::ShaderStage shaderStage, std::string path);

	virtual void destroy(Context * context) = 0;

	virtual void build(Context* context) = 0;

protected:
	std::vector<ShaderCode> shaders;
};

class Pipeline
{
public:
	virtual ~Pipeline() = default;

	virtual void destroy(Context* context) = 0;

	virtual void buildGraphics(Context* context, PipelineState& pipelineState, ShaderModuleContainer* shaderModule, VertexBuffer* vertexBuffer, std::vector<DescriptorSet*>& descriptorSet, RenderTarget* renderTarget) = 0;

	virtual void buildRayTracing(Context* context, ShaderModuleContainer* shaderModule, DescriptorSet* descriptorSet) = 0;

	virtual void buildCompute(Context* context, ShaderModuleContainer* shaderModule, std::vector<DescriptorSet*>& descriptorSet) = 0;

	virtual void bind(Context* context) = 0;
};

class GraphicsPipeline : public Pipeline
{
public:
	virtual void destroy(Context* context) override = 0;

	virtual void buildGraphics(Context* context, PipelineState& pipelineState, ShaderModuleContainer* shaderModule, VertexBuffer* vertexBuffer, std::vector<DescriptorSet*>& descriptorSet, RenderTarget* renderTarget) = 0;

	void buildRayTracing(Context* context, ShaderModuleContainer* shaderModule, DescriptorSet* descriptorSet) override {}

	void buildCompute(Context* context, ShaderModuleContainer* shaderModule, std::vector<DescriptorSet*>& descriptorSet) override {};

	virtual void bind(Context* context) override = 0;
};

class ComputePipeline : public Pipeline
{
public:
	virtual void destroy(Context* context) override = 0;

	void buildGraphics(Context* context, PipelineState& pipelineState, ShaderModuleContainer* shaderModule, VertexBuffer* vertexBuffer, std::vector<DescriptorSet*>& descriptorSet, RenderTarget* renderTarget) override {}

	virtual void buildRayTracing(Context* context, ShaderModuleContainer* shaderModule, DescriptorSet* descriptorSet) {}

	virtual void buildCompute(Context* context, ShaderModuleContainer* shaderModule, std::vector<DescriptorSet*>& descriptorSet) = 0;

	virtual void bind(Context* context) override = 0;
};

class RayTracingPipeline : public Pipeline
{
public:
	virtual void destroy(Context* context) override = 0;

	void buildGraphics(Context* context, PipelineState& pipelineState, ShaderModuleContainer* shaderModule, VertexBuffer* vertexBuffer, std::vector<DescriptorSet*>& descriptorSet, RenderTarget* renderTarget) override {}

	virtual void buildRayTracing(Context* context, ShaderModuleContainer* shaderModule, DescriptorSet* descriptorSet) override = 0;
	
	void buildCompute(Context* context, ShaderModuleContainer* shaderModule, std::vector<DescriptorSet*>& descriptorSet) override {};

	virtual void bind(Context* context) override = 0;
};
}