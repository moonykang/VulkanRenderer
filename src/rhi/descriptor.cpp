#include "rhi/descriptor.h"
#include "rhi/accelerationStructure.h"

namespace rhi
{
DescriptorInfo::DescriptorInfo(ShaderStageFlags stage, DescriptorType type, Descriptor* descriptor)
	: stage(stage)
	, type(type)
	, descriptor(descriptor)
{
}

uint32_t DescriptorInfo::getStage()
{
	return stage;
}

DescriptorType DescriptorInfo::getType()
{
	return type;
}

Descriptor* DescriptorInfo::getDescriptor()
{
	return descriptor;
}

DescriptorSet::DescriptorSet()
	: binding(0)
{

}

void DescriptorSet::registerDescriptor(ShaderStageFlags stage, DescriptorType type, Descriptor* descriptor)
{
	descriptors.push_back(DescriptorInfo(stage, type, descriptor));
}
}