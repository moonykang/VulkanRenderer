#pragma once

#include "scene/scene.h"
#include "rhi/resources.h"

namespace render
{
	class Renderpass;
}

namespace rhi
{
	class Texture;
	class DescriptorSet;
}

namespace scene
{
class BasicScene : public Scene
{
public:
	BasicScene();

	void initSceneRenderGraph(rhi::Context* contextVk, platform::AssetManager* assetManager) override;

	render::Renderpass* initSurfaceRenderpass(rhi::Context* contextVk, platform::AssetManager* assetManager, rhi::Texture* inputRenderTarget);
};
}