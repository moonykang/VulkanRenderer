#pragma once

#include <vector>
#include "rhi/buffer.h"
#include "rhi/texture.h"
#include "vulkan/context.h"
#include "rhi/resources.h"

namespace rhi
{
class Context;
class Pipeline;
class AccStructureManager;
class ScratchBuffer;
class GraphicsPipeline;
class ComputePipeline;
class RayTracingPipeline;

class DescriptorInfo
{
public:
	DescriptorInfo(ShaderStageFlags stage, DescriptorType type, Descriptor* descriptor);

	ShaderStageFlags getStage();

	DescriptorType getType();

	Descriptor* getDescriptor();
protected:
	DescriptorType type;
	ShaderStageFlags stage;
	Descriptor* descriptor;
};

class DescriptorSet
{
public:
	DescriptorSet();

	virtual ~DescriptorSet() = default;

	virtual void destroy(Context* context) = 0;

	virtual void build(Context* context) = 0;

	void registerDescriptor(ShaderStageFlags stage, DescriptorType type, Descriptor* descriptor);

	virtual void bind(Context* context, GraphicsPipeline* pipeline, uint32_t binding) = 0;

	virtual void bind(Context* context, ComputePipeline* pipeline, uint32_t binding) = 0;

	virtual void bind(Context* context, RayTracingPipeline* pipeline, uint32_t binding) = 0;
protected:
	std::vector<DescriptorInfo> descriptors;
	uint32_t binding;
};
}