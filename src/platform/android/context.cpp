#include "platform/android/context.h"
#include "platform/utils.h"

namespace platform
{
AndroidContext::AndroidContext()
    : Context()
    , app(nullptr)
    , platformWindow(nullptr)
{

}

void AndroidContext::init(android_app* app, ANativeWindow* nativeWindow)
{
	this->app = app;
    application = new Application();
	platformWindow = new platform::AndroidWindow(nativeWindow);
    assetManager = new AssetManager(app->activity->assetManager);
	initApplication();
}

void AndroidContext::destroy()
{
    if (assetManager != nullptr)
    {
        delete assetManager;
        assetManager = nullptr;
    }

	if (platformWindow != nullptr)
	{
		delete platformWindow;
		platformWindow = nullptr;
	}

	Context::destroy();
}

void AndroidContext::initApplication()
{
    application->init(platformWindow, assetManager, getInputHandler());
	initialized = true;
}

bool AndroidContext::update()
{
	return updateApplication();
}

void AndroidContext::looper(android_app * app)
{
	bool isFinished = false;
	int events = 0;
	android_poll_source* source = nullptr;

	while (true) {
		int ident;
		while ((ident = ALooper_pollAll(0, nullptr, &events, (void**)&source)) >= 0) {
			if (source) {
				source->process(app, source);
			}
			if (app->destroyRequested) {
				return;
			}
		}

		if (app->userData != nullptr)
		{
			AndroidContext* context = reinterpret_cast<AndroidContext*>(app->userData);
			if (!isFinished && !context->update()) {
				isFinished = true;
				ANativeActivity_finish(app->activity);
			}
		}
	}
}

void AndroidContext::handleMessages(android_app * app, int32_t cmd)
{
    ASSERT(app->userData != NULL);
	
	switch (cmd)
	{
	case APP_CMD_SAVE_STATE:
		LOGD("APP_CMD_SAVE_STATE");
		break;
	case APP_CMD_INIT_WINDOW:
		LOGD("APP_CMD_INIT_WINDOW");

        if (app->userData == nullptr)
        {
			AndroidContext* androidContext = new AndroidContext();
			androidContext->init(app, app->window);
			app->userData = androidContext;
        }
		break;
	case APP_CMD_LOST_FOCUS:
		LOGD("APP_CMD_LOST_FOCUS");
		break;
	case APP_CMD_GAINED_FOCUS:
		LOGD("APP_CMD_GAINED_FOCUS");
		break;
	case APP_CMD_TERM_WINDOW:
		// Window is hidden or closed, clean up resources
		LOGD("APP_CMD_TERM_WINDOW");

		if (app->userData != nullptr)
		{
			AndroidContext* androidContext = reinterpret_cast<AndroidContext*>(app->userData);
			androidContext->destroy();
			delete androidContext;
			app->userData = nullptr;
		}
		break;
	}
}
}