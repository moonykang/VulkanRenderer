#include "platform/utils.h"
#include "model/instance.h"
#include "model/material.h"
#include "rhi/context.h"
#include "platform/assetManager.h"
#include "rhi/buffer.h"
#include "rhi/descriptor.h"
#include "rhi/pipeline.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#ifdef VK_USE_PLATFORM_ANDROID_KHR
#define TINYGLTF_ANDROID_LOAD_FROM_ASSETS
#endif
#include "model/object.h"

namespace model
{
bool loadImageDataFuncEmpty(tinygltf::Image* image, const int imageIndex, std::string* error, std::string* warning, int req_width, int req_height, const unsigned char* bytes, int size, void* userData)
{
	// This function will be used for samples that don't require images to be loaded
	return true;
}

glm::mat4 Node::localMatrix() {
	return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
}

glm::mat4 Node::getMatrix() {
	glm::mat4 m = localMatrix();
	Node* p = parent;
	while (p) {
		m = p->localMatrix() * m;
		p = p->parent;
	}
	return m;
}

void Node::update() {
	if (mesh) {
		glm::mat4 m = getMatrix();		
		// memcpy(mesh->uniformBuffer.mapped, &m, sizeof(glm::mat4));
	}

	for (auto& child : children) {
		child->update();
	}
}

Node::~Node() {
	if (mesh) {
		delete mesh;
	}
	for (auto& child : children) {
		delete child;
	}
}

Object::Object()
	: vertexBuffer(nullptr)
	, indexBuffer(nullptr)
	, globalDescriptorSet(nullptr)
	, pipeline(nullptr)
	, localDescriptorSet(nullptr)
	, shaderModuleContainer(nullptr)
	, pipelineState(nullptr)
	, materialDescriptorSet(nullptr)
{

}
void Object::init(rhi::Context* context)
{
	if (vertexBuffer == nullptr)
	{
		vertexBuffer = context->createVertexBuffer();
	}
	
	if (indexBuffer == nullptr)
	{
		indexBuffer = context->createIndexBuffer();
	}

	if (globalDescriptorSet == nullptr)
	{
		globalDescriptorSet = context->createDescriptorSet();
	}

	if (shaderModuleContainer == nullptr)
	{
		shaderModuleContainer = context->createShaderModule();
	}

	if (pipelineState == nullptr)
	{
		pipelineState = new rhi::PipelineState();
	}

	initPipeline(context);
}

void Object::destroyVertexInput(rhi::Context* context)
{
	if (vertexBuffer != nullptr)
	{
		vertexBuffer->destroy(context);
		delete vertexBuffer;
		vertexBuffer = nullptr;
	}

	if (indexBuffer != nullptr)
	{
		indexBuffer->destroy(context);
		delete indexBuffer;
		indexBuffer = nullptr;
	}
}

void Object::destroy(rhi::Context* context)
{
	destroyVertexInput(context);

	if (globalDescriptorSet != nullptr)
	{
		globalDescriptorSet->destroy(context);
		delete globalDescriptorSet;
		globalDescriptorSet = nullptr;
	}

	if (pipeline != nullptr)
	{
		pipeline->destroy(context);
		delete pipeline;
		pipeline = nullptr;
	}

	if (shaderModuleContainer != nullptr)
	{
		shaderModuleContainer->destroy(context);
		delete shaderModuleContainer;
		shaderModuleContainer = nullptr;
	}

	if (pipelineState != nullptr)
	{
		delete pipelineState;
		pipelineState = nullptr;
	}

	for (auto& texture : textures)
	{
		texture->destroy(context);
		delete texture;
	}
	textures.clear();

	for (auto& material : materials)
	{
		material->destroy(context);
		delete material;
	}

	for (auto& node : linearNodes)
	{
		delete node;
	}

	for (auto& instance : instances)
	{
		instance.first->destroy(context);
		delete instance.first;
	}
	instances.clear();
}

void GraphicsObject::preBuild(rhi::Context* context)
{
	ASSERT(vertexBuffer);
	ASSERT(indexBuffer);
	vertexBuffer->build(context);
	indexBuffer->build(context);
}

void GraphicsObject::build(rhi::Context* context, rhi::RenderTarget* renderTarget)
{
	ASSERT(globalDescriptorSet);	
	ASSERT(pipeline);

	for (auto& texture : textures)
	{
		texture->build(context);
	}

	for (auto& material : materials)
	{
		material->build(context);
	}

	ASSERT(!instances.empty());
	for (auto& instance: instances)
	{
		instance.first->build(context);
	}
	ASSERT(localDescriptorSet);

	globalDescriptorSet->build(context);
	shaderModuleContainer->build(context);
	
	std::vector<rhi::DescriptorSet*> descriptorSets =
	{
		globalDescriptorSet,
		localDescriptorSet,
		materialDescriptorSet
	};

	pipeline->buildGraphics(context, *pipelineState, shaderModuleContainer, vertexBuffer, descriptorSets, renderTarget);
}

DerivedGraphicsObject* GraphicsObject::derive()
{
	return new DerivedGraphicsObject(vertexBuffer, indexBuffer);
}

rhi::Texture* Object::getTexture(uint32_t index)
{
	ASSERT(textures.size() > index);
	return textures[index];
}

void Object::loadTextures(rhi::Context* context, platform::AssetManager* assetManager, tinygltf::Model& gltfModel, std::string path)
{
	for (tinygltf::Image& image : gltfModel.images)
	{
		bool isKtx = false;
		if (image.uri.find_last_of(".") != std::string::npos)
		{
			if (image.uri.substr(image.uri.find_last_of(".") + 1) == "ktx")
			{
				isKtx = true;
			}
		}

		if (isKtx)
		{
			rhi::Texture* texture = context->createTexture(rhi::Format::R8G8B8A8_UNORM_SRGB, 500, 500, rhi::ImageLayout::TransferDst, rhi::ImageUsage::SAMPLED | rhi::ImageUsage::TRANSFER_DST);
			textures.push_back(texture);

			texture->loadTexture(assetManager, path + image.uri);
		}
	}
}

void Object::loadMaterials(rhi::Context* context, tinygltf::Model& gltfModel, rhi::MaterialFlags materialFlags)
{
	for (tinygltf::Material& mat : gltfModel.materials)
	{
		Material* material = new Material();
		materials.push_back(material);
		material->init(context);

		if ((materialFlags & rhi::MaterialFlag::BaseColorTexture) != 0 
			&& mat.values.find("baseColorTexture") != mat.values.end())
		{
			material->updateTexture(rhi::MaterialFlag::BaseColorTexture,
				getTexture(gltfModel.textures[mat.values["baseColorTexture"].TextureIndex()].source));
		}
		if ((materialFlags & rhi::MaterialFlag::MetalicRoughnessTexture) != 0 
			&& mat.values.find("metallicRoughnessTexture") != mat.values.end())
		{
			material->updateTexture(rhi::MaterialFlag::MetalicRoughnessTexture,
				getTexture(gltfModel.textures[mat.values["metallicRoughnessTexture"].TextureIndex()].source));
		}
		if ((materialFlags & rhi::MaterialFlag::NormalTexture) != 0 
			&& mat.additionalValues.find("normalTexture") != mat.additionalValues.end())
		{
			material->updateTexture(rhi::MaterialFlag::NormalTexture,
				getTexture(gltfModel.textures[mat.values["normalTexture"].TextureIndex()].source));
		}
		if ((materialFlags & rhi::MaterialFlag::EmissiveTexture) != 0 
			&& mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end())
		{
			material->updateTexture(rhi::MaterialFlag::EmissiveTexture,
				getTexture(gltfModel.textures[mat.values["emissiveTexture"].TextureIndex()].source));
		}
		if ((materialFlags & rhi::MaterialFlag::OcclusionTexture) != 0 
			&& mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end())
		{
			material->updateTexture(rhi::MaterialFlag::OcclusionTexture,
				getTexture(gltfModel.textures[mat.values["occlusionTexture"].TextureIndex()].source));
		}

		if (mat.values.find("roughnessFactor") != mat.values.end())
		{
			material->getMaterialUniform().setRoughnessFactor(static_cast<float>(mat.values["roughnessFactor"].Factor()));
		}
		if (mat.values.find("metallicFactor") != mat.values.end())
		{
			material->getMaterialUniform().setMetallicFactor(static_cast<float>(mat.values["metallicFactor"].Factor()));
		}
		if (mat.values.find("baseColorFactor") != mat.values.end())
		{
			material->getMaterialUniform().baseColorFactor = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
		}
		if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end())
		{
			tinygltf::Parameter param = mat.additionalValues["alphaMode"];
			if (param.string_value == "BLEND") {
				material->getMaterialUniform().alphaMode = AlphaMode::ALPHAMODE_BLEND;
			}
			if (param.string_value == "MASK") {
				material->getMaterialUniform().alphaMode = AlphaMode::ALPHAMODE_MASK;
			}
		}
		if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end())
		{
			material->getMaterialUniform().setAlphaCutoff(static_cast<float>(mat.additionalValues["alphaCutoff"].Factor()));
		}

		materialDescriptorSet = material->getDescriptorSet();
	}

	// empty material
	{
		Material* material = new Material();
		materials.push_back(material);
		material->init(context);

		if (materialDescriptorSet == nullptr)
		{
			materialDescriptorSet = material->getDescriptorSet();
		}
	}
}

void Object::loadNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, float globalscale, rhi::VertexChannelFlags desiredVertexChannelFlags)
{
	Node* newNode = new Node{};
	newNode->index = nodeIndex;
	newNode->parent = parent;
	newNode->name = node.name;
	newNode->skinIndex = node.skin;
	newNode->matrix = glm::mat4(1.0f);

	rhi::VertexChannelFlags vertexChannelFlags = 0;
	// Generate local node matrix
	glm::vec3 translation = glm::vec3(0.0f);
	if (node.translation.size() == 3)
	{
		translation = glm::make_vec3(node.translation.data());
		newNode->translation = translation;
	}
	
	glm::mat4 rotation = glm::mat4(1.0f);
	if (node.rotation.size() == 4)
	{
		glm::quat q = glm::make_quat(node.rotation.data());
		newNode->rotation = glm::mat4(q);
	}

	glm::vec3 scale = glm::vec3(1.0f);
	if (node.scale.size() == 3)
	{
		scale = glm::make_vec3(node.scale.data());
		newNode->scale = scale;
	}

	if (node.matrix.size() == 16)
	{
		newNode->matrix = glm::make_mat4x4(node.matrix.data());
		if (globalscale != 1.0f)
		{
			//newNode->matrix = glm::scale(newNode->matrix, glm::vec3(globalscale));
		}
	};

	// Node with children
	if (node.children.size() > 0)
	{
		for (auto i = 0; i < node.children.size(); i++)
		{
			loadNode(newNode, model.nodes[node.children[i]], node.children[i], model, globalscale, desiredVertexChannelFlags);
		}
	}

	// Node contains mesh data
	if (node.mesh > -1) {
		const tinygltf::Mesh mesh = model.meshes[node.mesh];
		Mesh* newMesh = new Mesh();

		newMesh->name = mesh.name;
		for (size_t j = 0; j < mesh.primitives.size(); j++)
		{
			const tinygltf::Primitive& primitive = mesh.primitives[j];

			if (primitive.indices < 0)
			{
				continue;
			}

			uint32_t indexStart = static_cast<uint32_t>(indexBuffer->size());
			uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer->size());
			uint32_t indexCount = 0;
			uint32_t vertexCount = 0;
			glm::vec3 posMin{};
			glm::vec3 posMax{};
			bool hasSkin = false;
			// Vertices
			{
				const float* bufferPos = nullptr;
				const float* bufferNormals = nullptr;
				const float* bufferTexCoords = nullptr;
				const float* bufferColors = nullptr;
				const float* bufferTangents = nullptr;
				uint32_t numColorComponents;
				const uint16_t* bufferJoints = nullptr;
				const float* bufferWeights = nullptr;

				// Position attribute is required
				assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

				const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
				bufferPos = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
				posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
				posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);

				ASSERT((desiredVertexChannelFlags & rhi::VertexChannel::Position) != 0);
				if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
					const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
					bufferNormals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));

					ASSERT((desiredVertexChannelFlags & rhi::VertexChannel::Normal) != 0);
				}

				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
					const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
					bufferTexCoords = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));

					ASSERT((desiredVertexChannelFlags & rhi::VertexChannel::Uv) != 0);
				}

				if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& colorAccessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& colorView = model.bufferViews[colorAccessor.bufferView];
					// Color buffer are either of type vec3 or vec4
					numColorComponents = colorAccessor.type == TINYGLTF_PARAMETER_TYPE_FLOAT_VEC3 ? 3 : 4;
					bufferColors = reinterpret_cast<const float*>(&(model.buffers[colorView.buffer].data[colorAccessor.byteOffset + colorView.byteOffset]));

					ASSERT((desiredVertexChannelFlags & rhi::VertexChannel::Color) != 0);
				}

				if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
				{
					const tinygltf::Accessor& tangentAccessor = model.accessors[primitive.attributes.find("TANGENT")->second];
					const tinygltf::BufferView& tangentView = model.bufferViews[tangentAccessor.bufferView];
					bufferTangents = reinterpret_cast<const float*>(&(model.buffers[tangentView.buffer].data[tangentAccessor.byteOffset + tangentView.byteOffset]));

					ASSERT((desiredVertexChannelFlags & rhi::VertexChannel::Tangent) != 0);
				}

				// Skinning
				// Joints
				if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
					bufferJoints = reinterpret_cast<const uint16_t*>(&(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]));

					ASSERT((desiredVertexChannelFlags & rhi::VertexChannel::Joint0) != 0);
				}

				if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
					bufferWeights = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));

					ASSERT((desiredVertexChannelFlags& rhi::VertexChannel::Weight0) != 0);
				}

				hasSkin = (bufferJoints && bufferWeights);

				vertexCount = static_cast<uint32_t>(posAccessor.count);

				for (size_t v = 0; v < posAccessor.count; v++)
				{
					rhi::VertexData vert{};
					vert.pos = glm::make_vec3(&bufferPos[v * 3]);
					vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * 3]) : glm::vec3(0.0f)));
					vert.uv = bufferTexCoords ? glm::make_vec2(&bufferTexCoords[v * 2]) : glm::vec3(0.0f);
					
					if (bufferColors)
					{
						switch (numColorComponents)
						{
						case 3:
							vert.color = glm::vec4(glm::make_vec3(&bufferColors[v * 3]), 1.0f);
						case 4:
							vert.color = glm::make_vec4(&bufferColors[v * 4]);
						}
					}
					else
					{
						vert.color = glm::vec4(1.0f);
					}
					glm::vec4 tangent = bufferTangents ? glm::vec4(glm::make_vec4(&bufferTangents[v * 4])) : glm::vec4(0.0f);
					vert.tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
					vert.bitangent = bufferTangents ? glm::cross(vert.normal, vert.tangent) * tangent.w : glm::vec3(0.f);
					vert.joint0 = hasSkin ? glm::vec4(glm::make_vec4(&bufferJoints[v * 4])) : glm::vec4(0.0f);
					vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * 4]) : glm::vec4(0.0f);
					vertexBuffer->append(vert);
				}
			}
			// Indices
			{
				const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				indexCount = static_cast<uint32_t>(accessor.count);

				switch (accessor.componentType) {
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
					uint32_t* buf = new uint32_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint32_t));
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer->append(buf[index] + vertexStart);
					}
					delete[] buf;
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					uint16_t* buf = new uint16_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint16_t));
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer->append(buf[index] + vertexStart);
					}
					delete[] buf;
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					uint8_t* buf = new uint8_t[accessor.count];
					memcpy(buf, &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(uint8_t));
					for (size_t index = 0; index < accessor.count; index++)
					{
						indexBuffer->append(buf[index] + vertexStart);
					}
					delete[] buf;
					break;
				}
				default:
					//std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
					return;
				}
			}

			Primitive* newPrimitive = new Primitive(indexStart, indexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
			newPrimitive->firstVertex = vertexStart;
			newPrimitive->vertexCount = vertexCount;
			//newPrimitive->setDimensions(posMin, posMax);
			newMesh->primitives.push_back(newPrimitive);
		}
		newNode->mesh = newMesh;
	}

	if (parent)
	{
		parent->children.push_back(newNode);
	}
	else
	{
		nodes.push_back(newNode);
	}
	linearNodes.push_back(newNode);
}

void Object::enableVertexChannel(rhi::VertexChannelFlags vertexChannelFlags)
{
	ASSERT(vertexBuffer);
	vertexBuffer->updateVertexDescriptions(vertexChannelFlags);
}

void Object::udpateLocalDescriptorSet(rhi::DescriptorSet* descriptorSet)
{
	if (localDescriptorSet == nullptr)
	{
		localDescriptorSet = descriptorSet;
	}
}

void Object::updateShaderCode(platform::AssetManager* assetManager, rhi::ShaderStage shaderStage, std::string path)
{
	ASSERT(shaderModuleContainer);
	shaderModuleContainer->updateShaderCode(assetManager, shaderStage, path);
}

std::vector<std::pair<Instance*, glm::mat4>>& Object::getInstances()
{
	return instances;
}

void Object::registerDescriptor(rhi::DescriptorType descriptorType, rhi::ShaderStageFlags stage, rhi::Descriptor* descriptor)
{
	globalDescriptorSet->registerDescriptor(stage, descriptorType, descriptor);
}

Instance* Object::instantiate(rhi::Context* context, glm::mat4 transform)
{
	Instance* prevInstance = nullptr;
	for (Node* node : linearNodes)
	{
		if (node->mesh)
		{
			const glm::mat4 localMatrix = transform * node->getMatrix();
			for (Primitive* primitive : node->mesh->primitives)
			{
				Instance* newInstance = new Instance(this, prevInstance, primitive->firstIndex, primitive->indexCount, primitive->firstVertex, primitive->vertexCount, localMatrix);
				prevInstance = newInstance;
				newInstance->init(context);
				newInstance->updateMaterialDescriptorSet(primitive->material->getDescriptorSet());
			}
		}
	}

	ASSERT(prevInstance != nullptr);
	instances.push_back(std::make_pair(prevInstance, transform));
	return prevInstance;
}

rhi::PipelineState* Object::getPipelineState()
{
	return pipelineState;
}

rhi::VertexBuffer* Object::getVertexBuffer()
{
	return vertexBuffer;
}

rhi::IndexBuffer* Object::getIndexBuffer()
{
	return indexBuffer;
}

void Object::loadGltfModel(rhi::Context* context, platform::AssetManager* assetManager, std::string path, std::string filename, GltfLoadingFlags loadFlags, rhi::VertexChannelFlags desiredVertexChannelFlags, rhi::MaterialFlags materialFlags)
{
	tinygltf::Model gltfModel;
	tinygltf::TinyGLTF gltfContext;
	vertexBuffer->updateVertexDescriptions(desiredVertexChannelFlags);

	// On Android all assets are packed with the apk in a compressed form, so we need to open them using the asset manager
	// We let tinygltf handle this, by passing the asset manager of our app
#if PLATFORM_ANDROID
	tinygltf::asset_manager = assetManager->getAssetManager();
#endif
	
	gltfContext.SetImageLoader(loadImageDataFuncEmpty, nullptr);

	std::string error, warning;

	std::string fullFileName = path + filename;
#if PLATFORM_WINDOW
	bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, assetManager->getAssetPath() + "/" + fullFileName);
#else
	bool fileLoaded = gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, fullFileName);
#endif

	ASSERT(fileLoaded);

	if (!(loadFlags & GltfLoadingFlag::DontLoadImages))
	{
		loadTextures(context, assetManager, gltfModel, path);
	}

	loadMaterials(context, gltfModel, materialFlags);

	const tinygltf::Scene& scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
	for (size_t i = 0; i < scene.nodes.size(); i++)
	{
		const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
		loadNode(nullptr, node, scene.nodes[i], gltfModel, 1.f, desiredVertexChannelFlags);
	}

	for (auto node : linearNodes)
	{
		// Initial pose
		if (node->mesh)
		{
			node->update();
		}
	}

	if ((loadFlags & GltfLoadingFlag::PreTransformVertices) || (loadFlags & GltfLoadingFlag::PreMultiplyVertexColors) || (loadFlags & GltfLoadingFlag::FlipY))
	{
		const bool preTransform = loadFlags & GltfLoadingFlag::PreTransformVertices;
		const bool preMultiplyColor = loadFlags & GltfLoadingFlag::PreMultiplyVertexColors;
		const bool flipY = loadFlags & GltfLoadingFlag::FlipY;

		for (Node* node : linearNodes)
		{
			if (node->mesh)
			{
				const glm::mat4 localMatrix = node->getMatrix();
				for (Primitive* primitive : node->mesh->primitives)
				{
					for (uint32_t i = 0; i < primitive->vertexCount; i++)
					{
						rhi::VertexData& vertex = vertexBuffer->at(primitive->firstVertex + i);
						// Pre-transform vertex positions by node-hierarchy
						if (preTransform) {
							vertex.pos = glm::vec3(localMatrix * glm::vec4(vertex.pos, 1.0f));
							vertex.normal = glm::normalize(glm::mat3(localMatrix) * vertex.normal);
						}
						// Flip Y-Axis of vertex positions
						if (flipY) {
							vertex.pos.y *= -1.0f;
							vertex.normal.y *= -1.0f;
						}
					}
				}
			}
		}

		if (preTransform)
		{
			for (Node* node : linearNodes)
			{
				node->matrix = glm::mat4(1.f);
				node->translation = glm::vec3(0.0f);
				node->rotation = glm::mat4(1.0f);
				node->scale = glm::vec3(1.0f);
			}
		}
	}
}

void GraphicsObject::initPipeline(rhi::Context* context)
{
	if (pipeline == nullptr)
	{
		pipeline = context->createPipeline(rhi::PipelineType::Graphics);
	}
}

void GraphicsObject::draw(rhi::Context* context)
{
	vertexBuffer->bind(context);
	indexBuffer->bind(context);

	pipeline->bind(context);

	rhi::GraphicsPipeline* graphicsPipeline = reinterpret_cast<rhi::GraphicsPipeline*>(pipeline);
	globalDescriptorSet->bind(context, graphicsPipeline, 0);

	for (auto& instance : instances)
	{
		instance.first->draw(context, graphicsPipeline);
	}
}

ComputeObject::ComputeObject()
	: Object()
	, groupCountX(1)
	, groupCountY(1)
	, groupCountZ(1)
	, bIndirect(false)
	, indirectStorageBuffer(nullptr)
{

}

void ComputeObject::initPipeline(rhi::Context* context)
{
	if (pipeline == nullptr)
	{
		pipeline = context->createPipeline(rhi::PipelineType::Compute);
	}
}

void ComputeObject::draw(rhi::Context* context)
{
	pipeline->bind(context);
	rhi::ComputePipeline* computePipeline = reinterpret_cast<rhi::ComputePipeline*>(pipeline);
	globalDescriptorSet->bind(context, computePipeline, 0);

	if (bIndirect)
	{
		context->dispatchIndirect(indirectStorageBuffer);
	}
	else
	{
		context->dispatch(groupCountX, groupCountY, groupCountZ);
	}	
}

void ComputeObject::preBuild(rhi::Context* context)
{

}

void ComputeObject::build(rhi::Context* context, rhi::RenderTarget* renderTarget)
{
	ASSERT(globalDescriptorSet);
	ASSERT(pipeline);

	globalDescriptorSet->build(context);
	shaderModuleContainer->build(context);

	std::vector<rhi::DescriptorSet*> descriptorSets =
	{
		globalDescriptorSet
	};

	pipeline->buildCompute(context, shaderModuleContainer, descriptorSets);
}

void ComputeObject::setGroupCount(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	this->groupCountX = groupCountX;
	this->groupCountY = groupCountY;
	this->groupCountZ = groupCountZ;
}

void ComputeObject::setIndirect(rhi::StorageBuffer* storageBuffer)
{
	bIndirect = true;
	indirectStorageBuffer = storageBuffer;
}

void RayTracingObject::initPipeline(rhi::Context* context)
{
	if (pipeline == nullptr)
	{
		pipeline = context->createPipeline(rhi::PipelineType::RayTracing);
	}
}

void RayTracingObject::draw(rhi::Context* context)
{
	pipeline->bind(context);

	rhi::GraphicsPipeline* graphicsPipeline = reinterpret_cast<rhi::GraphicsPipeline*>(pipeline);
	globalDescriptorSet->bind(context, graphicsPipeline, 0);

	for (auto& instance : instances)
	{
		//instance->draw(context, graphicsPipeline);
	}
}

void RayTracingObject::preBuild(rhi::Context* context)
{

}

void RayTracingObject::build(rhi::Context* context, rhi::RenderTarget* renderTarget)
{
	ASSERT(globalDescriptorSet);
	ASSERT(pipeline);

	globalDescriptorSet->build(context);
	shaderModuleContainer->build(context);

	pipeline->buildRayTracing(context, shaderModuleContainer, globalDescriptorSet);
}

DerivedGraphicsObject::DerivedGraphicsObject(rhi::VertexBuffer* vertexBuffer, rhi::IndexBuffer* indexBuffer)
{
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = indexBuffer;
}

void DerivedGraphicsObject::destroyVertexInput(rhi::Context* context)
{
	vertexBuffer = nullptr;
	indexBuffer = nullptr;
}
}
