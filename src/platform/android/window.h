#pragma once

#include <android/native_activity.h>
#include <android/asset_manager.h>
#include <android_native_app_glue.h>
#include "platform/window.h"
#include "rhi/resources.h"

namespace platform
{
class AndroidWindow : public Window
{
public:
    AndroidWindow(ANativeWindow* window);

    ANativeWindow* getWindow();

    uint32_t getWidth();

    uint32_t getHeight();
private:
    ANativeWindow* window;
    uint32_t width;
    uint32_t height;
};
}
