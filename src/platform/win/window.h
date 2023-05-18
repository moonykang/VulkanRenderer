#pragma once

#include <windows.h>
#include "platform/window.h"
#include "rhi/resources.h"

namespace platform
{
class WindowWindow : public Window
{
public:
    void updatePlatformWindow(HWND hWnd, HINSTANCE hInstance);
    HWND getWindow() { return wnd; }
    HINSTANCE getInstance() { return instance; }
private:
    HWND wnd;
    HINSTANCE instance;
};
}
