#include "platform/context.h"

namespace platform
{

InputHandler Context::inputHandler;
Context::Context()
    : initialized(false)
    , application(nullptr)
    , assetManager(nullptr)
    , tick(0)
{
}

void Context::destroy()
{
    if (application != nullptr)
    {
        application->terminate();
        delete application;
        application = nullptr;
    }
}

void Context::updateTick()
{
    tick++;
}

bool Context::updateApplication()
{
    updateTick();
    return application->update(assetManager, tick);
}

InputHandler* Context::getInputHandler()
{
    return &inputHandler;
}
}