#include "platform/win/window.h"

namespace platform
{
void WindowWindow::updatePlatformWindow(HWND hWnd, HINSTANCE hInstance)
{
	wnd = hWnd;
	instance = hInstance;
}
}