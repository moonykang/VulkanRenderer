#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include <sys/system_properties.h>

#include "platform/android/context.h"

void android_main(android_app *app)
{
    app->userData = nullptr;
    app->onAppCmd = platform::AndroidContext::handleMessages;
    platform::AndroidContext::looper(app);
}