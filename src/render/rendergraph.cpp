#include "render/renderpass.h"
#include "render/rendergraph.h"
#include "rhi/context.h"

namespace render
{
RenderGraph::RenderGraph()
    : surfaceRenderpass(nullptr)
{
}

void RenderGraph::destroy(rhi::Context* context)
{
    for (auto& renderpass : renderpasses)
    {
        renderpass->destroy(context);
        delete renderpass;
    }
    renderpasses.clear();

    if (surfaceRenderpass != nullptr)
    {
        surfaceRenderpass->destroy(context);
        delete surfaceRenderpass;
        surfaceRenderpass = nullptr;
    }
}

Renderpass* RenderGraph::allocateRenderpass(std::string name, rhi::RenderTargetType type)
{
    Renderpass* renderpass = nullptr;

    switch (type)
    {
    case rhi::RenderTargetType::Graphics:
        renderpass = new GraphicsRenderpass();
        break;
    case rhi::RenderTargetType::Surface:
        renderpass = new SurfaceRenderpass();
        break;
    case rhi::RenderTargetType::Compute:
        renderpass = new ComputeRenderpass();
        break;
    case rhi::RenderTargetType::RayTracing:
        renderpass = new RayTracingRenderpass();
        break;
    default:
        UNREACHABLE();
    }

    renderpass->setName(name);

    if (type == rhi::RenderTargetType::Surface)
    {
        registerSurfaceRenderpass(renderpass);
    }
    else
    {
        registerRenderpass(renderpass);
    }

    return renderpass;
}

void RenderGraph::registerRenderpass(Renderpass* renderpass)
{
    renderpasses.push_back(renderpass);
}

void RenderGraph::registerSurfaceRenderpass(Renderpass* renderpass)
{
    ASSERT(surfaceRenderpass == nullptr);
    surfaceRenderpass = renderpass;
}

void RenderGraph::preBuild(rhi::Context* context)
{
    for (auto& renderpass : renderpasses)
    {
        renderpass->preBuild(context);
    }

    if (surfaceRenderpass != nullptr)
    {
        surfaceRenderpass->preBuild(context);
    }
}

void RenderGraph::build(rhi::Context* context)
{
    for (auto& renderpass : renderpasses)
    {
        renderpass->build(context);
    }

    if (surfaceRenderpass != nullptr)
    {
        surfaceRenderpass->build(context);
    }
}

bool RenderGraph::render(rhi::Context* context)
{
    for (auto& renderpass : renderpasses)
    {
        renderpass->render(context);
    }
    return true;
}

bool RenderGraph::renderSurface(rhi::Context* context)
{
    ASSERT(surfaceRenderpass != nullptr);
    surfaceRenderpass->render(context);
    return true;
}

bool RenderGraph::hasOffscreenRenderPass()
{
    return !renderpasses.empty();
}
}