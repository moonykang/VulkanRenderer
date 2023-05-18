#include "application.h"
#include "scene/basicScene.h"
#include "scene/scene.h"
#include "scene/transition.h"
#include "vulkan/context.h"

bool Application::init(platform::Window* window, platform::AssetManager* assetManager, platform::InputHandler* inputHandler)
{
    LOGD("Start application");
    context = new vk::Context();
    
    if (!context->init(window))
    {
        return false;
    }

    sceneTransition = new scene::Transition();

    scene::Scene* scene = new scene::BasicScene();
    sceneTransition->registerScene(scene, 10000);

    const bool bRunReverse = false;
    
    sceneTransition->init(context, assetManager, bRunReverse);

    inputHandler->registerInputAdapter(scene->getSceneViewInputAdapter());

    return true;
}

bool Application::update(platform::AssetManager* assetManager, Tick tick)
{
    return sceneTransition->update(context, assetManager, tick);
}

bool Application::resume()
{
    return true;
}

bool Application::pause()
{
    return true;
}

bool Application::terminate()
{
    sceneTransition->destroy(context);
    context->terminate();
    delete context;
    return true;
}