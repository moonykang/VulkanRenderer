#include "rhi/buffer.h"

namespace rhi
{
constexpr uint32_t kChannelUnit = sizeof(float);

VertexBuffer::VertexBuffer()
    : binding(0)
	, vertexSize(0)
	, vertexCount(0)
    , vertexBindingDescription()
	, subAllocateInfo(nullptr)
{
}

void VertexBuffer::destroy(Context* context)
{
	if (subAllocateInfo != nullptr)
	{
		delete subAllocateInfo;
		subAllocateInfo = nullptr;
	}
}

void VertexBuffer::updateVertexDescriptions(VertexChannelFlags vertexChannelFlags)
{
	vertexChannels = vertexChannelFlags;

	vertexSize = sizeof(rhi::VertexData);

	vertexBindingDescription.binding = 0;
	vertexBindingDescription.stride = vertexSize;

	uint32_t binding = 0;

	if ((vertexChannels & rhi::VertexChannel::Position) != 0)
	{
		vertexDescriptions.push_back({ 0, binding++, Format::R32G32B32_FLOAT, offsetof(VertexData, pos)});
	}

	if ((vertexChannels & rhi::VertexChannel::Normal) != 0)
	{
		vertexDescriptions.push_back({ 0, binding++, Format::R32G32B32_FLOAT, offsetof(VertexData, normal) });
	}

	if ((vertexChannels & rhi::VertexChannel::Uv) != 0)
	{
		vertexDescriptions.push_back({ 0, binding++, Format::R32G32_FLOAT, offsetof(VertexData, uv) });
	}

	if ((vertexChannels & rhi::VertexChannel::Color) != 0)
	{
		vertexDescriptions.push_back({ 0, binding++, Format::R32G32B32A32_FLOAT, offsetof(VertexData, color) });
	}

	if ((vertexChannels & rhi::VertexChannel::Joint0) != 0)
	{
		vertexDescriptions.push_back({ 0, binding++, Format::R32G32B32A32_FLOAT, offsetof(VertexData, joint0) });
	}

	if ((vertexChannels & rhi::VertexChannel::Weight0) != 0)
	{
		vertexDescriptions.push_back({ 0, binding++, Format::R32G32B32A32_FLOAT, offsetof(VertexData, weight0) });
	}

	if ((vertexChannels & rhi::VertexChannel::Tangent) != 0)
	{
		vertexDescriptions.push_back({ 0, binding++, Format::R32G32B32_FLOAT, offsetof(VertexData, tangent) });
	}

	if ((vertexChannels & rhi::VertexChannel::Bitangent) != 0)
	{
		vertexDescriptions.push_back({ 0, binding++, Format::R32G32B32_FLOAT, offsetof(VertexData, bitangent) });
	}
}

void VertexBuffer::append(VertexData& vertex)
{
	vertices.push_back(vertex);
}

VertexData& VertexBuffer::at(size_t i)
{
	ASSERT(vertices.size() > i);
	return vertices.at(i);
}

uint32_t VertexBuffer::size()
{
	return static_cast<uint32_t>(vertices.size());
}

uint32_t VertexBuffer::unitSize()
{
	return vertexSize;
}

void VertexBuffer::suballocate(ScratchBuffer* scratchBuffer)
{
	size_t bufferSize = sizeof(VertexData) * vertices.size();
	subAllocateInfo = new SubAllocateInfo();
	subAllocateInfo->buffer = scratchBuffer;
	subAllocateInfo->offset = scratchBuffer->preSuballocate(bufferSize);
	subAllocateInfo->size = bufferSize;
}

IndexBuffer::IndexBuffer()
	: subAllocateInfo(nullptr)
{
}

void IndexBuffer::destroy(Context* context)
{
	if (subAllocateInfo != nullptr)
	{
		delete subAllocateInfo;
		subAllocateInfo = nullptr;
	}
}

void IndexBuffer::append(uint32_t index)
{
	indices.push_back(index);
}

void IndexBuffer::suballocate(ScratchBuffer* scratchBuffer)
{
	size_t bufferSize = sizeof(uint32_t) * indices.size();
	subAllocateInfo = new SubAllocateInfo();
	subAllocateInfo->buffer = scratchBuffer;
	subAllocateInfo->offset = scratchBuffer->preSuballocate(bufferSize);
	subAllocateInfo->size = bufferSize;
}

uint32_t IndexBuffer::size()
{
    return static_cast<uint32_t>(indices.size());
}
}