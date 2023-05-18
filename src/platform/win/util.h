#pragma once

#include <cstdio>
#include <cassert>

#if BUILD_DEBUG
#define LOGD(msg, ...) fprintf(stdout, "[%s:%u] " msg "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGE(msg, ...) fprintf(stderr, "[%s:%u] " msg "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define ASSERT(expression, ...) if (expression) { assert(true); }
    
#else
#define LOGD(msg, ...)
#define LOGE(msg, ...)

#define ASSERT(expression, ...)
#endif
