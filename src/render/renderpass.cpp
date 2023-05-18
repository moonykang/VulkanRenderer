#include "render/renderpass.h"
#include "platform/assetManager.h"
#include "rhi/context.h"
#include "rhi/rendertarget.h"
#include "rhi/texture.h"

namespace render
{
void Renderpass::destroy(rhi::Context* context)
{
    for (auto& object : objects)
    {
        object->destroy(context);
        delete object;
    }
    objects.clear();

    if (renderTarget != nullptr)
    {
        renderTarget->destroy(context);
        delete renderTarget;
        renderTarget = nullptr;
    }
}

void Renderpass::preBuild(rhi::Context* context)
{
    for (auto& object : objects)
    {
        object->preBuild(context);
    }
}

void Renderpass::build(rhi::Context* context)
{
    ASSERT(renderTarget);
    renderTarget->build(context);

    for (auto& object : objects)
    {
        object->build(context, renderTarget);
    }
}

bool Renderpass::render(rhi::Context* context)
{
    for (auto& transition : beginTransitions)
    {
        renderTarget->addTransition(context , &transition);
    }

    if (!clearColorTextures.empty())
    {
        renderTarget->flushTransition(context);
    }

    for (auto& clearColorTexture : clearColorTextures)
    {
        auto& texture = clearColorTexture.first;
        auto& clearValue = clearColorTexture.second;
        texture->clearColor(context, clearValue.r, clearValue.g, clearValue.b, clearValue.a);
    }

    renderTarget->begin(context);

    for (auto& object : objects)
    {
        object->draw(context);
    }

    renderTarget->end(context);

    for (auto& transition : endTransitions)
    {
        renderTarget->addTransition(context, &transition);
    }

    return true;
}

void Renderpass::addBeginTransition(rhi::Texture* texture, rhi::MemoryAccessFlags accessFlags)
{
    beginTransitions.push_back(rhi::Transition(texture, accessFlags));
}

void Renderpass::addBeginTransition(rhi::StorageBuffer* buffer, rhi::MemoryAccessFlags accessFlags)
{
    beginTransitions.push_back(rhi::Transition(buffer, accessFlags));
}

void Renderpass::addEndTransition(rhi::Texture* texture, rhi::MemoryAccessFlags accessFlags)
{
    endTransitions.push_back(rhi::Transition(texture, accessFlags));
}

void Renderpass::addEndTransition(rhi::StorageBuffer* buffer, rhi::MemoryAccessFlags accessFlags)
{
    endTransitions.push_back(rhi::Transition(buffer, accessFlags));
}

void Renderpass::addClearColorTexture(rhi::Texture* texture, glm::vec4 clearValue)
{
    clearColorTextures.push_back(std::make_pair(texture, clearValue));
}

void Renderpass::setName(std::string name)
{
    this->name = name;
}

rhi::RenderTarget* GraphicsRenderpass::initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height)
{
    renderTarget = context->createRenderTarget(rhi::RenderTargetType::Graphics, width, height);
    return renderTarget;
}

model::GraphicsObject* GraphicsRenderpass::generateObject(rhi::Context* context)
{
    model::GraphicsObject* object = new model::GraphicsObject();
    object->init(context);

    objects.push_back(object);

    return object;
}

model::DerivedGraphicsObject* GraphicsRenderpass::generateDerivedObject(rhi::Context* context, model::GraphicsObject* object)
{
    model::DerivedGraphicsObject* derivedObject = object->derive();
    derivedObject->init(context);
    objects.push_back(derivedObject);

    return derivedObject;
}

rhi::RenderTarget* SurfaceRenderpass::initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height)
{
    renderTarget = context->createRenderTarget(rhi::RenderTargetType::Surface, width, height);
    return renderTarget;
}

model::GraphicsObject* SurfaceRenderpass::generateObject(rhi::Context* context)
{
    model::GraphicsObject* object = new model::GraphicsObject();
    object->init(context);

    objects.push_back(object);

    return object;
}

rhi::RenderTarget* ComputeRenderpass::initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height)
{
    renderTarget = context->createRenderTarget(rhi::RenderTargetType::Compute, width, height);
    return renderTarget;
}

model::ComputeObject* ComputeRenderpass::generateObject(rhi::Context* context)
{
    model::ComputeObject* object = new model::ComputeObject();
    object->init(context);

    objects.push_back(object);

    return object;
}

rhi::RenderTarget* RayTracingRenderpass::initRenderTarget(rhi::Context* context, uint16_t width, uint16_t height)
{
    renderTarget = context->createRenderTarget(rhi::RenderTargetType::RayTracing, width, height);
    return renderTarget;
}

model::RayTracingObject* RayTracingRenderpass::generateObject(rhi::Context* context)
{
    model::RayTracingObject* object = new model::RayTracingObject();
    object->init(context);

    objects.push_back(object);

    return object;
}
}