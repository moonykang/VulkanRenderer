#include "platform/assetManager.h"
#include "scene/scene.h"
#include "render/rendergraph.h"
#include "rhi/context.h"
#include "rhi/buffer.h"
#include "rhi/texture.h"
#include "rhi/accelerationStructure.h"
#include "vulkan/context.h"
#include "model/object.h"

namespace scene
{
Scene::Scene()
    : renderGraph(nullptr)
    , sceneUniformBufferObject()
    , sceneUniformBuffer(nullptr)
    , vertexScratchBuffer(nullptr)
    , indexScratchBuffer(nullptr)
    , scratchBufferUsage()
    , accStructureManager(nullptr)
    , enableRayTracing(false)
    , enableScratchBuffer(false)
{
}

Scene::~Scene()
{
}

bool Scene::init(rhi::Context* context, platform::AssetManager* assetManager)
{
    preInit(context, assetManager);

    initSceneRenderGraph(context, assetManager);

    preBuild(context, assetManager);

    build(context, assetManager);

    postBuild(context, assetManager);

    return true;
}

void Scene::preInit(rhi::Context* context, platform::AssetManager* assetManager)
{
    renderGraph = new render::RenderGraph();

    if (enableRayTracing)
    {
        scratchBufferUsage = rhi::BufferUsage::BUFFER_STORAGE_BUFFER
            | rhi::BufferUsage::BUFFER_SHADER_DEVICE_ADDRESS
            | rhi::BufferUsage::BUFFER_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY;

        accStructureManager = context->createAccStructureManager();
    }

    if (enableScratchBuffer)
    {
        vertexScratchBuffer = context->createScratchBuffer(rhi::BufferUsage::BUFFER_VERTEX_BUFFER | scratchBufferUsage);
        indexScratchBuffer = context->createScratchBuffer(rhi::BufferUsage::BUFFER_INDEX_BUFFER | scratchBufferUsage);
    }

    initSceneUniformBuffers(context);
}

void Scene::initSceneRenderGraph(rhi::Context* context, platform::AssetManager* assetManager) {}

void Scene::build(rhi::Context* context, platform::AssetManager* assetManager) {}

void Scene::preBuild(rhi::Context* context, platform::AssetManager* assetManager)
{
    sceneUniformBuffer->build(context);

    for (auto& texture : sceneTextures)
    {
        texture->build(context);
    }

    for (auto& storageBuffer : storageBuffers)
    {
        storageBuffer->build(context);
    }

    if (enableScratchBuffer)
    {
        vertexScratchBuffer->build(context);
        indexScratchBuffer->build(context);
    }
    renderGraph->preBuild(context);
}

void Scene::postBuild(rhi::Context* context, platform::AssetManager* assetManager)
{
    if (enableRayTracing)
    {
        accStructureManager->preBuildBottomLevelAccStructure(context);
        accStructureManager->buildBottomLevelAccStructure(context);
        accStructureManager->preBuildTopLevelAccStructure(context);
        accStructureManager->buildTopLevelAccStructure(context);
    }

    renderGraph->build(context);
}

void Scene::update(rhi::Context* context, platform::AssetManager* assetManager, Tick tick)
{
    sceneView.update(tick);

    updateSceneObjects(context, tick);
    updateSceneUniformBuffers(context, tick);
    renderGraph->render(context);
    renderGraph->renderSurface(context);
}

void Scene::flush(rhi::Context* context, platform::AssetManager* assetManager, Tick tick)
{
    context->present();
}

void Scene::idle(rhi::Context* context)
{
    context->wait();
}

void Scene::destroy(rhi::Context* context)
{
    context->wait();

    if (accStructureManager != nullptr)
    {
        accStructureManager->destroy(context);
        delete accStructureManager;
        accStructureManager = nullptr;
    }

    if (sceneUniformBuffer != nullptr)
    {
        sceneUniformBuffer->destroy(context);
        delete sceneUniformBuffer;
        sceneUniformBuffer = nullptr;
    }

    for (auto& texture : sceneTextures)
    {
        texture->destroy(context);
        delete texture;
    }
    sceneTextures.clear();

    for (auto& storageBuffer : storageBuffers)
    {
        storageBuffer->destroy(context);
        delete storageBuffer;
    }
    storageBuffers.clear();

    if (vertexScratchBuffer != nullptr)
    {
        vertexScratchBuffer->destroy(context);
        delete vertexScratchBuffer;
        vertexScratchBuffer = nullptr;
    }

    if (indexScratchBuffer != nullptr)
    {
        indexScratchBuffer->destroy(context);
        delete indexScratchBuffer;
        indexScratchBuffer = nullptr;
    }

    if (renderGraph != nullptr)
    {
        renderGraph->destroy(context);
        delete renderGraph;
    }
}

rhi::Texture* Scene::allocateSceneTexture(rhi::Context* context, rhi::Format format, uint32_t width, uint32_t height, rhi::ImageLayout initialLayout, uint32_t usage)
{
    rhi::Texture* texture = context->createTexture(format, width, height, initialLayout, usage);
    sceneTextures.push_back(texture);
    return texture;
}

rhi::Texture* Scene::allocateSceneTexture(rhi::Context* context, rhi::Format format, uint32_t width, uint32_t height, uint32_t depth, uint32_t samples, uint32_t mipLevels,
                                          uint32_t layers, rhi::ImageLayout initialLayout, uint32_t usage)
{
    rhi::Texture* texture = context->createTexture(format, width, height, depth, samples, mipLevels, layers, initialLayout, usage);
    sceneTextures.push_back(texture);
    return texture;
}

rhi::StorageBuffer* Scene::allocateSceneStorageBuffer(rhi::Context* context, uint32_t size, rhi::BufferUsageFlags usage)
{
    rhi::StorageBuffer* storageBuffer = context->createStorageBuffer(rhi::BufferType::DeviceLocal, usage);
    storageBuffer->getBuffer()->resize(size);
    storageBuffers.push_back(storageBuffer);
    return storageBuffer;
}

void Scene::registerObject(rhi::Context* context, model::Object* object)
{
    if (enableScratchBuffer)
    {
        object->getVertexBuffer()->suballocate(vertexScratchBuffer);
        object->getIndexBuffer()->suballocate(indexScratchBuffer);
    }

    if (enableRayTracing)
    {
        accStructureManager->registerModel(context, object);
    }
}

void Scene::initSceneUniformBuffers(rhi::Context* context)
{
    sceneView.type = SceneView::CameraType::firstperson;

    sceneView.setPerspective(60.0f, (float) context->getWidth() / (float) context->getHeight(), 0.1f, 64.0f);
    sceneView.setRotation(glm::vec3(0.0f, 90.f, 0.f));
    sceneView.setTranslation(glm::vec3(1.0f, 0.75f, 0.f));

    sceneLight.setTranslation(glm::vec3(4.f, 0.5f, -1.5f));
    sceneLight.lightRadius = 0.08f;
    sceneLight.lightIntensity = 10.0f;
    sceneLight.setRotation(glm::vec3(-10.f, 0.f, 30.f));
    sceneUniformBufferObject.num_frames = 0;
    sceneUniformBufferObject.inverse_scale = 1;

    sceneUniformBuffer = context->createUniformBuffer(rhi::BufferType::HostCoherent);
    sceneUniformBuffer->set<SceneUniformBufferObject>(1, &sceneUniformBufferObject);
}

void Scene::updateSceneUniformBuffers(rhi::Context* context, Tick tick)
{
    glm::mat4& view = sceneView.matrices.view;
    glm::mat4& proj = sceneView.matrices.perspective;

    sceneUniformBufferObject.view_inverse = glm::inverse(view);
    sceneUniformBufferObject.proj_inverse = glm::inverse(proj);
    sceneUniformBufferObject.view_proj = proj * view;
    sceneUniformBufferObject.view_proj_inverse = glm::inverse(sceneUniformBufferObject.view_proj);
    sceneUniformBufferObject.prev_view_proj = glm::mat4(1.f);

    sceneUniformBufferObject.view_pos = sceneView.viewPos;
    sceneUniformBufferObject.current_prev_jitter = glm::vec4(0.f);

    sceneLight.rotate(glm::vec3(0.02f, 0.02f, 0.0f));
    sceneLight.updateLight(sceneUniformBufferObject.scenLight);

    sceneUniformBufferObject.num_frames++;
    sceneUniformBuffer->set<SceneUniformBufferObject>(1, &sceneUniformBufferObject);
    sceneUniformBuffer->update(context);
}
}