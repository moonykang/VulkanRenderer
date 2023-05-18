#pragma once

#include <vector>
#include "vulkan/vk_wrapper.h"

namespace vk
{
class DescriptorPool
{
public:
	void init(VkDevice device);

	void destroy(VkDevice device);

	VkDescriptorSet allocate(VkDevice device, VkDescriptorSetAllocateInfo& allocateInfo);

private:
	handle::DescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;
};
}