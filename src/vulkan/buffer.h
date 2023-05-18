#pragma once

#include "platform/memorybuffer.h"
#include "vulkan/context.h"
#include "vulkan/commandBuffer.h"
#include "vulkan/vk_wrapper.h"
#include "rhi/buffer.h"

namespace vk
{
class Context;
class Buffer
{
public:
	Buffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryProperty, const VkMemoryAllocateFlags allocateFlags, const size_t size);

	virtual ~Buffer() = default;

	virtual void init(Context* context, const void* data) = 0;

	void destroy(Context* context);

	void initBuffer(Context* context, const VkBufferUsageFlags usage);

	void initStagingBuffer(Context* context);

	void mapMemory(Context* context, size_t size, const void* mapData);

	virtual void* mapMemory(Context* context, size_t size);

	void unmapMemory(Context* context, size_t size);

	virtual void invalidateMemory(Context* context, size_t size = VK_WHOLE_SIZE) {}

	virtual void flushMemory(Context* context, size_t size = VK_WHOLE_SIZE) {}

	void mapStagingMemory(Context* context, size_t size, const void* mapData);

	void Copy(Context* context, VkBuffer srcBuffer, VkDeviceSize size);

	void Copy(Context* context, VkBuffer srcBuffer, VkDeviceSize offset, VkDeviceSize size);

	uint32_t getMemoryTypeIndex(VkPhysicalDevice physicalDevice, const uint32_t memoryTypeBits, const VkMemoryPropertyFlags memoryProperty);
public:
	VkBuffer getBuffer();

	VkDeviceMemory getDeviceMemory();

	VkDeviceSize getDeviceAddress(VkDevice device);
protected:
	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags memoryProperty;
	VkMemoryAllocateFlags allocateFlags;
	size_t size;

	handle::Buffer buffer;
	handle::Buffer stagingBuffer;
	handle::DeviceMemory memory;
	handle::DeviceMemory stagingMemory;
};

class DeviceLocalBuffer : public Buffer
{
public:
	DeviceLocalBuffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryProperty, const VkMemoryAllocateFlags allocateFlags, const size_t size);
	void init(Context* context, const void* data) override;
};

class HostSharedBuffer : public Buffer
{
public:
	HostSharedBuffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryProperty, const VkMemoryAllocateFlags allocateFlags, const size_t size);
	void init(Context* context, const void* data) override;
};

class HostCachedBuffer : public HostSharedBuffer
{
public:
	HostCachedBuffer(const VkBufferUsageFlags usage, const VkMemoryPropertyFlags memoryProperty, const VkMemoryAllocateFlags allocateFlags, const size_t size);
	void* mapMemory(Context* context, size_t size) override;
	void invalidateMemory(Context* context, size_t size = VK_WHOLE_SIZE) override;
	virtual void flushMemory(Context* context, size_t size = VK_WHOLE_SIZE) override;
};

class BufferFactory
{
public:
	static Buffer* createBuffer(rhi::BufferType bufferType, const VkBufferUsageFlags usage, const VkMemoryAllocateFlags allocateFlags, const size_t size);
};

class ScratchBuffer : public rhi::ScratchBuffer
{
public:
	ScratchBuffer(rhi::BufferUsageFlags bufferUsage);

	void destroy(rhi::Context* context) override;

	size_t preSuballocate(uint32_t size) override;

	void build(rhi::Context* context) override;

	virtual VkBuffer getHandle() { return buffer->getBuffer(); }

	void* getDescriptorData(rhi::DescriptorType type) override final;

	uint64_t getDeviceAddress(rhi::Context* rhiContext);

	vk::Buffer* getBuffer() { return buffer; }
private:
	vk::Buffer* buffer;
	size_t bufferSize;
	size_t offset;
	VkBufferUsageFlags bufferUsage;
	VkDescriptorBufferInfo descriptorBufferInfo;
};

class VertexBuffer : public rhi::VertexBuffer
{
public:
	VertexBuffer();

	void destroy(rhi::Context* context) override final;

	void build(rhi::Context* context) override;

	void bind(rhi::Context* context) override final;

	void suballocate(vk::Buffer* buffer, size_t offset);
public:
	void updateVertexInputState(VkPipelineVertexInputStateCreateInfo* vertexInputState);

	VkDeviceSize getDeviceAddress(VkDevice device);

	size_t getOffset();
private:
	bool built;
	bool suballocated; // TODO
	rhi::BufferType bufferType;
	vk::Buffer* buffer;
	vk::Buffer* stagingBuffer;
	size_t offset;
	VkVertexInputBindingDescription vertexInputBindingDescription;
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;	
};

class IndexBuffer : public rhi::IndexBuffer
{
public:
	IndexBuffer(rhi::IndexSize indexSize);

	void destroy(rhi::Context* context) override final;

	void build(rhi::Context* context) override;

	void bind(rhi::Context* context) override final;

	void suballocate(vk::Buffer* buffer, size_t offset);
public:
	VkDeviceSize getDeviceAddress(VkDevice device);

	size_t getOffset();

	VkIndexType getIndexType();
private:
	bool built;
	bool suballocated;
	vk::Buffer* buffer;
	vk::Buffer* stagingBuffer;
	size_t offset;
	VkIndexType indexType;	
};

class NullIndexBuffer : public rhi::IndexBuffer
{
public:
	void destroy(rhi::Context* context) {}

	void build(rhi::Context* context) {}

	void bind(rhi::Context* context) {}
};

class UniformBuffer : public rhi::UniformBuffer
{
public:
	UniformBuffer(rhi::BufferType bufferType);

	void destroy(rhi::Context* context) override final;

	void build(rhi::Context* context) override;

	void update(rhi::Context* context) override;

	void bind(rhi::Context* context) override final;

	void* getDescriptorData(rhi::DescriptorType type) override final;
private:
	virtual VkBuffer getHandle();

	virtual size_t getSize();
private:
	rhi::BufferType bufferType;
	vk::Buffer* buffer;
	VkDescriptorBufferInfo descriptorBufferInfo;
};

class StorageBuffer : public rhi::StorageBuffer
{
public:
	StorageBuffer(rhi::BufferType bufferType, VkBufferUsageFlags usageFlags);

	void destroy(rhi::Context* context) override final;

	void build(rhi::Context* context) override;

	void update(rhi::Context* context) override;

	void bind(rhi::Context* context) override final;

	void* getDescriptorData(rhi::DescriptorType type) override final;

	Transition* getTransition(rhi::MemoryAccessFlags accessFlags);

	VkBuffer getHandle();

	size_t getSize();
private:
	rhi::BufferType bufferType;
	vk::Buffer* buffer;
	VkDescriptorBufferInfo descriptorBufferInfo;
	VkAccessFlags currentAccessFlags;
	VkPipelineStageFlags currentPipelineFlags;
	VkBufferUsageFlags usageFlags;
};
}