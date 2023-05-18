#pragma once

#include <vector>
#include "rhi/accelerationStructure.h"
#include "vulkan/vk_wrapper.h"

namespace vk
{
class Buffer;
class Context;

class AccStructure
{
public:
	AccStructure();

	~AccStructure() = default;

	VkAccelerationStructureKHR& getHandle() { return accStructure; }

	size_t getScratchSize()
	{
		return accelerationStructureBuildSizesInfo.buildScratchSize;
	}

	size_t getAccStructureSize()
	{
		return accelerationStructureBuildSizesInfo.accelerationStructureSize;
	}
protected:
	void updateAccStructureBuildSizes(Context* context);

	void createAccStructure(Context* context, Buffer* buffer, VkDeviceSize resultOffset);

	VkAccelerationStructureBuildSizesInfoKHR getAccStructureBuildSize()
	{
		return accelerationStructureBuildSizesInfo;
	}
protected:
	VkAccelerationStructureKHR accStructure;
	VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo;
	VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo;
	std::vector<uint32_t> maxPrimitiveCounts;
};

class BottomLevelAccStructure : public rhi::BottomLevelAccStructure, public AccStructure
{
public:
	BottomLevelAccStructure();

	void destroy(rhi::Context* context) override;

	void registerGeometry(rhi::Context* context, rhi::VertexBuffer* vertexBuffer, rhi::IndexBuffer* indexBuffer) override;

	void preBuild(Context* context);

	void build(Context* context, Buffer* scratchBuffer, size_t scratchOffset, Buffer* buffer, size_t offset);
private:
	uint64_t deviceAddress = 0;
	std::vector<VkAccelerationStructureGeometryKHR> accStructureGeometries;
	std::vector<VkAccelerationStructureBuildRangeInfoKHR> geometryOffsetInfos;
};

class TopLevelAccStructure : public AccStructure
{
public:
	TopLevelAccStructure();

	void destroy(Context* context);

	void prebuild(Context* context, const VkDeviceAddress instanceAddress, const uint32_t instancesCount);

	void build(Context* context, Buffer* scratchBuffer, Buffer* buffer);
private:
	uint32_t instancesCount;
	VkAccelerationStructureGeometryInstancesDataKHR accStructureGeometryInstancesData;
	VkAccelerationStructureGeometryKHR accStructureGeometry;
};

class AccStructureManager : public rhi::AccStructureManager
{
public:
	AccStructureManager();

	void destroy(rhi::Context* context) override;

	void buildBottomLevelAccStructure(rhi::Context* context) override;

	void buildTopLevelAccStructure(rhi::Context* context) override;

	void postBuild(rhi::Context* context) override;

	void buildAccInstance(rhi::Context* context,
						  rhi::BottomLevelAccStructure* blas,
						  const glm::mat4& transform,
						  const uint32_t instanceId,
						  const uint32_t hitGroupId) override;

	void* getDescriptorData(rhi::DescriptorType type) override final;
private:
	virtual TopLevelAccStructure* getTLAS() { return topLevelAccStructure; }

private:
	TopLevelAccStructure* topLevelAccStructure;
	Buffer* blasScratchBuffer;
	Buffer* blasBuffer;
	Buffer* instanceBuffer;
	Buffer* tlasScratchBuffer;
	Buffer* tlasBuffer;
	std::vector<VkAccelerationStructureInstanceKHR> accStructureInstances;
	VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructure;
};
}