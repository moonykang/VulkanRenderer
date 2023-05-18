#include "rhi/texture.h"
#include "platform/assetManager.h"

namespace rhi
{
SamplerInfo::SamplerInfo(SampleMode magFilter, SampleMode minFilter, SampleMode mipmapMode, bool anisotropyEnable, bool unnormalizedCoordinates)
    : magFilter(magFilter)
    , minFilter(minFilter)
    , mipmapMode(mipmapMode)
    , anisotropyEnable(anisotropyEnable)
    , unnormalizedCoordinates(unnormalizedCoordinates)
{
}


SamplerInfo::Builder::Builder()
    : magFilter(SampleMode::Nearest)
    , minFilter(SampleMode::Nearest)
    , mipmapMode(SampleMode::Nearest)
    , anisotropyEnable(false)
    , unnormalizedCoordinates(false)
{

}
SamplerInfo::Builder& SamplerInfo::Builder::setMagFilter(SampleMode magFilter)
{
    this->magFilter = magFilter;
    return *this;
}
SamplerInfo::Builder& SamplerInfo::Builder::setMinFilter(SampleMode minFilter)
{
    this->minFilter = minFilter;
    return *this;
}
SamplerInfo::Builder& SamplerInfo::Builder::setMipmapMode(SampleMode mipmapMode)
{
    this->mipmapMode = mipmapMode;
    return *this;
}
SamplerInfo::Builder& SamplerInfo::Builder::setAnisotropyEnable(bool anisotropyEnable)
{
    this->anisotropyEnable = anisotropyEnable;
    return *this;
}
SamplerInfo::Builder& SamplerInfo::Builder::setUnnormalizedCoordinates(bool unnormalizedCoordinates)
{
    this->unnormalizedCoordinates = unnormalizedCoordinates;
    return *this;
}

SampleMode SamplerInfo::getMagFilter()
{
    return magFilter;
}

SampleMode SamplerInfo::getMinFilter()
{
    return minFilter;
}

SampleMode SamplerInfo::getMipmapMode()
{
    return mipmapMode;
}

bool SamplerInfo::getAnisotropyEnable()
{
    return anisotropyEnable;
}

bool SamplerInfo::getUnnormalizedCoordinates()
{
    return unnormalizedCoordinates;
}

SamplerInfo SamplerInfo::Builder::build() const
{
    return SamplerInfo(magFilter, minFilter, mipmapMode, anisotropyEnable, unnormalizedCoordinates);
}

Texture::Texture(Format format, uint32_t width, uint32_t height, ImageLayout initialLayout, uint32_t usage)
    : Texture(format, width, height, 1, 1, 1, 1, initialLayout, usage)
{

}

Texture::Texture(Format format, uint32_t width, uint32_t height, ImageLayout initialLayout, uint32_t usage, SamplerInfo samplerInfo)
    : Texture(format, width, height, 1, 1, 1, 1, initialLayout, usage, samplerInfo)
{

}


Texture::Texture(Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels, uint32_t layers, ImageLayout initialLayout, uint32_t usage)
    : Texture(format, width, height, depth, samples, mipLevels, layers, initialLayout, usage, SamplerInfo::Builder().build())
{

}

Texture::Texture(Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels, uint32_t layers, ImageLayout initialLayout, uint32_t usage, SamplerInfo samplerInfo)
    : format(format)
    , width(width)
    , height(height)
    , depth(depth)
    , samples(samples)
    , mipLevels(mipLevels)
    , layers(layers)
    , initialLayout(initialLayout)
    , usage(usage)
    , samplerInfo(samplerInfo)
    , textureLoaded(false)
{

}

void Texture::loadTexture(platform::AssetManager* assetManager, std::string path)
{
    textureLoaded = true;
    assetManager->readImage(path, &memoryBuffer, &width, &height, &mipLevels, mipOffsets);
}

void Texture::setSamplerInfo(SamplerInfo info)
{
    samplerInfo = info;
}
}