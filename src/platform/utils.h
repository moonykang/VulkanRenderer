#pragma once

#include <stdexcept>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#if PLATFORM_WINDOW
#include "platform/win/util.h"
#else
#include "platform/android/util.h"
#endif

#define EMPTY_STRUCT {}

#define UNREACHABLE() ASSERT(true)

#if __has_cpp_attribute(nodiscard)
#    define NO_DISCARD [[nodiscard]]
#else
#    define NO_DISCARD
#endif  // __has_cpp_attribute(nodiscard)

class NonCopyable
{
  protected:
    constexpr NonCopyable() = default;
    ~NonCopyable()          = default;

  private:
    NonCopyable(const NonCopyable &) = delete;
    void operator=(const NonCopyable &) = delete;
};

namespace Util
{
inline uint64_t align(uint64_t size, uint64_t granularity)
{
    const auto divUp = (size + granularity - 1) / granularity;
    return divUp * granularity;
}
}
typedef uint64_t Tick;

#if defined(_MSC_VER)
#    define ALIGNED(x) __declspec(align(x))
#else
#    if defined(__GNUC__) || defined(__clang__)
#        define ALIGNED(x) __attribute__((aligned(x)))
#    endif
#endif