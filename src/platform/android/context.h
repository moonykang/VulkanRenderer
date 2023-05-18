#pragma once

#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>
#include "platform/context.h"
#include "platform/android/window.h"

namespace platform
{
class AndroidContext : public Context
{
public:
	AndroidContext();

	void init(android_app* app, ANativeWindow* nativeWindow);
	
	void destroy() override;

	void initApplication();

	bool update();

    static void handleMessages(android_app * app, int32_t cmd);

    static void looper(android_app * app);
private:
    android_app* app;
    platform::AndroidWindow* platformWindow;
};
}