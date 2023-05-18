#include "scene/transition.h"
#include "scene/scene.h"
#include "vulkan/context.h"
#include "platform/assetManager.h"
#include "rhi/context.h"

#if PLATFORM_ANDROID
#include <iostream>
#include <fstream>
#endif //PLATFORM_ANDROID

namespace scene
{
const uint64_t period = 30;
//const uint64_t period = 600;

Transition::Transition()
	: status(Status::Idle)
{
}

void Transition::init(rhi::Context* context, platform::AssetManager* assetManager, bool bReverse)
{
	sceneIterator = scenes.begin();
	if (sceneIterator != scenes.end())
	{
		initScene(context, assetManager);
	}
}

void Transition::destroy(rhi::Context* context)
{
	if (status == Status::Running)
	{
		destroyScene(context);
	}

	for (auto scene : scenes)
	{
		delete scene;
	}
	scenes.clear();
}

bool Transition::update(rhi::Context* context, platform::AssetManager* assetManager, Tick tick)
{
	if (status == Status::Idle)
	{
		LOGD("Compledted %zu tests", scenes.size());
		return false;
	}

	// just update
	updateScene(context, assetManager, tick);
	
	return true;
}

void Transition::registerScene(Scene* scene, const size_t submitCount)
{
	scenes.push_back(scene);
}

void Transition::nextScene(rhi::Context* context, platform::AssetManager* assetManager)
{
	destroyScene(context);
	
	sceneIterator++;

	if (sceneIterator != scenes.end())
	{
		initScene(context, assetManager);
	}
	else
	{
		status = Status::Idle;
	}
}

void Transition::initScene(rhi::Context* context, platform::AssetManager* assetManager)
{
	Scene* scene = *sceneIterator;
	scene->init(context, assetManager);

	status = Status::Begin;
}

void Transition::updateScene(rhi::Context* context, platform::AssetManager* assetManager, Tick tick)
{
	Scene* scene = *sceneIterator;
	scene->update(context, assetManager, tick);

	scene->flush(context, assetManager, tick);

	status = Status::Running;
}

void Transition::destroyScene(rhi::Context* context)
{	
	Scene* scene = *sceneIterator;
	scene->idle(context);
	scene->destroy(context);

	status = Status::End;
}
}