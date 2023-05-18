#pragma once
#include "vk_wrapper.h"

namespace vk
{
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData);

void setupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack);

void freeDebugCallback(VkInstance instance);

namespace debug
{
std::string physicalDeviceTypeString(VkPhysicalDeviceType type);
}
}