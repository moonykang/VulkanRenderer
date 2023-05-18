#pragma once

#include <string>
#include <vector>
#include "platform/utils.h"
#include "rhi/resources.h"
#include "scene/sceneView.h"
#include "scene/sceneLight.h"

namespace platform
{
    class AssetManager;
}

namespace render
{
    class RenderGraph;
}

namespace rhi
{
class Context;
class UniformBuffer;
class Texture;
class ScratchBuffer;
class AccStructureManager;
class StorageBuffer;
}

namespace model
{
    class Object;
}

namespace scene
{

class Scene
{
public:
    Scene();

    ~Scene();

    bool init(rhi::Context* context, platform::AssetManager* assetManager);

    virtual void preInit(rhi::Context* context, platform::AssetManager* assetManager);

    virtual void initSceneRenderGraph(rhi::Context* context, platform::AssetManager* assetManager);

    virtual void preBuild(rhi::Context* context, platform::AssetManager* assetManager);

    virtual void build(rhi::Context* context, platform::AssetManager* assetManager);

    virtual void postBuild(rhi::Context* context, platform::AssetManager* assetManager);

    virtual void update(rhi::Context* context, platform::AssetManager* assetManager, Tick tick);

    virtual void flush(rhi::Context* context, platform::AssetManager* assetManager, Tick tick);

    virtual void updateSceneObjects(rhi::Context* context, Tick tick) {}

    virtual void idle(rhi::Context* context);

    virtual void destroy(rhi::Context* context);

    virtual void initSceneUniformBuffers(rhi::Context* context);

    SceneViewInputAdapter* getSceneViewInputAdapter()
    {
        return sceneView.getSceneViewInputAdapter();
    }
public:
    virtual void updateSceneUniformBuffers(rhi::Context* context, Tick tick);

    rhi::Texture* allocateSceneTexture(rhi::Context* context, rhi::Format format, uint32_t width, uint32_t height, rhi::ImageLayout initialLayout, uint32_t usage);

    rhi::Texture* allocateSceneTexture(rhi::Context* context, rhi::Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels, uint32_t layers,
                                       rhi::ImageLayout initialLayout, uint32_t usage);

    rhi::StorageBuffer* allocateSceneStorageBuffer(rhi::Context* context, uint32_t size, rhi::BufferUsageFlags usage);

    void registerObject(rhi::Context* context, model::Object* object);

protected:
    render::RenderGraph* renderGraph;
    std::vector<rhi::Texture*> sceneTextures;
    std::vector<rhi::StorageBuffer*> storageBuffers;

    rhi::BufferUsageFlags scratchBufferUsage;
    rhi::ScratchBuffer* vertexScratchBuffer;
    rhi::ScratchBuffer* indexScratchBuffer;
protected:
    struct SceneUniformBufferObject {
        ALIGNED(16)
        glm::mat4 view_inverse;
        ALIGNED(16)
        glm::mat4 proj_inverse;
        ALIGNED(16)
        glm::mat4 view_proj_inverse;
        ALIGNED(16)
        glm::mat4 prev_view_proj;
        ALIGNED(16)
        glm::mat4 view_proj;
        ALIGNED(16)
        glm::vec4 view_pos;
        ALIGNED(16)
        glm::vec4 current_prev_jitter;
        ALIGNED(32)
        Light scenLight;
        ALIGNED(4)
        uint32_t num_frames;
        uint32_t inverse_scale;
    };

    SceneView sceneView;
    SceneLight sceneLight;

    SceneUniformBufferObject sceneUniformBufferObject;
    rhi::UniformBuffer* sceneUniformBuffer;

    bool enableRayTracing;
    bool enableScratchBuffer;
    rhi::AccStructureManager* accStructureManager;
};
}