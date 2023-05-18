#include "vulkan//buffer.h"
#include "vulkan/resources.h"

namespace vk
{
const VkFlags kStagingBufferUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
const VkFlags kHostVisibleMemoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
const VkFlags kHostCachedMemoryProperty = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;

Buffer* BufferFactory::createBuffer(rhi::BufferType bufferType, const VkBufferUsageFlags usage, const VkMemoryAllocateFlags allocateFlags, const size_t size)
{
	switch (bufferType)
	{
	case rhi::BufferType::DeviceLocal:
		return new DeviceLocalBuffer(usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, allocateFlags, size);
	case rhi::BufferType::HostCached:
		return new HostCachedBuffer(usage, kHostCachedMemoryProperty, allocateFlags, size);
	case rhi::BufferType::HostCoherent:
		return new HostSharedBuffer(usage, kHostVisibleMemoryProperty, allocateFlags, size);
	default:
		UNREACHABLE();
		return nullptr;
	}
}

Buffer::Buffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryProperty, const VkMemoryAllocateFlags allocateFlags, const size_t size)
	: usage(usage)
	, memoryProperty(memoryProperty)
	, allocateFlags(allocateFlags)
	, size(size)	
{
}

void Buffer::destroy(Context* context)
{
	buffer.destroy(context->getDevice());
	memory.destroy(context->getDevice());

	// TODO:
	stagingBuffer.destroy(context->getDevice());
	stagingMemory.destroy(context->getDevice());
}

void Buffer::initBuffer(Context* context, const VkBufferUsageFlags usage)
{
	uint32_t queueFamilyIndices[] = { context->getQueueFamilyIndex() };

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.size = static_cast<VkDeviceSize>(size);
	bufferCreateInfo.usage = usage;
		
	buffer.init(context->getDevice(), bufferCreateInfo);

	VkMemoryRequirements memRequirements;
	buffer.getMemoryRequirements(context->getDevice(), &memRequirements);

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo = {};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = allocateFlags;

	VkMemoryPropertyFlags allocatedMemoryProperty = memoryProperty;

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = &memoryAllocateFlagsInfo;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(context->getPhysicalDevice(), memRequirements.memoryTypeBits, memoryProperty);

	VKCALL(memory.allocate(context->getDevice(), memoryAllocateInfo));
	VKCALL(buffer.bindMemory(context->getDevice(), memory));
}

void Buffer::initStagingBuffer(Context* context)
{
	uint32_t queueFamilyIndices[] = { context->getQueueFamilyIndex() };

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.pNext = nullptr;
	bufferCreateInfo.queueFamilyIndexCount = 1;
	bufferCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferCreateInfo.size = static_cast<VkDeviceSize>(size);
	bufferCreateInfo.usage = kStagingBufferUsage;

	stagingBuffer.init(context->getDevice(), bufferCreateInfo);

	VkMemoryRequirements memRequirements;
	stagingBuffer.getMemoryRequirements(context->getDevice(), &memRequirements);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = getMemoryTypeIndex(context->getPhysicalDevice(), memRequirements.memoryTypeBits, kHostVisibleMemoryProperty);

	VKCALL(stagingMemory.allocate(context->getDevice(), memoryAllocateInfo));
	VKCALL(stagingBuffer.bindMemory(context->getDevice(), stagingMemory));
}

void* Buffer::mapMemory(Context* context, size_t size)
{
	uint8_t* data = nullptr;
	VKCALL(memory.map(context->getDevice(), 0, size, 0, &data));

	return data;
}

void Buffer::mapMemory(Context* context, size_t size, const void* mapData)
{
	invalidateMemory(context, size);

	uint8_t* data = nullptr;
	VKCALL(memory.map(context->getDevice(), 0, size, 0, &data));
	memcpy(data, mapData, size);
	memory.unmap(context->getDevice());	

	flushMemory(context);
}

void Buffer::unmapMemory(Context* context, size_t size)
{
	memory.unmap(context->getDevice());

	flushMemory(context);
}

void Buffer::mapStagingMemory(Context* context, size_t size, const void* mapData)
{
	uint8_t* data = nullptr;
	VKCALL(stagingMemory.map(context->getDevice(), 0, size, 0, &data));
	memcpy(data, mapData, size);
	stagingMemory.unmap(context->getDevice());
}

void Buffer::Copy(Context* context, VkBuffer srcBuffer, VkDeviceSize offset, VkDeviceSize size)
{
	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = offset;
	copyRegion.size = size;

	CommandBuffer* commandBuffer = context->getUploadCommandBuffer();
	commandBuffer->copyBuffer(srcBuffer, buffer.getHandle(), copyRegion);
	context->submitUploadCommandBuffer();
}

void Buffer::Copy(Context* context, VkBuffer srcBuffer, VkDeviceSize size)
{
	Copy(context, srcBuffer, 0, size);
}

uint32_t Buffer::getMemoryTypeIndex(VkPhysicalDevice physicalDevice, const uint32_t memoryTypeBits, const VkMemoryPropertyFlags memoryProperty)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & memoryProperty) == memoryProperty)
		{
			return i;
		}
	}

	UNREACHABLE();
	return 0;
}

VkBuffer Buffer::getBuffer()
{
	return buffer.getHandle();
}

VkDeviceMemory Buffer::getDeviceMemory()
{
	return memory.getHandle();
}

VkDeviceSize Buffer::getDeviceAddress(VkDevice device)
{
	VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{};
	bufferDeviceAddressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAddressInfo.buffer = buffer.getHandle();
	return vkGetBufferDeviceAddressKHR(device, &bufferDeviceAddressInfo);
}

ScratchBuffer::ScratchBuffer(rhi::BufferUsageFlags rhiBufferUsage)
	: buffer(nullptr)
	, bufferSize(0)
	, offset(0)
	, bufferUsage(0)
	, descriptorBufferInfo()
{
	bufferUsage = convertToVkBufferUsageFlag(rhiBufferUsage);
}

void ScratchBuffer::destroy(rhi::Context* rhiContext)
{
	Context* context = reinterpret_cast<Context*>(rhiContext);
	if (buffer != nullptr)
	{
		buffer->destroy(context);
		delete buffer;
		buffer = nullptr;
	}
}

size_t ScratchBuffer::preSuballocate(uint32_t size)
{
	size_t offset = bufferSize;
	bufferSize += size;
	return offset;
}

void ScratchBuffer::build(rhi::Context* rhiContext)
{
	Context* context = reinterpret_cast<Context*>(rhiContext);

	buffer = BufferFactory::createBuffer(rhi::BufferType::DeviceLocal, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, bufferSize);
	buffer->initBuffer(context, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
}

void* ScratchBuffer::getDescriptorData(rhi::DescriptorType type)
{
	descriptorBufferInfo.buffer = getHandle();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = VK_WHOLE_SIZE;

	return &descriptorBufferInfo;
}

uint64_t ScratchBuffer::getDeviceAddress(rhi::Context* rhiContext)
{
	ASSERT(buffer);
	Context* context = reinterpret_cast<Context*>(rhiContext);
	return static_cast<uint64_t>(buffer->getDeviceAddress(context->getDevice()));
}

DeviceLocalBuffer::DeviceLocalBuffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryProperty, const VkMemoryAllocateFlags allocateFlags, const size_t size)
	: Buffer(usage, memoryProperty, allocateFlags, size)
{

}

void DeviceLocalBuffer::init(Context* context, const void* data)
{
	uint32_t queueFamilyIndices[] = { context->getQueueFamilyIndex() };

	VkDeviceSize bufferSize = static_cast<VkDeviceSize>(size);

	initBuffer(context, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	initStagingBuffer(context);
	mapStagingMemory(context, size, data);

	Copy(context, stagingBuffer.getHandle(), bufferSize);
}

HostSharedBuffer::HostSharedBuffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryProperty, const VkMemoryAllocateFlags allocateFlags, const size_t size)
	: Buffer(usage, memoryProperty, allocateFlags, size)
{

}

void HostSharedBuffer::init(Context* context, const void* data)
{
	uint32_t queueFamilyIndices[] = { context->getQueueFamilyIndex() };

	initBuffer(context, usage);

	mapMemory(context, size, data);
}

HostCachedBuffer::HostCachedBuffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryProperty, const VkMemoryAllocateFlags allocateFlags, const size_t size)
	: HostSharedBuffer(usage, memoryProperty, allocateFlags, size)
{

}

void* HostCachedBuffer::mapMemory(Context* context, size_t size)
{
	VkMappedMemoryRange range = {};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.pNext = nullptr;
	range.offset = 0;
	range.size = size;
	range.memory = memory.getHandle();
	memory.invalidate(context->getDevice(), range);

	uint8_t* data = nullptr;
	VKCALL(memory.map(context->getDevice(), 0, size, 0, &data));

	return data;
}

void HostCachedBuffer::invalidateMemory(Context* context, size_t size)
{
	VkMappedMemoryRange range = {};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.pNext = nullptr;
	range.offset = 0;
	range.size = size;
	range.memory = memory.getHandle();
	memory.invalidate(context->getDevice(), range);
}

void HostCachedBuffer::flushMemory(Context* context, VkDeviceSize size)
{
	VkMappedMemoryRange range = {};
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.pNext = nullptr;
	range.offset = 0;
	range.size = size;
	range.memory = memory.getHandle();
	memory.flush(context->getDevice(), range);
}

}