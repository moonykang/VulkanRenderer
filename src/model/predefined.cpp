#include "model/object.h"
#include "model/material.h"
#include "rhi/context.h"
#include "rhi/buffer.h"
#include "rhi/resources.h"

namespace model
{
void Object::loadPredefinedScreen(rhi::Context* context)
{
	Node* newNode = new Node{};
	newNode->index = 0;
	newNode->parent = nullptr;
	newNode->name = "Screen";
	newNode->matrix = glm::mat4(1.0f);

	newNode->mesh = new Mesh();
	
	uint32_t indexStart = 0;
	uint32_t vertexStart = 0;

	const uint32_t slices = 2;
	const uint32_t stacks = 2;

	const uint32_t top_left = 0;
	const uint32_t top_right = 1;
	const uint32_t bottom_left = 2;
	const uint32_t bottom_right = 3;
	/*
	
    The s coordinate goes from 0.0 to 1.0, left to right.
    The t coordinate goes from 0.0 to 1.0, top to bottom.

	*/
	static const uint32_t num_angles = 4;
	const glm::vec3 positions[num_angles] =
	{
		{ -1.0f,  1.0f, 0.0f },
		{ -1.0f, -1.0f, 0.0f },
		{  1.0f, 1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f }
	};
	const glm::vec2 uvs[num_angles] =
	{
		{ 0.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f }		
	};
	for (uint32_t i = 0; i < num_angles; i++)
	{
		rhi::VertexData vertex;
		vertex.pos = positions[i];
		vertex.uv = uvs[i];
		vertexBuffer->append(vertex);
	}
	vertexBuffer->updateVertexDescriptions(rhi::VertexChannel::Position | rhi::VertexChannel::Uv);

	indexBuffer->append(0);
	indexBuffer->append(1);
	indexBuffer->append(2);
	indexBuffer->append(2);
	indexBuffer->append(1);
	indexBuffer->append(3);

	uint32_t indexCount = indexBuffer->size();
	uint32_t vertexCount = vertexBuffer->size();
	

	Material* material = new Material();
	materials.push_back(material);
	material->init(context);

	if (materialDescriptorSet == nullptr)
	{
		materialDescriptorSet = material->getDescriptorSet();
	}

	Primitive* newPrimitive = new Primitive(indexStart, indexCount, material);
	newPrimitive->firstVertex = vertexStart;
	newPrimitive->vertexCount = vertexCount;
	newNode->mesh->primitives.push_back(newPrimitive);

	nodes.push_back(newNode);
	linearNodes.push_back(newNode);
}
}