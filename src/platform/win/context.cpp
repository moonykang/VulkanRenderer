#include "platform/win/context.h"

#define KEY_ESCAPE VK_ESCAPE 
#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_F3 VK_F3
#define KEY_F4 VK_F4
#define KEY_F5 VK_F5

#define KEY_A 0x41
#define KEY_B 0x42
#define KEY_C 0x43
#define KEY_D 0x44
#define KEY_E 0x45
#define KEY_F 0x46
#define KEY_G 0x47
#define KEY_H 0x48
#define KEY_I 0x49
#define KEY_J 0x4A
#define KEY_K 0x4B
#define KEY_L 0x4C
#define KEY_M 0x4D
#define KEY_N 0x4E
#define KEY_O 0x4F
#define KEY_P 0x50
#define KEY_Q 0x51
#define KEY_R 0x52
#define KEY_S 0x53
#define KEY_T 0x54
#define KEY_U 0x55
#define KEY_V 0x56
#define KEY_W 0x57
#define KEY_X 0x58
#define KEY_Y 0x59
#define KEY_Z 0x5A

#define KEY_SPACE 0x20
#define KEY_KPADD 0x6B
#define KEY_KPSUB 0x6D

namespace platform
{
WindowContext::WindowContext()
	: Context()
	, platformWindow(nullptr)
{
}

void WindowContext::init(HINSTANCE instance, long width, long height)
{
	application = new Application();
	platformWindow = new WindowWindow();
    assetManager = new AssetManager();
	assetManager->init();
	setupWindow(instance, width, height);
	initApplication();
}

void WindowContext::destroy()
{
    if (assetManager != nullptr)
    {
		assetManager->destroy();
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

void WindowContext::initApplication()
{
	application->init(platformWindow, assetManager, getInputHandler());
}

void WindowContext::update()
{
	messageLoop(platformWindow->getWindow());
}

void WindowContext::setupWindow(HINSTANCE instance, long width, long height)
{
	HWND window = setupPlatformWindow(instance, WndProc, width, height);
	platformWindow->updatePlatformWindow(window, instance);
	platformWindow->updateWindowSize(width, height);
}

LRESULT CALLBACK WindowContext::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	handleMessages(hWnd, uMsg, wParam, lParam);
	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

HWND WindowContext::setupPlatformWindow(HINSTANCE hinstance, WNDPROC wndproc, long width, long height)
{
	//this->windowInstance = hinstance;

	bool fullscreen = false;
	std::string name = "Little Renderer";


	WNDCLASSEX wndClass;
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wndproc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hinstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = name.c_str();
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		LOGE("Could not register window class!");
		exit(1);
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen)
	{
		if ((width != (uint32_t)screenWidth) && (height != (uint32_t)screenHeight))
		{
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPelsWidth = width;
			dmScreenSettings.dmPelsHeight = height;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, "Fullscreen Mode not supported!\n Switch to window mode?", "Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					fullscreen = false;
				}
				else
				{
					return nullptr;
				}
			}
			screenWidth = width;
			screenHeight = height;
		}

	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}

	RECT windowRect;
	windowRect.left = 0L;
	windowRect.top = 0L;
	windowRect.right = fullscreen ? (long)screenWidth : (long)width;
	windowRect.bottom = fullscreen ? (long)screenHeight : (long)height;

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	std::string windowTitle = name;
	HWND window = CreateWindowEx(0,
		name.c_str(),
		windowTitle.c_str(),
		dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		0,
		0,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		hinstance,
		NULL);

	if (!fullscreen)
	{
		// Center on screen
		uint32_t x = (GetSystemMetrics(SM_CXSCREEN) - windowRect.right) / 2;
		uint32_t y = (GetSystemMetrics(SM_CYSCREEN) - windowRect.bottom) / 2;
		SetWindowPos(window, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}

	if (!window)
	{
		LOGE("Could not create window!");
		return nullptr;
	}

	ShowWindow(GetConsoleWindow(), SW_SHOW);
	ShowWindow(window, SW_SHOW);
	SetForegroundWindow(window);
	SetFocus(window);

	return window;
}

void WindowContext::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case KEY_W:
			getInputHandler()->handleKeyForward(true);
			break;
		case KEY_S:
			getInputHandler()->handleKeyBackward(true);
			break;
		case KEY_A:
			getInputHandler()->handleKeyLeft(true);
			break;
		case KEY_D:
			getInputHandler()->handleKeyRight(true);
			break;
		}
		break;
	case WM_KEYUP:
		switch (wParam)
		{
		case KEY_W:
			getInputHandler()->handleKeyForward(false);
			break;
		case KEY_S:
			getInputHandler()->handleKeyBackward(false);
			break;
		case KEY_A:
			getInputHandler()->handleKeyLeft(false);
			break;
		case KEY_D:
			getInputHandler()->handleKeyRight(false);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		getInputHandler()->handleMouseLButton(true, (float)LOWORD(lParam), (float)HIWORD(lParam));
		break;
	case WM_RBUTTONDOWN:
		getInputHandler()->handleMouseRButton(true, (float)LOWORD(lParam), (float)HIWORD(lParam));
		break;
	case WM_MBUTTONDOWN:
		getInputHandler()->handleMouseMButton(true, (float)LOWORD(lParam), (float)HIWORD(lParam));
		break;
	case WM_LBUTTONUP:
		getInputHandler()->handleMouseLButton(false, 0.f, 0.f);
		break;
	case WM_RBUTTONUP:
		getInputHandler()->handleMouseRButton(false, 0.f, 0.f);
		break;
	case WM_MBUTTONUP:
		getInputHandler()->handleMouseMButton(false, 0.f, 0.f);
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_MOUSEMOVE:
		getInputHandler()->handleMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_SIZE:
		break;
	case WM_GETMINMAXINFO:
		break;
	case WM_ENTERSIZEMOVE:
		break;
	case WM_EXITSIZEMOVE:
		break;
	}
}

void WindowContext::messageLoop(HWND window)
{
	MSG msg;
	memset(&msg, 0, sizeof(msg));
	bool quitMessageReceived = false;

	auto before = std::chrono::system_clock::now();

	// control fps print frqency, 60 means every 60 frame print the log
	const int fpsCountMax = 60;

	while (!quitMessageReceived)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				quitMessageReceived = true;
				break;
			}
		}

		// fps print, disabled by default
		// TODO - change to pre-define feature
		if (false)
		{
			static int fpsCount = 0;
			fpsCount++;
			if (fpsCount == fpsCountMax) {
				fpsCount = 0;
				auto now = std::chrono::system_clock::now();
				std::chrono::duration<double> elapsed_seconds = now - before;
				before = now;
				auto milisec = elapsed_seconds.count() * 1000.0f;
				LOGE("fps : %6.2lf, frame time: %6.2lf", 1000.0f / milisec * fpsCountMax, milisec / fpsCountMax);
			}
		}

		if (!updateApplication())
		{
			PostQuitMessage(0);
		}
	}
}
}