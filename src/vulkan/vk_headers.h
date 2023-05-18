#pragma once

#include "platform/utils.h"

#if PLATFORM_ANDROID
#include "vulkan/android/vulkan_wrapper.h"
#else
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_core.h"
#include "vulkan/win/vulkan_wrapper.h"
#endif

#define VKCALL(expr)			\
	if ((expr) != VK_SUCCESS)	\
	{							\
		ASSERT(true)			\
	}
	