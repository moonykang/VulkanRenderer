#include <fstream>
#include <string>
#include <android/asset_manager.h>

#include "platform/utils.h"
#include "platform/assetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <ktx.h>
#include <ktxvulkan.h>

namespace platform
{
AssetManager::AssetManager(AAssetManager* assetManager)
    : assetManager(assetManager)
{

}

AAssetManager* AssetManager::getAssetManager()
{
    return assetManager;
}

void AssetManager::readFile(std::string path, util::MemoryBuffer* buffer)
{
    AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_STREAMING);
    
    if (!asset) {
        return;
    }

    assert(asset);
    size_t size = AAsset_getLength(asset);
    assert(size > 0);

    buffer->resize(size);
    AAsset_read(asset, buffer->data(), size);
    AAsset_close(asset);
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

    AAsset* file = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_BUFFER);
    size_t fileLength = AAsset_getLength(file);
    stbi_uc* fileContent = new unsigned char[fileLength];
    AAsset_read(file, fileContent, fileLength);
    AAsset_close(file);

    uint32_t imgWidth, imgHeight, n;
    unsigned char* data = stbi_load_from_memory(
            fileContent, fileLength, reinterpret_cast<int*>(&imgWidth),
            reinterpret_cast<int*>(&imgHeight), reinterpret_cast<int*>(&n), 4);

    ASSERT(data);

    *width = imgWidth;
    *height = imgHeight;

    size_t fileSize = (*width) * (*height) * STBI_rgb_alpha;

    buffer->resize(fileSize);
    memcpy(buffer->data(), data, fileSize);
}

void AssetManager::readImageKTX(std::string path, util::MemoryBuffer* buffer, uint32_t* width, uint32_t* height, uint32_t* mipLevels, std::vector<std::pair<uint32_t, size_t>>& mipOffsets)
{
    ktxTexture* ktxTexture;
    AAsset* asset = AAssetManager_open(assetManager, path.c_str(), AASSET_MODE_STREAMING);
    assert(asset);
    size_t size = AAsset_getLength(asset);
    assert(size > 0);
    void *textureData = malloc(size);
    AAsset_read(asset, textureData, size);
    AAsset_close(asset);
    ktxResult result = ktxTexture_CreateFromMemory((ktx_uint8_t *)textureData, size, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &ktxTexture);
    free(textureData);

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
