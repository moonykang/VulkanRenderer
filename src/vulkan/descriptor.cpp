#include "rhi/context.h"
#include "vulkan/descriptor.h"
#include "vulkan/resources.h"
#include "vulkan/texture.h"
#include "vulkan/buffer.h"
#include "vulkan/pipeline.h"
#include "vulkan/commandBuffer.h"
#include "vulkan/accelerationStructure.h"

namespace vk
{

void DescriptorSet::destroy(rhi::Context* rhiContext)
{
	Context* context = reinterpret_cast<Context*>(rhiContext);

	descriptorSetLayout.destroy(context->getDevice());
}

void DescriptorSet::build(rhi::Context* rhiContext)
{
	Context* context = reinterpret_cast<Context*>(rhiContext);

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;
	size_t bufferInfoSize = 0;
	size_t imageInfoSize = 0;

	uint32_t binding = 0;
	for (auto& descriptor : descriptors)
	{
		descriptorSetLayoutBindings.push_back({
			binding++,
			convertToVkDescriptorType(descriptor.getType()),
			1,
			convertToVkShaderStageFlag(descriptor.getStage()), nullptr });
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	VKCALL(descriptorSetLayout.init(context->getDevice(), descriptorSetLayoutCreateInfo));

	const uint32_t descriptorSetCount = 1;
	VkDescriptorSetLayout descriptorSetLayouts[descriptorSetCount] = { descriptorSetLayout.getHandle() };
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorSetCount = descriptorSetCount;
	descriptorSetAllocateInfo.pSetLayouts = descriptorSetLayouts;

	descriptorSet = context->getDescriptorPool()->allocate(context->getDevice(), descriptorSetAllocateInfo);
	writeDescriptorSets.reserve(descriptors.size());

	for (auto& descriptor : descriptors)
	{
		updateWriteDescriptorSet(descriptor);
		auto& writeDS = writeDescriptorSets.back();
	}

	vkUpdateDescriptorSets(context->getDevice(), static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
}

void DescriptorSet::bind(rhi::Context* rhiContext, rhi::GraphicsPipeline* rhiPipeline, uint32_t binding)
{
	Context* contextVk = reinterpret_cast<Context*>(rhiContext);
	GraphicsPipeline* pipeline = reinterpret_cast<GraphicsPipeline*>(rhiPipeline);

	CommandBuffer* commandBuffer = contextVk->getActiveCommandBuffer();

	VkPipelineBindPoint pipelineBindPoint = pipeline->getBindPoint();
	VkPipelineLayout pipelineLayout = pipeline->getLayout();
	commandBuffer->bindDescriptorSets(
		pipelineBindPoint, pipelineLayout, binding, 1, &descriptorSet, 0, nullptr);
}

void DescriptorSet::bind(rhi::Context* rhiContext, rhi::ComputePipeline* rhiPipeline, uint32_t binding)
{
	Context* contextVk = reinterpret_cast<Context*>(rhiContext);
	ComputePipeline* pipeline = reinterpret_cast<ComputePipeline*>(rhiPipeline);

	CommandBuffer* commandBuffer = contextVk->getActiveCommandBuffer();

	VkPipelineBindPoint pipelineBindPoint = pipeline->getBindPoint();
	VkPipelineLayout pipelineLayout = pipeline->getLayout();
	commandBuffer->bindDescriptorSets(
		pipelineBindPoint, pipelineLayout, binding, 1, &descriptorSet, 0, nullptr);
}

void DescriptorSet::bind(rhi::Context* rhiContext, rhi::RayTracingPipeline* rhiPipeline, uint32_t binding)
{
	Context* contextVk = reinterpret_cast<Context*>(rhiContext);
	RayTracingPipeline* pipeline = reinterpret_cast<RayTracingPipeline*>(rhiPipeline);

	CommandBuffer* commandBuffer = contextVk->getActiveCommandBuffer();

	VkPipelineBindPoint pipelineBindPoint = pipeline->getBindPoint();
	VkPipelineLayout pipelineLayout = pipeline->getLayout();
	commandBuffer->bindDescriptorSets(
		pipelineBindPoint, pipelineLayout, binding, 1, &descriptorSet, 0, nullptr);
}

void DescriptorSet::updateWriteDescriptorSet(rhi::DescriptorInfo& descriptorInfo)
{
	rhi::DescriptorType descriptorType = descriptorInfo.getType();

	switch (descriptorType)
	{
	case rhi::DescriptorType::Sampler:
	case rhi::DescriptorType::Combined_Image_Sampler:
	case rhi::DescriptorType::Sampled_Image:
	case rhi::DescriptorType::Input_Attachment:
	{
		uint32_t binding = static_cast<uint32_t>(writeDescriptorSets.size());
		VkWriteDescriptorSet& writeDescriptorSet = writeDescriptorSets.emplace_back();
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = convertToVkDescriptorType(descriptorType);
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.pImageInfo = reinterpret_cast<VkDescriptorImageInfo*>(descriptorInfo.getDescriptor()->getDescriptorData(descriptorType));
		break;
	}
	case rhi::DescriptorType::Storage_Image:
	{
		uint32_t binding = static_cast<uint32_t>(writeDescriptorSets.size());
		VkWriteDescriptorSet& writeDescriptorSet = writeDescriptorSets.emplace_back();
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = convertToVkDescriptorType(descriptorType);
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.pImageInfo = reinterpret_cast<VkDescriptorImageInfo*>(descriptorInfo.getDescriptor()->getDescriptorData(descriptorType));

		break;
	}
	case rhi::DescriptorType::Uniform_Texel_Buffer:
	case rhi::DescriptorType::Storage_Texel_Buffer:
	case rhi::DescriptorType::Uniform_Buffer:
	case rhi::DescriptorType::Uniform_Buffer_Dynamic:
	case rhi::DescriptorType::Storage_Buffer_Dynamic:
	{
		uint32_t binding = static_cast<uint32_t>(writeDescriptorSets.size());
		VkWriteDescriptorSet& writeDescriptorSet = writeDescriptorSets.emplace_back();
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = convertToVkDescriptorType(descriptorType);
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.pBufferInfo = reinterpret_cast<VkDescriptorBufferInfo*>(descriptorInfo.getDescriptor()->getDescriptorData(descriptorType));

		break;
	}
	case rhi::DescriptorType::Storage_Buffer:
	{
		uint32_t binding = static_cast<uint32_t>(writeDescriptorSets.size());
		VkWriteDescriptorSet& writeDescriptorSet = writeDescriptorSets.emplace_back();
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = convertToVkDescriptorType(descriptorType);
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.pBufferInfo = reinterpret_cast<VkDescriptorBufferInfo*>(descriptorInfo.getDescriptor()->getDescriptorData(descriptorType));

		break;
	}
	case rhi::DescriptorType::Acceleration_structure:
	{
		uint32_t binding = static_cast<uint32_t>(writeDescriptorSets.size());
		VkWriteDescriptorSet& writeDescriptorSet = writeDescriptorSets.emplace_back();
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.pNext = descriptorInfo.getDescriptor()->getDescriptorData(descriptorType);
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.dstArrayElement = 0;
		writeDescriptorSet.descriptorType = convertToVkDescriptorType(descriptorType);
		writeDescriptorSet.descriptorCount = 1;

		break;
	}
	}
}
}