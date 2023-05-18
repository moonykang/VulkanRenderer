#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <ShellScalingAPI.h>
#include <iostream>
#include <cstdio>

#include "platform/win/context.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
#if BUILD_DEBUG
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
#endif

	long width = 1024;
	long height = 1024;

	platform::WindowContext context;
	context.init(hInstance, width, height);
	context.update();
	context.destroy();

    return 0;
}