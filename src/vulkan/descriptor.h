#include "rhi/descriptor.h"
#include "vulkan/context.h"
#include "vulkan/vk_wrapper.h"

namespace rhi
{
class Context;
class UniformBuffer;
class Pipeline;
class DescriptorInfo;
}

namespace vk
{
class Context;

class DescriptorSet : public rhi::DescriptorSet
{
public:
	void destroy(rhi::Context* context) override;

	void build(rhi::Context* context) override;

	void bind(rhi::Context* context, rhi::GraphicsPipeline* pipeline, uint32_t binding) override;

	void bind(rhi::Context* context, rhi::ComputePipeline* pipeline, uint32_t binding) override;

	void bind(rhi::Context* context, rhi::RayTracingPipeline* pipeline, uint32_t binding) override;

	void updateWriteDescriptorSet(rhi::DescriptorInfo& descriptor);

	inline VkDescriptorSet& getHandle() { return descriptorSet; }

	inline VkDescriptorSetLayout getLayout() { return descriptorSetLayout.getHandle(); }
private:
	VkDescriptorSet descriptorSet;
	handle::DescriptorSetLayout descriptorSetLayout;
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
};
}