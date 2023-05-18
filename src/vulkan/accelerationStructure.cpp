#include "vulkan/accelerationStructure.h"
#include "vulkan/buffer.h"
#include "vulkan/context.h"

namespace vk
{
AccStructureManager::AccStructureManager()
	: topLevelAccStructure(nullptr)
	, blasScratchBuffer(nullptr)
	, blasBuffer(nullptr)
	, instanceBuffer(nullptr)
	, tlasScratchBuffer(nullptr)
	, tlasBuffer(nullptr)
	, writeDescriptorSetAccelerationStructure()
{

}

void AccStructureManager::destroy(rhi::Context* rhiContext)
{
	rhi::AccStructureManager::destroy(rhiContext);

	Context* context = reinterpret_cast<Context*>(rhiContext);

#define DESTROY_OBJ(obj) if (obj != nullptr) obj->destroy(context); delete obj; obj = nullptr;
	DESTROY_OBJ(topLevelAccStructure);
	DESTROY_OBJ(blasBuffer);
	DESTROY_OBJ(blasScratchBuffer);
	DESTROY_OBJ(instanceBuffer);
	DESTROY_OBJ(tlasScratchBuffer);
	DESTROY_OBJ(tlasBuffer);
#undef DESTROY_OBJ
}

void AccStructureManager::buildBottomLevelAccStructure(rhi::Context* rhiContext)
{
	if (bottomLevelAccStructures.empty())
	{
		LOGD("Empty BLAS");
		return;
	}

	// Bottom level acceleration structure
	Context* context = reinterpret_cast<Context*>(rhiContext);

	size_t blasScratchBufferSize = 0;
	size_t blasSize = 0;

	for (auto& bottomLevelAccStructure : bottomLevelAccStructures)
	{
		BottomLevelAccStructure* blas = reinterpret_cast<BottomLevelAccStructure*>(bottomLevelAccStructure.second);
		blas->preBuild(context);
		
		blasScratchBufferSize += blas->getScratchSize();
		blasSize += blas->getAccStructureSize();
	}

	VkBufferUsageFlags scratchBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	blasScratchBuffer = BufferFactory::createBuffer(rhi::BufferType::DeviceLocal, scratchBufferUsage, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, blasScratchBufferSize); // memprop
	blasScratchBuffer->initBuffer(context, scratchBufferUsage);

	VkBufferUsageFlags accBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
	blasBuffer = BufferFactory::createBuffer(rhi::BufferType::DeviceLocal, accBufferUsage, 0, blasSize); // memprop
	blasBuffer->initBuffer(context, accBufferUsage); // VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT

	size_t blasScratchOffset = 0;
	size_t blasOffset = 0;

	for (auto& bottomLevelAccStructure : bottomLevelAccStructures)
	{
		BottomLevelAccStructure* blas = reinterpret_cast<BottomLevelAccStructure*>(bottomLevelAccStructure.second);
		blas->build(context, blasScratchBuffer, blasScratchOffset, blasBuffer, blasOffset);
		blasScratchOffset += blas->getScratchSize();
		blasOffset += blas->getAccStructureSize();
	}
}

void AccStructureManager::buildTopLevelAccStructure(rhi::Context* rhiContext)
{
	if (accStructureInstances.empty())
	{
		LOGD("Empty Instances");
		return;
	}

	Context* context = reinterpret_cast<Context*>(rhiContext);

	VkBufferUsageFlags instanceBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	instanceBuffer = BufferFactory::createBuffer(rhi::BufferType::DeviceLocal, instanceBufferUsage, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, sizeof(VkAccelerationStructureInstanceKHR) * accStructureInstances.size()); // memprop
	instanceBuffer->init(context, accStructureInstances.data());

	topLevelAccStructure = new TopLevelAccStructure();
	topLevelAccStructure->prebuild(context, instanceBuffer->getDeviceAddress(context->getDevice()), accStructureInstances.size());

	VkBufferUsageFlags scratchBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	tlasScratchBuffer = BufferFactory::createBuffer(rhi::BufferType::DeviceLocal, scratchBufferUsage, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, topLevelAccStructure->getScratchSize()); // memprop
	tlasScratchBuffer->initBuffer(context, scratchBufferUsage);

	VkBufferUsageFlags accBufferUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
	tlasBuffer = BufferFactory::createBuffer(rhi::BufferType::DeviceLocal, accBufferUsage, 0, topLevelAccStructure->getAccStructureSize()); // memprop
	tlasBuffer->initBuffer(context, accBufferUsage);

	topLevelAccStructure->build(context, tlasScratchBuffer, tlasBuffer);
}

void AccStructureManager::postBuild(rhi::Context* rhiContext)
{
	Context* context = reinterpret_cast<Context*>(rhiContext);

	context->wait();
	tlasScratchBuffer->destroy(context);
	delete tlasScratchBuffer;
	tlasScratchBuffer = nullptr;

	blasScratchBuffer->destroy(context);
	delete blasScratchBuffer;
	blasScratchBuffer = nullptr;
}

void AccStructureManager::buildAccInstance(rhi::Context* rhiContext,
										   rhi::BottomLevelAccStructure* rhiBlas,
										   const glm::mat4& transform,
										   const uint32_t instanceId,
										   const uint32_t hitGroupId)
{
	vk::Context* context = reinterpret_cast<vk::Context*>(rhiContext);

	BottomLevelAccStructure* blas = reinterpret_cast<BottomLevelAccStructure*>(rhiBlas);
	VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {};
	addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addressInfo.accelerationStructure = blas->getHandle();

	const VkDeviceAddress address = vkGetAccelerationStructureDeviceAddressKHR(context->getDevice(), &addressInfo);

	auto& instance = accStructureInstances.emplace_back();
	instance.instanceCustomIndex = instanceId;
	instance.mask = 0xFF;
	instance.instanceShaderBindingTableRecordOffset = hitGroupId;
	instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	instance.accelerationStructureReference = address;
	std::memcpy(&instance.transform, &transform, sizeof(instance.transform));
}

void* AccStructureManager::getDescriptorData(rhi::DescriptorType type)
{
	writeDescriptorSetAccelerationStructure.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	writeDescriptorSetAccelerationStructure.accelerationStructureCount = 1;
	writeDescriptorSetAccelerationStructure.pAccelerationStructures = &getTLAS()->getHandle();

	return &writeDescriptorSetAccelerationStructure;
}

AccStructure::AccStructure()
	: accStructure(VK_NULL_HANDLE)
	, accelerationStructureBuildGeometryInfo()
	, accelerationStructureBuildSizesInfo()
{

}

void AccStructure::updateAccStructureBuildSizes(Context* context)
{
	accelerationStructureBuildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	vkGetAccelerationStructureBuildSizesKHR(
		context->getDevice(),
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&accelerationStructureBuildGeometryInfo,
		maxPrimitiveCounts.data(),
		&accelerationStructureBuildSizesInfo);

	const uint64_t AccelerationStructureAlignment = 256; // official Vulkan specs
	const uint64_t ScratchAlignment = context->getAccelerationStructureProperties().minAccelerationStructureScratchOffsetAlignment;
	
	accelerationStructureBuildSizesInfo.accelerationStructureSize = 
		Util::align(accelerationStructureBuildSizesInfo.accelerationStructureSize, AccelerationStructureAlignment);
	accelerationStructureBuildSizesInfo.buildScratchSize = 
		Util::align(accelerationStructureBuildSizesInfo.buildScratchSize, ScratchAlignment);
}

void AccStructure::createAccStructure(Context* context, Buffer* buffer, VkDeviceSize resultOffset)
{
	ASSERT(buffer);

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = {};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.pNext = nullptr;
	accelerationStructureCreateInfo.type = accelerationStructureBuildGeometryInfo.type;
	accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.buffer = buffer->getBuffer();
	accelerationStructureCreateInfo.offset = resultOffset;

	VKCALL(vkCreateAccelerationStructureKHR(context->getDevice(), &accelerationStructureCreateInfo, nullptr, &accStructure));
}

BottomLevelAccStructure::BottomLevelAccStructure()
	: AccStructure()
{

}

void BottomLevelAccStructure::destroy(rhi::Context* rhiContext)
{
	vk::Context* context = reinterpret_cast<vk::Context*>(rhiContext);

	if (accStructure != VK_NULL_HANDLE)
	{
		vkDestroyAccelerationStructureKHR(context->getDevice(), accStructure, nullptr);
		accStructure = VK_NULL_HANDLE;
	}
}

void BottomLevelAccStructure::registerGeometry(rhi::Context* rhiContext, rhi::VertexBuffer* rhiVertexBuffer, rhi::IndexBuffer* rhiIndexBuffer)
{
	vk::Context* context = reinterpret_cast<vk::Context*>(rhiContext);
	vk::VertexBuffer* vertexBuffer = reinterpret_cast<vk::VertexBuffer*>(rhiVertexBuffer);
	vk::IndexBuffer* indexBuffer = reinterpret_cast<vk::IndexBuffer*>(rhiIndexBuffer);

	VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
	VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};

	vertexBufferDeviceAddress.deviceAddress = vertexBuffer->getDeviceAddress(context->getDevice()) + vertexBuffer->getOffset();
	indexBufferDeviceAddress.deviceAddress = indexBuffer->getDeviceAddress(context->getDevice()) + indexBuffer->getOffset();

	auto& accStructureGeometry = accStructureGeometries.emplace_back();
	accStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accStructureGeometry.pNext = nullptr;
	accStructureGeometry.flags = 0;
	accStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	accStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	accStructureGeometry.geometry.triangles.pNext = nullptr;
	accStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT; // TODO
	accStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
	accStructureGeometry.geometry.triangles.maxVertex = vertexBuffer->size();
	accStructureGeometry.geometry.triangles.vertexStride = vertexBuffer->unitSize();

	accStructureGeometry.geometry.triangles.indexType = indexBuffer->getIndexType();
	accStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
	accStructureGeometry.geometry.triangles.transformData = {};

	auto& geometryOffsetInfo = geometryOffsetInfos.emplace_back();
	geometryOffsetInfo.firstVertex = 0;
	geometryOffsetInfo.primitiveOffset = 0;
	geometryOffsetInfo.primitiveCount = static_cast<uint32_t>(indexBuffer->size() / 3);
	geometryOffsetInfo.transformOffset = 0;

	maxPrimitiveCounts.push_back(geometryOffsetInfo.primitiveCount);
}

void BottomLevelAccStructure::preBuild(Context* context)
{
	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = static_cast<uint32_t>(accStructureGeometries.size());
	accelerationStructureBuildGeometryInfo.pGeometries = accStructureGeometries.data();

	updateAccStructureBuildSizes(context);
}

void BottomLevelAccStructure::build(Context* context, Buffer* scratchBuffer, size_t scratchOffset, Buffer* buffer, size_t offset)
{
	createAccStructure(context, buffer, offset);

	accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationStructureBuildGeometryInfo.dstAccelerationStructure = accStructure;
	accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress(context->getDevice()) + offset;

	const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = geometryOffsetInfos.data();

	CommandBuffer* commandBuffer = context->getUploadCommandBuffer();
	commandBuffer->buildAccelerationStructures(1, &accelerationStructureBuildGeometryInfo, &pBuildOffsetInfo);
}

TopLevelAccStructure::TopLevelAccStructure()
	: AccStructure()
	, accStructureGeometryInstancesData()
	, accStructureGeometry()
	, instancesCount(0)
{
}

void TopLevelAccStructure::destroy(Context* context)
{
	if (accStructure != VK_NULL_HANDLE)
	{
		vkDestroyAccelerationStructureKHR(context->getDevice(), accStructure, nullptr);
	}	
}

void TopLevelAccStructure::prebuild(Context* context, const VkDeviceAddress instanceAddress, const uint32_t instancesCount)
{
	// Create VkAccelerationStructureGeometryInstancesDataKHR. This wraps a device pointer to the above uploaded instances.
	accStructureGeometryInstancesData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	accStructureGeometryInstancesData.arrayOfPointers = VK_FALSE;
	accStructureGeometryInstancesData.data.deviceAddress = instanceAddress;

	// Put the above into a VkAccelerationStructureGeometryKHR. We need to put the
	// instances struct in a union and label it as instance data.
	accStructureGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	accStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	accStructureGeometry.geometry.instances = accStructureGeometryInstancesData;

	accelerationStructureBuildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	accelerationStructureBuildGeometryInfo.geometryCount = 1;
	accelerationStructureBuildGeometryInfo.pGeometries = &accStructureGeometry;
	accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
	accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	accelerationStructureBuildGeometryInfo.srcAccelerationStructure = nullptr;

	maxPrimitiveCounts.push_back(instancesCount);
	updateAccStructureBuildSizes(context);
	this->instancesCount = instancesCount;
}

void TopLevelAccStructure::build(Context* context, Buffer* scratchBuffer, Buffer* buffer)
{
	// Create the acceleration structure.
	createAccStructure(context, buffer, 0);

	// Build the actual bottom-level acceleration structure
	VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo = {};
	buildOffsetInfo.primitiveCount = instancesCount;

	const VkAccelerationStructureBuildRangeInfoKHR* pBuildOffsetInfo = &buildOffsetInfo;

	accelerationStructureBuildGeometryInfo.dstAccelerationStructure = accStructure;
	accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer->getDeviceAddress(context->getDevice());

	CommandBuffer* commandBuffer = context->getUploadCommandBuffer();
	commandBuffer->buildAccelerationStructures(1, &accelerationStructureBuildGeometryInfo, &pBuildOffsetInfo);
}
}