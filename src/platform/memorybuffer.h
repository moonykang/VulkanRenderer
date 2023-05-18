#pragma once

#include "platform/utils.h"

namespace util
{
class MemoryBuffer final : NonCopyable
{
public:
    MemoryBuffer() = default;
    ~MemoryBuffer();

    MemoryBuffer(MemoryBuffer &&other);
    MemoryBuffer &operator=(MemoryBuffer &&other);

    bool resize(size_t size);

    bool resize(size_t size, size_t typeSize);

    void set(size_t size, const void* data);

    void clear()
    {
        resize(0);
    }

    size_t size() const
    {
        return mSize;
    }

    bool empty() const
    {
        return mSize == 0;
    }

    const uint8_t *data() const
    {
        return mData;
    }

    uint8_t *data()
    {
        ASSERT(mData);
        return mData;
    }

    uint8_t &operator[](size_t pos)
    {
        ASSERT(pos < mSize);
        return mData[pos];
    }
    const uint8_t &operator[](size_t pos) const
    {
        ASSERT(pos < mSize);
        return mData[pos];
    }

    void fill(uint8_t datum);

private:
    size_t mSize = 0;
    uint8_t *mData = nullptr;
};
}