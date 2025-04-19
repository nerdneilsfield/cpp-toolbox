#pragma once

#include <cstdio>

// Use C++20 endian check if available, otherwise fallback (e.g., linux
// specific)
#if __cplusplus >= 202002L && defined(__has_include)
#  if __has_include(<bit>)
#    include <bit>
#    define CPP_TOOLBOX_NATIVE_ENDIAN std::endian::native
#    define CPP_TOOLBOX_LITTLE_ENDIAN std::endian::little
#    define CPP_TOOLBOX_BIG_ENDIAN std::endian::big
#  else
// Fallback or error if <bit> not found despite C++20
#    include <endian.h>  // Requires glibc or similar
#    define CPP_TOOLBOX_NATIVE_ENDIAN \
      (__BYTE_ORDER == __LITTLE_ENDIAN ? std::endian::little : std::endian::big)
#    define CPP_TOOLBOX_LITTLE_ENDIAN std::endian::little
#    define CPP_TOOLBOX_BIG_ENDIAN std::endian::big
// Need to define std::endian enum values if <bit> isn't available
namespace std
{
enum class endian
{
  little,
  big,
  native = CPP_TOOLBOX_NATIVE_ENDIAN
};
}  // namespace std

#  endif
#else
// Fallback for pre-C++20 (assuming little-endian for common platforms, or use
// compile defs)
#  include <endian.h>  // Requires glibc or similar
#  define CPP_TOOLBOX_NATIVE_ENDIAN \
    (__BYTE_ORDER == __LITTLE_ENDIAN ? 1 : 0)  // 1 for little, 0 for big approx
#  define CPP_TOOLBOX_LITTLE_ENDIAN 1
#  define CPP_TOOLBOX_BIG_ENDIAN 0
// Define dummy std::endian if needed for compatibility below
namespace std
{
enum class endian
{
  little,
  big,
  native
};
}  // namespace std
constexpr std::endian determine_native_endian()
{
#  ifdef _WIN32  // Windows is typically little-endian
  return std::endian::little;
#  elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return std::endian::little;
#  elif defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return std::endian::big;
#  else  // Default assumption or require specific flag
  return std::endian::little;
#  endif
}
constexpr std::endian native_endian_value = determine_native_endian();
#  undef CPP_TOOLBOX_NATIVE_ENDIAN
#  define CPP_TOOLBOX_NATIVE_ENDIAN native_endian_value

#endif