#include "platform/android/window.h"
#include "vulkan/surface.h"
#include "vulkan/vk_headers.h"

namespace vk
{

bool Surface::initSurface(VkInstance instance, platform::Window* window)
{
    platform::AndroidWindow* androidWindow = reinterpret_cast<platform::AndroidWindow*>(window);

    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.window = androidWindow->getWindow();

    VKCALL(vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface));
    updateSurfaceSize(androidWindow->getWidth(), androidWindow->getHeight());

    return true;
}
}