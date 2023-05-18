#pragma once
#include "platform/utils.h"
#include "rhi/resources.h"

namespace rhi
{
	class Texture;
	class Context;
	class UniformBuffer;
	class DescriptorSet;
}

namespace model
{
enum class AlphaMode
{
	ALPHAMODE_OPAQUE,
	ALPHAMODE_MASK,
	ALPHAMODE_BLEND
};

class Material
{
private:
	struct MaterialUniformBlock
	{
		glm::vec4 materialFactors; // x: alphaCutoff, y: metallicFactor, z: roughnessFactor, w: reserved
		glm::vec4 baseColorFactor;
		AlphaMode alphaMode; // align?

		MaterialUniformBlock();

		void setAlphaCutoff(float v);

		void setMetallicFactor(float v);

		void setRoughnessFactor(float v);
	};
public:
	Material();
	
	~Material() = default;

	void init(rhi::Context* context);

	void destroy(rhi::Context * context);

	void build(rhi::Context* context);

	MaterialUniformBlock& getMaterialUniform();

	void updateTexture(rhi::MaterialFlag materialFlag, rhi::Texture* texture);

	rhi::Texture* getTexture(rhi::MaterialFlag materialFlag);

	rhi::DescriptorSet* getDescriptorSet();
private:
	MaterialUniformBlock materialUBO;
	rhi::Texture* baseColorTexture;
	rhi::Texture* metallicRoughnessTexture;
	rhi::Texture* normalTexture;
	rhi::Texture* occlusionTexture;
	rhi::Texture* emissiveTexture;
	rhi::Texture* specularGlossinessTexture;
	rhi::Texture* diffuseTexture;

	rhi::UniformBuffer* materialUniformBuffer;
	rhi::DescriptorSet* materialDescriptorSet;
};
}