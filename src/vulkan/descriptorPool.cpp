#include "vulkan/descriptorPool.h"

namespace vk
{
void DescriptorPool::init(VkDevice device)
{
	// TODO
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes;
	{
		auto& descriptorPoolSize = descriptorPoolSizes.emplace_back();
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorPoolSize.descriptorCount = 1000;
	}

	{
		auto& descriptorPoolSize = descriptorPoolSizes.emplace_back();
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorPoolSize.descriptorCount = 1000;
	}

	{
		auto& descriptorPoolSize = descriptorPoolSizes.emplace_back();
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		descriptorPoolSize.descriptorCount = 1000;
	}

	{
		auto& descriptorPoolSize = descriptorPoolSizes.emplace_back();
		descriptorPoolSize.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptorPoolSize.descriptorCount = 1000;
	}
	
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
	descriptorPoolCreateInfo.maxSets = 5000;

	VKCALL(descriptorPool.init(device, descriptorPoolCreateInfo));
}

void DescriptorPool::destroy(VkDevice device)
{
	descriptorPool.freeDescriptorSets(device, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data());
	descriptorSets.clear();
	descriptorPool.destroy(device);
}

VkDescriptorSet DescriptorPool::allocate(VkDevice device, VkDescriptorSetAllocateInfo& allocateInfo)
{
	ASSERT(descriptorPool.valid());

	VkDescriptorSet descriptorSetOut = descriptorSets.emplace_back();
	allocateInfo.descriptorPool = descriptorPool.getHandle();

	VKCALL(descriptorPool.allocateDescriptorSets(device, allocateInfo, &descriptorSetOut));

	ASSERT(descriptorSetOut != VK_NULL_HANDLE);

	return descriptorSetOut;
}
}