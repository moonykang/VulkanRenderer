#pragma once

#include "platform/utils.h"

namespace platform
{
class AssetManager;
class Window;
class InputHandler;
}

namespace rhi
{
class Context;
}

namespace scene
{
class Scene;
class Transition;
}

class Application
{
public:
    bool init(platform::Window* window, platform::AssetManager* assetManager, platform::InputHandler* inputHandler);
    bool update(platform::AssetManager* assetManager, Tick tick);
    bool resume();
    bool pause();
    bool terminate();

private:
    rhi::Context* context;
    scene::Scene* scene;

    scene::Transition* sceneTransition;
};