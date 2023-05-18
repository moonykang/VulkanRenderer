#include "rhi/context.h"
#include "vulkan/buffer.h"
#include "vulkan/context.h"

namespace vk
{
UniformBuffer::UniformBuffer(rhi::BufferType bufferType)
	: buffer(nullptr)
	, bufferType(bufferType)
	, descriptorBufferInfo()
{

}

void UniformBuffer::destroy(rhi::Context* context)
{
	Context* contextVk = reinterpret_cast<Context*>(context);

	if (buffer != nullptr)
	{
		buffer->destroy(contextVk);
		delete buffer;
		buffer = nullptr;
	}
}

void UniformBuffer::build(rhi::Context* context)
{
	ASSERT(memoryBuffer.size() != 0);
	Context* contextVk = reinterpret_cast<Context*>(context);

	buffer = BufferFactory::createBuffer(bufferType, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 0, memoryBuffer.size());
	buffer->init(contextVk, memoryBuffer.data());
}

void UniformBuffer::update(rhi::Context* context)
{
	ASSERT(buffer);
	Context* contextVk = reinterpret_cast<Context*>(context);

	buffer->mapMemory(contextVk, memoryBuffer.size(), memoryBuffer.data());
}

void UniformBuffer::bind(rhi::Context* context)
{
	//context->getActiveCommandBuffer()->bindIndexBuffers(buffer->getBuffer());
}

void* UniformBuffer::getDescriptorData(rhi::DescriptorType type)
{
	descriptorBufferInfo.buffer = getHandle();
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = getSize();

	return &descriptorBufferInfo;
}

VkBuffer UniformBuffer::getHandle() { return buffer->getBuffer(); }
size_t UniformBuffer::getSize() { return memoryBuffer.size(); }
}