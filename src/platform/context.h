#pragma once

#include "application/application.h"
#include "platform/assetManager.h"
#include "platform/utils.h"
#include "platform/window.h"
#include "platform/inputHandler.h"
#include <chrono>

namespace platform
{
class Context
{
public:
	Context();

	virtual void destroy();

	void updateTick();

	bool updateApplication();

	static InputHandler* getInputHandler();

protected:
	bool initialized;
	Application* application;
    AssetManager* assetManager;	
	Tick tick;

private:
	static InputHandler inputHandler;
};
}