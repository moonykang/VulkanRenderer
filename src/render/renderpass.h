#pragma once

#include <vector>
#include "platform/utils.h"
#include "rhi/resources.h"
#include "rhi/transition.h"
#include "model/object.h"

namespace platform
{
    class AssetManager;
}

namespace rhi
{
class Context;
class RenderTarget;
class Texture;
}

namespace render
{
class Renderpass
{
public:
    void destroy(rhi::Context* context);

    virtual rhi::RenderTarget* initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height) = 0;

    void preBuild(rhi::Context* context);

    void build(rhi::Context* context);

    virtual model::Object* generateObject(rhi::Context* context) = 0;

    void addBeginTransition(rhi::Texture* texture, rhi::MemoryAccessFlags accessFlags);

    void addBeginTransition(rhi::StorageBuffer* storageBuffer, rhi::MemoryAccessFlags accessFlags);

    void addEndTransition(rhi::Texture* texture, rhi::MemoryAccessFlags accessFlags);

    void addEndTransition(rhi::StorageBuffer* storageBuffer, rhi::MemoryAccessFlags accessFlags);

    void addClearColorTexture(rhi::Texture* texture, glm::vec4 clearValue);

    void setName(std::string name);
public:
    bool render(rhi::Context* context);

private:
    std::string name;
    std::vector<rhi::Transition> beginTransitions;
    std::vector<rhi::Transition> endTransitions;
    std::vector<std::pair<rhi::Texture*, glm::vec4>> clearColorTextures;
protected:
    rhi::RenderTarget* renderTarget;
    std::vector<model::Object*> objects;
};

class GraphicsRenderpass : public Renderpass
{
public:
    rhi::RenderTarget* initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height) override;

    model::GraphicsObject* generateObject(rhi::Context* context) override;

    model::DerivedGraphicsObject* generateDerivedObject(rhi::Context* context, model::GraphicsObject* object);
};

class SurfaceRenderpass : public Renderpass
{
public:
    rhi::RenderTarget* initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height) override;

    model::GraphicsObject* generateObject(rhi::Context* context) override;
};

class ComputeRenderpass : public Renderpass
{
public:
    rhi::RenderTarget* initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height) override;

    model::ComputeObject* generateObject(rhi::Context* context) override;
};

class RayTracingRenderpass : public Renderpass
{
public:
    rhi::RenderTarget* initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height) override;

    model::RayTracingObject* generateObject(rhi::Context* context) override;
};
}