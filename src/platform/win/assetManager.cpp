#include <fstream>
#include <string>
#include "platform/utils.h"
#include "platform/assetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <ktx.h>
#include <ktxvulkan.h>

namespace platform
{
AssetManager::AssetManager()
{
    
}

void AssetManager::init()
{
}

void AssetManager::destroy()
{
}

std::string AssetManager::getAssetPath()
{
    return std::string(ASSET_PATH);
}

void AssetManager::readFile(std::string path, util::MemoryBuffer* buffer)
{
    std::string assetPath = std::string(ASSET_PATH);

    std::string filename = assetPath + "/" + path;

    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        return;
    }

    size_t fileSize = (size_t)file.tellg();
    buffer->resize(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer->data()), fileSize);

    file.close();
}

void AssetManager::readImage(std::string path, util::MemoryBuffer* buffer, uint32_t* width, uint32_t* height, uint32_t* mipLevels, std::vector<std::pair<uint32_t, size_t>>& mipOffsets)
{
    std::string ktx = "ktx";
    size_t length = path.length();

    if (ktx == path.substr(length - 3, 3))
    {
        readImageKTX(path, buffer, width, height, mipLevels, mipOffsets);
    }
    else
    {
        readImageSTB(path, buffer, width, height, mipLevels, mipOffsets);
    }    
}

void AssetManager::readImageSTB(std::string path, util::MemoryBuffer* buffer, uint32_t* width, uint32_t* height, uint32_t* mipLevels, std::vector<std::pair<uint32_t, size_t>>& mipOffsets)
{
    std::string assetPath = std::string(ASSET_PATH);

    std::string filename = assetPath + "/" + path;

    int texWidth, texHeight, texChannels;
    unsigned char* data = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if(!data) {
        throw std::runtime_error("failed to open image file!");
    }
    ASSERT(data);
    
    // TODO: set channel from Format
    *width = static_cast<uint32_t>(texWidth);
    *height = static_cast<uint32_t>(texHeight);
    *mipLevels = 1;
    mipOffsets.push_back(std::make_pair(0, 0));

    size_t fileSize = (*width) * (*height) * STBI_rgb_alpha;

    buffer->resize(fileSize);
    memcpy(buffer->data(), data, fileSize);
}

void AssetManager::readImageKTX(std::string path, util::MemoryBuffer* buffer, uint32_t* width, uint32_t* height, uint32_t* mipLevels, std::vector<std::pair<uint32_t, size_t>>& mipOffsets)
{
    std::string assetPath = std::string(ASSET_PATH);

    std::string filename = assetPath + "/" + path;

    ktxTexture* ktxTexture;
    ktxResult result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);

    ASSERT(result == KTX_SUCCESS);
    ktx_size_t ktxSize = ktxTexture_GetSize(ktxTexture);
    ktx_uint8_t* ktxImage = ktxTexture_GetData(ktxTexture);

    *width = static_cast<uint32_t>(ktxTexture->baseWidth);
    *height = static_cast<uint32_t>(ktxTexture->baseHeight);
    *mipLevels = static_cast<uint32_t>(ktxTexture->numLevels);

    buffer->resize(ktxSize);
    memcpy(buffer->data(), ktxImage, ktxSize);

    for (uint32_t i = 0; i < *mipLevels; i++)
    {
        ktx_size_t offset;
        KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
        assert(result == KTX_SUCCESS);
        mipOffsets.push_back(std::make_pair(i, offset));
    }

    ktxTexture_Destroy(ktxTexture);
}
}
