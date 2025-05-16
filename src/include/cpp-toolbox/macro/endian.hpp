#pragma once

#include <cstdio>

// Use C++20 endian check if available, otherwise fallback (e.g., linux
// specific)
#if __cplusplus >= 202002L && defined(__has_include) && __has_include(<bit>)
// --- C++20 Standard Way (Ideal) ---
#  include <bit>
#  define CPP_TOOLBOX_ENDIAN_NAMESPACE std
// Define helper macros based on standard enum
constexpr bool CPP_TOOLBOX_NATIVE_IS_LITTLE =
    (std::endian::native == std::endian::little);
constexpr bool CPP_TOOLBOX_NATIVE_IS_BIG =
    (std::endian::native == std::endian::big);

#else
// --- Pre-C++20 Fallback ---

// Define the enum class locally if std::endian isn't available
// We primarily need the values for comparison logic below.
namespace cpp_toolbox::detail
{
enum class endian
{
  little,
  big
};
}  // namespace cpp_toolbox::detail
#  define CPP_TOOLBOX_ENDIAN_NAMESPACE cpp_toolbox::detail

#  if defined(_WIN32) || defined(_WIN64)
   // Windows (MSVC, MinGW) is almost always little-endian on supported
   // architectures (x86, x64, ARM)
#    define CPP_TOOLBOX_NATIVE_ENDIAN_ENUM cpp_toolbox::detail::endian::little
constexpr bool CPP_TOOLBOX_NATIVE_IS_LITTLE = true;
constexpr bool CPP_TOOLBOX_NATIVE_IS_BIG = false;

#  elif defined(__APPLE__) && defined(__MACH__)
   // macOS (Clang) on supported architectures (x86_64, arm64) is little-endian
// Include <machine/endian.h> or rely on compiler builtins if available
#    include <machine/endian.h>  // Standard macOS header for BYTE_ORDER etc.
   // Check __BYTE_ORDER__ defensively, but default to little if check fails?
   // Common case is LE.
#    if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      warning \
          "Detected Big Endian macOS? This is unusual. Verify target architecture."
#      define CPP_TOOLBOX_NATIVE_ENDIAN_ENUM cpp_toolbox::detail::endian::big
constexpr bool CPP_TOOLBOX_NATIVE_IS_LITTLE = false;
constexpr bool CPP_TOOLBOX_NATIVE_IS_BIG = true;
#    else
#      define CPP_TOOLBOX_NATIVE_ENDIAN_ENUM cpp_toolbox::detail::endian::little
constexpr bool CPP_TOOLBOX_NATIVE_IS_LITTLE = true;
constexpr bool CPP_TOOLBOX_NATIVE_IS_BIG = false;
#    endif

#  elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) \
      && defined(__ORDER_BIG_ENDIAN__)
   // Common Linux/GCC/Clang way using predefined macros
#    if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#      define CPP_TOOLBOX_NATIVE_ENDIAN_ENUM cpp_toolbox::detail::endian::little
constexpr bool CPP_TOOLBOX_NATIVE_IS_LITTLE = true;
constexpr bool CPP_TOOLBOX_NATIVE_IS_BIG = false;
#    elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#      define CPP_TOOLBOX_NATIVE_ENDIAN_ENUM cpp_toolbox::detail::endian::big
constexpr bool CPP_TOOLBOX_NATIVE_IS_LITTLE = false;
constexpr bool CPP_TOOLBOX_NATIVE_IS_BIG = true;
#    else
#      error \
          "Unknown machine endianness detected (__BYTE_ORDER__ defined but not LE/BE)."
constexpr bool CPP_TOOLBOX_NATIVE_IS_LITTLE = false;  // Avoid compilation error
constexpr bool CPP_TOOLBOX_NATIVE_IS_BIG = false;
#    endif

#  else
   // --- Fallback/Unknown ---
#    warning \
        "Could not reliably determine platform endianness. Assuming Little Endian. Define CPP_TOOLBOX_FORCE_ENDIANNESS if necessary."
   // Default assumption: Little Endian (most common desktop/mobile platform)
#    define CPP_TOOLBOX_NATIVE_ENDIAN_ENUM cpp_toolbox::detail::endian::little
constexpr bool CPP_TOOLBOX_NATIVE_IS_LITTLE = true;
constexpr bool CPP_TOOLBOX_NATIVE_IS_BIG = false;
// Consider adding a compile error here if strictness is required:
// #error "Cannot determine platform endianness. Please define
// CPP_TOOLBOX_FORCE_ENDIANNESS (little or big)."

#  endif  // Platform checks

#endif  // C++20 check
