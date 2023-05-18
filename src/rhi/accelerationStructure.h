#pragma once
#include <vector>
#include "platform/utils.h"
#include "rhi/resources.h"

namespace model
{
class Object;
}

namespace rhi
{
class Context;
class IndexBuffer;
class VertexBuffer;

class BottomLevelAccStructure
{
public:
	virtual ~BottomLevelAccStructure() = default;

	virtual void destroy(Context* context) = 0;

	virtual void registerGeometry(Context* context, VertexBuffer* vertexBuffer, IndexBuffer* indexBuffer) = 0;
};

class AccStructureInstance
{
public:
	AccStructureInstance(const glm::mat4* transform, const uint32_t instanceId, const uint32_t hitGroupId);

	const glm::mat4 getTransform() { return transform;}
	const uint32_t getInstanceId() { return instanceId;}
	const uint32_t getHitGroupId() { return hitGroupId;}
private:
	glm::mat4 transform;
	const uint32_t instanceId;
	const uint32_t hitGroupId;
};

class AccStructureManager : public Descriptor
{
public:
	virtual void destroy(Context* context);

	void preBuildBottomLevelAccStructure(Context* context);

	virtual void buildBottomLevelAccStructure(Context* context) = 0;

	void preBuildTopLevelAccStructure(Context* context);

	virtual void buildTopLevelAccStructure(Context* context) = 0;

	virtual void postBuild(Context* context) = 0;

	virtual void buildAccInstance(Context* context,
								BottomLevelAccStructure* blas,
								const glm::mat4& transform,
								const uint32_t instanceId,
								const uint32_t hitGroupId) = 0;

public:
	BottomLevelAccStructure* registerModel(Context* context, model::Object* object);

	void registerInstance(BottomLevelAccStructure* blas, glm::mat4* transform, const uint32_t instanceId, const uint32_t hitGroupId);
protected:
	std::vector<std::pair<model::Object*, BottomLevelAccStructure*>> bottomLevelAccStructures;
	std::vector<std::pair<BottomLevelAccStructure*, AccStructureInstance*>> accStructureInstances;
};
}
