#include "model/instance.h"
#include "rhi/context.h"
#include "rhi/descriptor.h"
#include "rhi/resources.h"
#include "rhi/pipeline.h"
#include "model/object.h"

namespace model
{
Instance::Instance(Object* object, Instance* instance,
		uint32_t firstIndex, uint32_t indexCount,
		uint32_t firstVertex, uint32_t vertexCount,
		glm::mat4 transform)
	: object(object)
	, prevInstance(instance)
	, firstIndex(firstIndex)
	, indexCount(indexCount)
	, firstVertex(firstVertex)
	, vertexCount(vertexCount)
	, ubo({transform})
	, instanceUniformBuffer(nullptr)
	, instanceDescriptorSet(nullptr)
	, materialDescriptorSet(nullptr)
{
}

void Instance::init(rhi::Context* context)
{
	instanceUniformBuffer = context->createUniformBuffer(rhi::BufferType::HostCoherent);
	instanceUniformBuffer->set<InstanceUniformBlock>(1, &ubo);

	instanceDescriptorSet = context->createDescriptorSet();
	instanceDescriptorSet->registerDescriptor(rhi::ShaderStage::Vertex, rhi::DescriptorType::Uniform_Buffer, instanceUniformBuffer);
}

void Instance::destroy(rhi::Context* context)
{
	if (instanceUniformBuffer != nullptr)
	{
		instanceUniformBuffer->destroy(context);
		delete instanceUniformBuffer;
		instanceUniformBuffer = nullptr;
	}

	if (instanceDescriptorSet != nullptr)
	{
		instanceDescriptorSet->destroy(context);
		delete instanceDescriptorSet;
		instanceDescriptorSet = nullptr;
	}

	if (prevInstance != nullptr)
	{
		prevInstance->destroy(context);
		delete prevInstance;
		prevInstance = nullptr;
	}
}

void Instance::build(rhi::Context* context)
{
	ASSERT(instanceUniformBuffer);
	ASSERT(instanceDescriptorSet);

	instanceUniformBuffer->build(context);
	instanceDescriptorSet->build(context);

	object->udpateLocalDescriptorSet(instanceDescriptorSet);

	if (prevInstance != nullptr)
	{
		prevInstance->build(context);
	}
}

void Instance::draw(rhi::Context* context, rhi::GraphicsPipeline* pipeline)
{
	instanceDescriptorSet->bind(context, pipeline, 1);
	materialDescriptorSet->bind(context, pipeline, 2);

	context->drawIndexed(indexCount, 1, firstIndex, 0, 0);

	if (prevInstance != nullptr)
	{
		prevInstance->draw(context, pipeline);
	}
}

void Instance::updateMaterialDescriptorSet(rhi::DescriptorSet* descriptorSet)
{
	materialDescriptorSet = descriptorSet;
}
}
