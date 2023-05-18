#pragma once

#include "rhi/texture.h"
#include "rhi/buffer.h"
#include "rhi/resources.h"

namespace rhi
{
class Transition
{
public:
    // TODO: make it use ImageLayout
    Transition(Texture* texture, MemoryAccessFlags accessFlags)
        : texture(texture)
        , buffer(nullptr)
        , accessFlags(accessFlags)
    {

    }

    Transition(StorageBuffer* buffer, MemoryAccessFlags accessFlags)
            : texture(nullptr)
            , buffer(buffer)
            , accessFlags(accessFlags)
    {

    }

    Texture* getTexture() { return texture; }

    StorageBuffer* getBuffer() { return buffer; }

    MemoryAccessFlags getMemoryAccessFlags() { return accessFlags; }

private:
    MemoryAccessFlags accessFlags;
    Texture* texture;
    StorageBuffer* buffer;
};
}