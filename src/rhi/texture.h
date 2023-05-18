#pragma once

#include <string>
#include "rhi/buffer.h"
#include "rhi/resources.h"
#include "vulkan/context.h"

namespace platform
{
class AssetManager;
}

namespace rhi
{
class Context;

class SamplerInfo
{
private:
    SamplerInfo(SampleMode magFilter, SampleMode minFilter, SampleMode mipmapMode, bool anisotropyEnable, bool unnormalizedCoordinates);
public:
    SampleMode getMagFilter();
    SampleMode getMinFilter();
    SampleMode getMipmapMode();
    bool getAnisotropyEnable();
    bool getUnnormalizedCoordinates();
private:
    SampleMode magFilter;
    SampleMode minFilter;
    SampleMode mipmapMode;
    bool anisotropyEnable;
    bool unnormalizedCoordinates;

public:
    class Builder
    {
    public:
        Builder();

        Builder& setMagFilter(SampleMode magFilter);

        Builder& setMinFilter(SampleMode minFilter);

        Builder& setMipmapMode(SampleMode mipmapMode);

        Builder& setAnisotropyEnable(bool anisotropyEnable);

        Builder& setUnnormalizedCoordinates(bool unnormalizedCoordinates);

        SamplerInfo build() const;
    private:
        SampleMode magFilter;
        SampleMode minFilter;
        SampleMode mipmapMode;
        bool anisotropyEnable;
        bool unnormalizedCoordinates;
    };
};

class Texture : public Buffer, public Descriptor
{
public:
    Texture(Format format, uint32_t width, uint32_t height, ImageLayout initialLayout, uint32_t usage);
    Texture(Format format, uint32_t width, uint32_t height, ImageLayout initialLayout, uint32_t usage, SamplerInfo samplerInfo);
    Texture(Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels, uint32_t layers, ImageLayout initialLayout, uint32_t usage);
    Texture(Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels, uint32_t layers, ImageLayout initialLayout, uint32_t usage, SamplerInfo samplerInfo);

    virtual ~Texture() = default;

    void loadTexture(platform::AssetManager* assetManager, std::string path);

    virtual void destroy(Context* context) = 0;

    virtual void build(Context* context) = 0;

    virtual void clearColor(Context* context, float r, float g, float b, float a) = 0;

    virtual void CopyTo(Context* context, Texture* dstTexture, uint32_t srcMipLevel, uint32_t dstMipLevel) = 0;
public:
    void setSamplerInfo(SamplerInfo info);

protected:
    Format format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t samples;
    uint32_t mipLevels;
    uint32_t layers;
    ImageLayout initialLayout;
    uint32_t usage;
    SamplerInfo samplerInfo;
    bool textureLoaded;
    std::vector<std::pair<uint32_t, size_t>> mipOffsets;
};
}