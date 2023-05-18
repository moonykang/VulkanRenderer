#pragma once

#include <vector>
#include <string>

namespace rhi
{
class Context;
}

namespace render
{
class Renderpass;
class RenderGraph
{
public:
    RenderGraph();

    void destroy(rhi::Context* context);

    Renderpass* allocateRenderpass(std::string name, rhi::RenderTargetType type);

    void registerRenderpass(Renderpass* renderpass);

    void registerSurfaceRenderpass(Renderpass* renderpass);

    void preBuild(rhi::Context* context);

    void build(rhi::Context* context);

    bool render(rhi::Context* context);

    bool renderSurface(rhi::Context* context);

    bool hasOffscreenRenderPass();
private:
    std::vector<Renderpass*> renderpasses;
    Renderpass* surfaceRenderpass;
};
}