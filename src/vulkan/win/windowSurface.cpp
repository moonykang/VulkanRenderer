#include "platform/win/window.h"
#include "vulkan/surface.h"
#include "vulkan/vk_headers.h"

namespace vk
{
bool Surface::initSurface(VkInstance instance, platform::Window* window)
{
    platform::WindowWindow* platformWindow = reinterpret_cast<platform::WindowWindow*>(window);
    HWND hWnd = platformWindow->getWindow();
    HINSTANCE hInstance = platformWindow->getInstance();
    
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.hinstance = hInstance;
    surfaceCreateInfo.hwnd = hWnd;
	VKCALL(vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));

    updateSurfaceSize(platformWindow->getWidth(), platformWindow->getHeight());

    return true;
}
}