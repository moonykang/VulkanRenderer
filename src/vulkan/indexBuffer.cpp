#include "rhi/context.h"
#include "vulkan/context.h"
#include "vulkan/buffer.h"

namespace vk
{
IndexBuffer::IndexBuffer(rhi::IndexSize indexSize)
	: buffer(nullptr)
	, stagingBuffer(nullptr)
	, suballocated(false)
	, offset(0)
	, built(false)
{
	switch (indexSize)
	{
	case rhi::IndexSize::UINT16:
		indexType = VK_INDEX_TYPE_UINT16;
		break;
	case rhi::IndexSize::UINT32:
		indexType = VK_INDEX_TYPE_UINT32;
		break;
	default:
		UNREACHABLE();
	}
}

void IndexBuffer::destroy(rhi::Context* context)
{
	Context* contextVk = reinterpret_cast<Context*>(context);
	if (!suballocated && buffer != nullptr)
	{
		buffer->destroy(contextVk);
		delete buffer;
		buffer = nullptr;
	}

	if (stagingBuffer != nullptr)
	{
		stagingBuffer->destroy(contextVk);
		delete stagingBuffer;
		stagingBuffer = nullptr;
	}

	rhi::IndexBuffer::destroy(context);
}

void IndexBuffer::build(rhi::Context* rhiContext)
{
	if (built)
	{
		return;
	}
	ASSERT(indices.size() != 0);
	Context* context = reinterpret_cast<Context*>(rhiContext);

	size_t bufferSize = indices.size() * sizeof(uint32_t);

	if (subAllocateInfo)
	{
		suballocated = true;

		ScratchBuffer* scratchBuffer = reinterpret_cast<ScratchBuffer*>(subAllocateInfo->buffer);
		buffer = scratchBuffer->getBuffer();
		stagingBuffer = BufferFactory::createBuffer(rhi::BufferType::HostCoherent, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, bufferSize);
		stagingBuffer->init(context, indices.data());
		buffer->Copy(context, stagingBuffer->getBuffer(), subAllocateInfo->offset, bufferSize);
	}
	else
	{
		buffer = BufferFactory::createBuffer(rhi::BufferType::DeviceLocal, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 0, bufferSize);
		buffer->init(context, indices.data());
	}

	built = true;
}

void IndexBuffer::bind(rhi::Context* context)
{
	Context* contextVk = reinterpret_cast<Context*>(context);
	contextVk->getActiveCommandBuffer()->bindIndexBuffers(buffer->getBuffer(), offset, indexType);
}

void IndexBuffer::suballocate(vk::Buffer* scratchBuffer, size_t allocatedOffset)
{
	ASSERT(scratchBuffer);
	buffer = scratchBuffer;
	offset = allocatedOffset;
	suballocated = true;
}

VkDeviceSize IndexBuffer::getDeviceAddress(VkDevice device)
{
	ASSERT(buffer);
	return buffer->getDeviceAddress(device);
}

size_t IndexBuffer::getOffset()
{
	return offset;
}

VkIndexType IndexBuffer::getIndexType()
{
	return indexType;
}
}