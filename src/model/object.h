#pragma once

#include <vector>
#include "platform/utils.h"
#include "rhi/resources.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

namespace rhi
{
	class Context;
	class Pipeline;
	class VertexBuffer;
	class IndexBuffer;
	class DescriptorSet;
	class Texture;
	class ShaderModuleContainer;
	class PipelineState;
	class RenderTarget;
	class StorageBuffer;
}

namespace platform
{
	class AssetManager;
}

namespace model
{
class DerivedGraphicsObject;
class Instance;
class Material;

enum GltfLoadingFlag
{
	None = 0x00000000,
	PreTransformVertices = 0x00000001,
	PreMultiplyVertexColors = 0x00000002,
	FlipY = 0x00000004,
	DontLoadImages = 0x00000008
};
typedef uint32_t GltfLoadingFlags;

struct Primitive {
	uint32_t firstIndex;
	uint32_t indexCount;
	uint32_t firstVertex;
	uint32_t vertexCount;
	Material* material;

	Primitive(uint32_t firstIndex, uint32_t indexCount, Material* material) : firstIndex(firstIndex), indexCount(indexCount), material(material) {};
};

struct Mesh {
	std::vector<Primitive*> primitives;
	std::string name;
};

struct Node {
	Node* parent;
	uint32_t index;
	std::vector<Node*> children;
	glm::mat4 matrix;
	std::string name;
	Mesh* mesh;
	int32_t skinIndex = -1;
	glm::vec3 translation{};
	glm::vec3 scale{ 1.0f };
	glm::quat rotation{};

	glm::mat4 localMatrix();

	glm::mat4 getMatrix();

	void update();

	~Node();
};

class Object
{
public:
	Object();

	~Object() = default;

	void init(rhi::Context* context);

	virtual void initPipeline(rhi::Context* context) = 0;

	virtual void destroyVertexInput(rhi::Context* context);

	void destroy(rhi::Context* context);

	virtual void preBuild(rhi::Context* context) = 0;

	virtual void build(rhi::Context* context, rhi::RenderTarget* renderTarget) = 0;

	virtual void draw(rhi::Context* context) {}

	void loadGltfModel(rhi::Context* context, platform::AssetManager* assetManager, std::string path, std::string filename, GltfLoadingFlags loadFlags, rhi::VertexChannelFlags desiredVertexChannelFlags, rhi::MaterialFlags materialFlags);

	void loadPredefinedScreen(rhi::Context* context);

	void loadTextures(rhi::Context* context, platform::AssetManager* assetManager, tinygltf::Model& gltfModel, std::string path);

	void loadMaterials(rhi::Context* context, tinygltf::Model& gltfModel, rhi::MaterialFlags materialFlags);

	void loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, float globalscale, rhi::VertexChannelFlags desiredVertexChannelFlags);

	void enableVertexChannel(rhi::VertexChannelFlags vertexChannelFlags);

	void registerDescriptor(rhi::DescriptorType descriptorType, rhi::ShaderStageFlags stage, rhi::Descriptor* descriptor);

	void udpateLocalDescriptorSet(rhi::DescriptorSet* descriptorSet);

	void updateShaderCode(platform::AssetManager* assetManager, rhi::ShaderStage shaderStage, std::string path);

	std::vector<std::pair<Instance*, glm::mat4>>& getInstances();

	Instance* instantiate(rhi::Context* context, glm::mat4 transform);

	rhi::PipelineState* getPipelineState();

	rhi::VertexBuffer* getVertexBuffer();

	rhi::IndexBuffer* getIndexBuffer();

private:
	rhi::Texture* getTexture(uint32_t index);

protected:
	rhi::VertexBuffer* vertexBuffer;
	rhi::IndexBuffer* indexBuffer;
	rhi::Pipeline* pipeline;
	rhi::DescriptorSet* globalDescriptorSet; // Per-object
	rhi::DescriptorSet* localDescriptorSet; // Per-instance
	rhi::DescriptorSet* materialDescriptorSet; // Per-material

	rhi::ShaderModuleContainer* shaderModuleContainer;
	rhi::PipelineState* pipelineState;

	std::vector<rhi::Texture*> textures;
	std::vector<Material*> materials;
	std::vector<Node*> nodes;
	std::vector<Node*> linearNodes;
	std::vector<std::pair<Instance*, glm::mat4>> instances;
};

class GraphicsObject : public Object
{
public:
	void initPipeline(rhi::Context* context) override;

	void draw(rhi::Context* context) override;

	void preBuild(rhi::Context* context) override;

	void build(rhi::Context* context, rhi::RenderTarget* renderTarget) override;

	DerivedGraphicsObject* derive();
};

class ComputeObject : public Object
{
public:
	ComputeObject();

	void initPipeline(rhi::Context* context) override;

	void draw(rhi::Context* context) override;

	void preBuild(rhi::Context* context) override;

	void build(rhi::Context* context, rhi::RenderTarget* renderTarget) override;

	void setGroupCount(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

	void setIndirect(rhi::StorageBuffer* storageBuffer);
private:
	bool bIndirect;
	uint32_t groupCountX;
	uint32_t groupCountY;
	uint32_t groupCountZ;
	rhi::StorageBuffer* indirectStorageBuffer;
};

class RayTracingObject : public Object
{
public:
	void initPipeline(rhi::Context* context) override;

	void draw(rhi::Context* context) override;

	void preBuild(rhi::Context* context) override;

	void build(rhi::Context* context, rhi::RenderTarget* renderTarget) override;
};

class DerivedGraphicsObject : public GraphicsObject
{
public:
	DerivedGraphicsObject(rhi::VertexBuffer* vertexBuffer, rhi::IndexBuffer* indexBuffer);

	void destroyVertexInput(rhi::Context* context) override;;
};
}