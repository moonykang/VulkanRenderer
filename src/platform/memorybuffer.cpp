#include "platform/memorybuffer.h"

#include <algorithm>
#include <cstdlib>

namespace util
{
// MemoryBuffer implementation.
MemoryBuffer::~MemoryBuffer()
{
    if (mData)
    {
        free(mData);
        mData = nullptr;
    }
}

bool MemoryBuffer::resize(size_t size, size_t typeSize)
{
    size_t unitSize = typeSize / sizeof(uint8_t);
    return resize(size * unitSize);
}

bool MemoryBuffer::resize(size_t size)
{
    if (size == 0)
    {
        if (mData)
        {
            free(mData);
            mData = nullptr;
        }
        mSize = 0;
        return true;
    }

    if (size == mSize)
    {
        return true;
    }

    // Only reallocate if the size has changed.
    uint8_t *newMemory = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * size));
    if (newMemory == nullptr)
    {
        return false;
    }

    if (mData)
    {
        // Copy the intersection of the old data and the new data
        std::copy(mData, mData + std::min(mSize, size), newMemory);
        free(mData);
    }

    mData = newMemory;
    mSize = size;

    return true;
}

void MemoryBuffer::set(size_t size, const void* data)
{
    resize(size);
    memcpy(mData, data, size);
}

void MemoryBuffer::fill(uint8_t datum)
{
    if (!empty())
    {
        std::fill(mData, mData + mSize, datum);
    }
}

MemoryBuffer::MemoryBuffer(MemoryBuffer &&other) : MemoryBuffer()
{
    *this = std::move(other);
}

MemoryBuffer &MemoryBuffer::operator=(MemoryBuffer &&other)
{
    std::swap(mSize, other.mSize);
    std::swap(mData, other.mData);
    return *this;
}
}