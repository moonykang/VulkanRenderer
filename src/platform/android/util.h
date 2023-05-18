#pragma once

#include <cstdio>
#include <cassert>
#include <android/log.h>

#if BUILD_DEBUG
#define TAG "LittleRenderer"
#define LOGD(msg, ...) __android_log_print(ANDROID_LOG_DEBUG, TAG, "[%s:%u] " msg "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGE(msg, ...) __android_log_print(ANDROID_LOG_ERROR, TAG, "[%s:%u] " msg "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ASSERT(expression, ...) \
        assert(true);           \
    
#else
#define LOGD(msg, ...)
#define LOGE(msg, ...)

#define ASSERT(expression, ...)
#endif
