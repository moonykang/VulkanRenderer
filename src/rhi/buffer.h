#pragma once

#include <vector>
#include "platform/memorybuffer.h"
#include "rhi/resources.h"

namespace rhi
{
class Context;

class Buffer
{
public:
    Buffer()
        : count(0)
    {
    }

    template<typename T>
    inline void set(size_t size, const void* data)
    {
        size_t binarySize = size * (sizeof(T) / sizeof(uint8_t));
        memoryBuffer.set(binarySize, data);
        count = size;
    }

    util::MemoryBuffer* getBuffer() { return &memoryBuffer; }

    const size_t getCount() { return count; }

    size_t size() { return memoryBuffer.size(); }

    void clear() { memoryBuffer.clear(); }
protected:
	util::MemoryBuffer memoryBuffer;
    size_t count;
};

class ScratchBuffer : public Descriptor
{
public:
    virtual ~ScratchBuffer() = default;

    virtual void destroy(Context* context) = 0;

    virtual size_t preSuballocate(uint32_t size) = 0;

    virtual void build(Context* context) = 0;

    virtual uint64_t getDeviceAddress(Context* context) = 0;
};

struct SubAllocateInfo
{
    ScratchBuffer* buffer;
    size_t offset;
    size_t size;
};

struct VertexDescription
{
    uint32_t location;
    uint32_t binding;
    Format format;
    uint32_t offset;
};

struct VertexBindingDescription
{
    uint32_t binding;
    uint32_t stride;
};

class VertexBuffer
{
public:
    VertexBuffer();

    virtual ~VertexBuffer() = default;

    virtual void destroy(Context* context);

    void updateVertexDescriptions(VertexChannelFlags vertexChannelFlags);

    void append(VertexData& vertex);

    VertexData& at(size_t i);

    uint32_t size();

    uint32_t unitSize();

    virtual void build(Context* context) = 0;

    virtual void bind(Context* context) = 0;

    void suballocate(ScratchBuffer* scratchBuffer);
protected:
    std::vector<VertexData> vertices;
    uint32_t binding;
    uint32_t vertexSize;
    uint32_t vertexCount;
    VertexChannelFlags vertexChannels;
    VertexBindingDescription vertexBindingDescription;
    std::vector<VertexDescription> vertexDescriptions;
    SubAllocateInfo* subAllocateInfo;
};

class IndexBuffer
{
public:
    IndexBuffer();

    virtual ~IndexBuffer() = default;

    virtual void destroy(Context* context);

    void append(uint32_t indiex);

    virtual void build(Context* context) = 0;

    virtual void bind(Context* context) = 0;

    uint32_t size();

    void suballocate(ScratchBuffer* scratchBuffer);
protected:
    std::vector<uint32_t> indices;
    SubAllocateInfo* subAllocateInfo;
};

class UniformBuffer : public Buffer, public Descriptor
{
public:
    virtual ~UniformBuffer() = default;

    virtual void destroy(Context* context) = 0;

    virtual void build(Context* context) = 0;

    virtual void update(Context* context) = 0;

    virtual void bind(Context* context) = 0;
};

class StorageBuffer : public Buffer, public Descriptor
{
public:
    virtual ~StorageBuffer() = default;

    virtual void destroy(Context* context) = 0;

    virtual void build(Context* context) = 0;

    virtual void update(Context* context) = 0;

    virtual void bind(Context* context) = 0;
};
}