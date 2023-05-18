#pragma once

#include <vector>
#include "platform/utils.h"

namespace platform
{
class AssetManager;
}

namespace rhi
{
class Context;
}

namespace scene
{
class Scene;
class Metric;

enum class Status
{
	Idle,
	Begin,
	Running,
	End
};

class Transition
{
public:
	Transition();

	void init(rhi::Context* context, platform::AssetManager* assetManager, bool bReverse = false);

	void destroy(rhi::Context* context);

	bool update(rhi::Context* context, platform::AssetManager* assetManager, Tick tick);

	void registerScene(Scene* scene, const size_t submitCount);

	void nextScene(rhi::Context* context, platform::AssetManager* assetManager);

	void initScene(rhi::Context* context, platform::AssetManager* assetManager);

	void updateScene(rhi::Context* context, platform::AssetManager* assetManager, Tick tick);

	void destroyScene(rhi::Context* context);
private:
	std::vector<Scene*> scenes;
	std::vector<Scene*>::iterator sceneIterator;
	Status status;
};
}