#include "rhi/context.h"
#include "vulkan/buffer.h"
#include "vulkan/resources.h"

namespace vk
{
VertexBuffer::VertexBuffer()
	: buffer(nullptr)
	, stagingBuffer(nullptr)
	, vertexInputBindingDescription()
	, offset(0)
	, suballocated(false)
	, built(false)
{

}

void VertexBuffer::destroy(rhi::Context* context)
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

	rhi::VertexBuffer::destroy(context);
}

void VertexBuffer::build(rhi::Context* rhiContext)
{
	if (built)
	{
		return;
	}

	ASSERT(vertices.size() != 0);
	Context* context = reinterpret_cast<Context*>(rhiContext);

	vertexInputBindingDescription.binding = vertexBindingDescription.binding;
	vertexInputBindingDescription.stride = vertexBindingDescription.stride;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	for (auto& vertexDescription : vertexDescriptions)
	{
		vertexInputAttributeDescriptions.push_back({
			vertexDescription.binding,
			vertexDescription.location,
			convertToVkFormat(vertexDescription.format),
			vertexDescription.offset });
	}

	size_t bufferSize = vertices.size() * sizeof(rhi::VertexData);

	if (subAllocateInfo)
	{
		suballocated = true;

		ScratchBuffer* scratchBuffer = reinterpret_cast<ScratchBuffer*>(subAllocateInfo->buffer);
		buffer = scratchBuffer->getBuffer();
		stagingBuffer = BufferFactory::createBuffer(rhi::BufferType::HostCoherent, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, 0, bufferSize);
		stagingBuffer->init(context, vertices.data());
		buffer->Copy(context, stagingBuffer->getBuffer(), subAllocateInfo->offset, bufferSize);
	}
	else
	{
		buffer = BufferFactory::createBuffer(rhi::BufferType::DeviceLocal, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0, bufferSize);
		buffer->init(context, vertices.data());
	}

	built = true;
}

void VertexBuffer::bind(rhi::Context* context)
{
	Context* contextVk = reinterpret_cast<Context*>(context);
	contextVk->getActiveCommandBuffer()->bindVertexBuffers(buffer->getBuffer(), offset);
}

void VertexBuffer::suballocate(vk::Buffer* scratchBuffer, size_t allocatedOffset)
{
	ASSERT(scratchBuffer);
	buffer = scratchBuffer;
	offset = allocatedOffset;
	suballocated = true;
}

void VertexBuffer::updateVertexInputState(VkPipelineVertexInputStateCreateInfo* vertexInputState)
{
	vertexInputState->vertexBindingDescriptionCount = 1;
	vertexInputState->pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputState->vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size());
	vertexInputState->pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
}

VkDeviceSize VertexBuffer::getDeviceAddress(VkDevice device)
{
	ASSERT(buffer);
	return buffer->getDeviceAddress(device);
}

size_t VertexBuffer::getOffset()
{
	return offset;
}
}