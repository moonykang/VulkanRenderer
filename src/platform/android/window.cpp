#include <android/native_window.h>
#include "platform/android/window.h"
#include "platform/utils.h"

namespace platform
{
AndroidWindow::AndroidWindow(ANativeWindow* window)
    : Window()
    , window(window)
{
    ASSERT(window);

    width = static_cast<uint32_t>(ANativeWindow_getWidth(window));
    height = static_cast<uint32_t>(ANativeWindow_getHeight(window));
}

ANativeWindow* AndroidWindow::getWindow()
{
    return window;
}

uint32_t AndroidWindow::getWidth()
{
    return width;
}

uint32_t AndroidWindow::getHeight()
{
    return height;
}
}
