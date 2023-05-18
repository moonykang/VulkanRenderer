#pragma once
#include "platform/utils.h"
#include "rhi/resources.h"

namespace rhi
{
	class Context;
	class DescriptorSet;
	class UniformBuffer;
	class Pipeline;
	class GraphicsPipeline;
}

namespace model
{
class Object;
struct Material;

class Instance
{
public:
	Instance(Object* object, Instance* instance,
			uint32_t firstIndex, uint32_t indexCount,
			uint32_t firstVertex, uint32_t vertexCount,
			glm::mat4 transform);

	void init(rhi::Context* context);

	void destroy(rhi::Context* context);

	void build(rhi::Context* context);

	void draw(rhi::Context* context, rhi::GraphicsPipeline* pipeline);

	void updateMaterialDescriptorSet(rhi::DescriptorSet* descriptorSet);
private:
	Object* object;
	Instance* prevInstance;

	uint32_t firstIndex;
	uint32_t indexCount;
	uint32_t firstVertex;
	uint32_t vertexCount;

	struct InstanceUniformBlock
	{
		glm::mat4 transform;
	} ubo;

	rhi::UniformBuffer* instanceUniformBuffer;
	rhi::DescriptorSet* instanceDescriptorSet;

	// shared
	rhi::DescriptorSet* materialDescriptorSet;
};
}