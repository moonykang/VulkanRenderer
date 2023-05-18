#include "rhi/context.h"
#include "vulkan/buffer.h"
#include "vulkan/context.h"
#include "vulkan/resources.h"
#include "vulkan/transition.h"

namespace vk
{
	StorageBuffer::StorageBuffer(rhi::BufferType bufferType, VkBufferUsageFlags usageFlags)
		: buffer(nullptr)
		, bufferType(bufferType)
		, descriptorBufferInfo()
		, currentAccessFlags(VK_ACCESS_SHADER_READ_BIT) // TODO
		, currentPipelineFlags(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT) // TODO
		, usageFlags(usageFlags)
	{

	}

	void StorageBuffer::destroy(rhi::Context* context)
	{
		Context* contextVk = reinterpret_cast<Context*>(context);

		if (buffer != nullptr)
		{
			buffer->destroy(contextVk);
			delete buffer;
			buffer = nullptr;
		}
	}

	void StorageBuffer::build(rhi::Context* context)
	{
		ASSERT(memoryBuffer.size() != 0);
		Context* contextVk = reinterpret_cast<Context*>(context);

		buffer = BufferFactory::createBuffer(bufferType, usageFlags, 0, memoryBuffer.size());
		buffer->init(contextVk, memoryBuffer.data());
	}

	void StorageBuffer::update(rhi::Context* context)
	{
		ASSERT(buffer);
		Context* contextVk = reinterpret_cast<Context*>(context);

		buffer->mapMemory(contextVk, memoryBuffer.size(), memoryBuffer.data());
	}

	void StorageBuffer::bind(rhi::Context* context)
	{
		//context->getActiveCommandBuffer()->bindIndexBuffers(buffer->getBuffer());
	}

	void* StorageBuffer::getDescriptorData(rhi::DescriptorType type)
	{
		descriptorBufferInfo.buffer = getHandle();
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = getSize();

		return &descriptorBufferInfo;
	}

	VkBuffer StorageBuffer::getHandle() { return buffer->getBuffer(); }
	size_t StorageBuffer::getSize() { return memoryBuffer.size(); }

	Transition* StorageBuffer::getTransition(rhi::MemoryAccessFlags accessFlags)
	{
		VkAccessFlags srcAccessFlags =  currentAccessFlags;
		VkPipelineStageFlags srcPipelineStageFlags = currentPipelineFlags;

		VkAccessFlags dstAccessFlags = 0;
		VkPipelineStageFlags dstPipelineStageFlags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		if (accessFlags & rhi::MemoryAccess::Read)
		{
			if (accessFlags & rhi::MemoryAccess::Indirect)
			{
				dstAccessFlags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
				dstPipelineStageFlags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
			}
			else
			{
				dstAccessFlags |= VK_ACCESS_SHADER_READ_BIT;
			}
		}
		else if (accessFlags & rhi::MemoryAccess::Write)
		{
			dstAccessFlags |= VK_ACCESS_SHADER_WRITE_BIT;
			if (accessFlags & rhi::MemoryAccess::Indirect)
			{
				dstPipelineStageFlags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
			}
		}
		currentAccessFlags = dstAccessFlags;
		currentPipelineFlags = dstPipelineStageFlags;

		return new Transition(getHandle(), srcPipelineStageFlags, dstPipelineStageFlags, srcAccessFlags, dstAccessFlags);
	}
}