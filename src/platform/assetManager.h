#pragma once

#include <string>
#include <vector>
#include "platform/memorybuffer.h"


#if PLATFORM_ANDROID
class AAssetManager;
#endif

namespace data
{
class GltfLoader;
class GltfInstance;
}
namespace platform
{
class AssetManager
{
public:
    AssetManager();
#if PLATFORM_ANDROID
    AssetManager(AAssetManager* assetManager);
#endif

#if PLATFORM_WINDOW
    void init();
    void destroy();
    std::string getAssetPath();
#endif
    void readFile(std::string path, util::MemoryBuffer* buffer);
    void readImage(std::string path, util::MemoryBuffer* buffer, uint32_t* width, uint32_t* height, uint32_t* mipLevels, std::vector<std::pair<uint32_t, size_t>>& mipOffsets);
    void readImageSTB(std::string path, util::MemoryBuffer* buffer, uint32_t* width, uint32_t* height, uint32_t* mipLevels, std::vector<std::pair<uint32_t, size_t>>& mipOffsets);
    void readImageKTX(std::string path, util::MemoryBuffer* buffer, uint32_t* width, uint32_t* height, uint32_t* mipLevels, std::vector<std::pair<uint32_t, size_t>>& mipOffsets);

#if PLATFORM_ANDROID
    AAssetManager* getAssetManager();
#endif
private:
#if PLATFORM_ANDROID
    AAssetManager* assetManager;
#endif
};
}