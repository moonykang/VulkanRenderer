#pragma once

#include "platform/context.h"
#include "platform/win/window.h"
namespace platform
{
class WindowContext : public Context
{
public:
	WindowContext();

	void init(HINSTANCE instance, long width, long height);
	
	void destroy() override;

	void initApplication();

	void update();

	void setupWindow(HINSTANCE instance, long width, long height);

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND setupPlatformWindow(HINSTANCE hinstance, WNDPROC wndproc, long width, long height);

	static void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void messageLoop(HWND window);
private:
	platform::WindowWindow* platformWindow;
};
}