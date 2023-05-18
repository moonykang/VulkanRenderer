#include "model/material.h"
#include "rhi/context.h"
#include "rhi/texture.h"
#include "rhi/descriptor.h"
#include "rhi/buffer.h"

namespace model
{
Material::MaterialUniformBlock::MaterialUniformBlock()
	: alphaMode(AlphaMode::ALPHAMODE_OPAQUE)
	, materialFactors(glm::vec4(1.f))
	, baseColorFactor(glm::vec4(1.f))
{

}

void Material::MaterialUniformBlock::setAlphaCutoff(float alphaCutoff)
{
	materialFactors.x = alphaCutoff;
}

void Material::MaterialUniformBlock::setMetallicFactor(float metallicFactor)
{
	materialFactors.y = metallicFactor;
}

void Material::MaterialUniformBlock::setRoughnessFactor(float roughnessFactor)
{
	materialFactors.z = roughnessFactor;
}

Material::Material()
	: materialUBO()
	, baseColorTexture(nullptr)
	, metallicRoughnessTexture(nullptr)
	, normalTexture(nullptr)
	, occlusionTexture(nullptr)
	, emissiveTexture(nullptr)
	, specularGlossinessTexture(nullptr)
	, diffuseTexture(nullptr)
	, materialUniformBuffer(nullptr)
	, materialDescriptorSet(nullptr)
{
}

void Material::init(rhi::Context* context)
{
	ASSERT(materialUniformBuffer == nullptr);
	ASSERT(materialDescriptorSet == nullptr);

	materialUniformBuffer = context->createUniformBuffer(rhi::BufferType::HostCoherent);
	materialDescriptorSet = context->createDescriptorSet();
}

void Material::destroy(rhi::Context* context)
{
	if (materialUniformBuffer != nullptr)
	{
		materialUniformBuffer->destroy(context);
		delete materialUniformBuffer;
		materialUniformBuffer = nullptr;
	}

	if (materialDescriptorSet != nullptr)
	{
		materialDescriptorSet->destroy(context);
		delete materialDescriptorSet;
		materialDescriptorSet = nullptr;
	}
}

void Material::build(rhi::Context* context)
{
	ASSERT(materialUniformBuffer);
	ASSERT(materialDescriptorSet);

	materialUniformBuffer->set<MaterialUniformBlock>(1, &materialUBO);
	materialUniformBuffer->build(context);

	materialDescriptorSet->registerDescriptor(rhi::ShaderStage::Fragment, rhi::DescriptorType::Uniform_Buffer, materialUniformBuffer);

	if (baseColorTexture)
	{
		materialDescriptorSet->registerDescriptor(rhi::ShaderStage::Fragment, rhi::DescriptorType::Combined_Image_Sampler, baseColorTexture);
	}

	if (metallicRoughnessTexture)
	{
		materialDescriptorSet->registerDescriptor(rhi::ShaderStage::Fragment, rhi::DescriptorType::Combined_Image_Sampler, metallicRoughnessTexture);
	}

	if (normalTexture)
	{
		materialDescriptorSet->registerDescriptor(rhi::ShaderStage::Fragment, rhi::DescriptorType::Combined_Image_Sampler, normalTexture);
	}

	if (occlusionTexture)
	{
		materialDescriptorSet->registerDescriptor(rhi::ShaderStage::Fragment, rhi::DescriptorType::Combined_Image_Sampler, occlusionTexture);
	}

	if (emissiveTexture)
	{
		materialDescriptorSet->registerDescriptor(rhi::ShaderStage::Fragment, rhi::DescriptorType::Combined_Image_Sampler, emissiveTexture);
	}

	if (specularGlossinessTexture)
	{
		materialDescriptorSet->registerDescriptor(rhi::ShaderStage::Fragment, rhi::DescriptorType::Combined_Image_Sampler, specularGlossinessTexture);
	}

	if (diffuseTexture)
	{
		materialDescriptorSet->registerDescriptor(rhi::ShaderStage::Fragment, rhi::DescriptorType::Combined_Image_Sampler, diffuseTexture);
	}
	materialDescriptorSet->build(context);
}

Material::MaterialUniformBlock& Material::getMaterialUniform()
{
	return materialUBO;
}

void Material::updateTexture(rhi::MaterialFlag materialFlag, rhi::Texture* texture)
{
	switch (materialFlag)
	{
	case rhi::MaterialFlag::BaseColorTexture:
		baseColorTexture = texture;
		break;
	case rhi::MaterialFlag::MetalicRoughnessTexture:
		metallicRoughnessTexture = texture;
		break;
	case rhi::MaterialFlag::NormalTexture:
		normalTexture = texture;
		break;
	case rhi::MaterialFlag::OcclusionTexture:
		occlusionTexture = texture;
		break;
	case rhi::MaterialFlag::EmissiveTexture:
		emissiveTexture = texture;
		break;
	case rhi::MaterialFlag::SpecularGlossinessTexture:
		specularGlossinessTexture = texture;
		break;
	case rhi::MaterialFlag::DiffuseTexture:
		diffuseTexture = texture;
		break;
	}
}

rhi::Texture* Material::getTexture(rhi::MaterialFlag materialFlag)
{
	switch (materialFlag)
	{
	case rhi::MaterialFlag::BaseColorTexture:
		return baseColorTexture;
	case rhi::MaterialFlag::MetalicRoughnessTexture:
		return metallicRoughnessTexture;
	case rhi::MaterialFlag::NormalTexture:
		return normalTexture;
	case rhi::MaterialFlag::OcclusionTexture:
		return occlusionTexture;
	case rhi::MaterialFlag::EmissiveTexture:
		return emissiveTexture;
	case rhi::MaterialFlag::SpecularGlossinessTexture:
		return specularGlossinessTexture;
	case rhi::MaterialFlag::DiffuseTexture:
		return diffuseTexture;
	}
}

rhi::DescriptorSet* Material::getDescriptorSet()
{
	ASSERT(materialDescriptorSet);
	return materialDescriptorSet;
}
}