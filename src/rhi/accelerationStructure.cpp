#include "rhi/accelerationStructure.h"
#include "rhi/context.h"
#include "model/object.h"

namespace rhi
{
AccStructureInstance::AccStructureInstance(const glm::mat4* transform, const uint32_t instanceId, const uint32_t hitGroupId)
    : transform(glm::mat4(1.0f))
    , instanceId(instanceId)
    , hitGroupId(hitGroupId)
{
    memcpy(&this->transform, transform, sizeof(glm::mat4));
}

void AccStructureManager::destroy(Context* context)
{
	for (auto& bottomLevelAccStructure : bottomLevelAccStructures)
	{
		bottomLevelAccStructure.second->destroy(context);
		delete bottomLevelAccStructure.second;
	}
	bottomLevelAccStructures.clear();

	for (auto& accStructureInstance : accStructureInstances)
	{
		delete accStructureInstance.second;
	}
	accStructureInstances.clear();
}

void AccStructureManager::preBuildBottomLevelAccStructure(Context* context)
{
	for (auto& bottomLevelAccStructure : bottomLevelAccStructures)
	{
		auto object = bottomLevelAccStructure.first;
		auto blas = bottomLevelAccStructure.second;
		blas->registerGeometry(context, object->getVertexBuffer(), object->getIndexBuffer());
	}
}

void AccStructureManager::preBuildTopLevelAccStructure(Context* context)
{
	for (auto& bottomLevelAccStructure : bottomLevelAccStructures)
	{
		auto object = bottomLevelAccStructure.first;
		auto blas = bottomLevelAccStructure.second;

		auto& instances = object->getInstances();

		for (auto& instance : instances)
		{
			registerInstance(blas, &instance.second, 0, 0);
		}
	}

	for (auto& accStructureInstance : accStructureInstances)
	{
		auto blas = accStructureInstance.first;
		auto instance = accStructureInstance.second;

		buildAccInstance(context,
                         blas,
                         instance->getTransform(),
                         instance->getInstanceId(),
                         instance->getHitGroupId());
	}
}

BottomLevelAccStructure* AccStructureManager::registerModel(Context* context, model::Object* object)
{
	BottomLevelAccStructure* blas = context->createBottomLevelAccStructure();
	bottomLevelAccStructures.push_back(std::make_pair(object, blas));
	return blas;
}

void AccStructureManager::registerInstance(BottomLevelAccStructure* blas, glm::mat4* transform, const uint32_t instanceId, const uint32_t hitGroupId)
{
	accStructureInstances.push_back(std::make_pair(blas, new AccStructureInstance(transform, instanceId, hitGroupId)));
}
}
