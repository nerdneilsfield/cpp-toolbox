#pragma once

/**
 * @file macro.hpp
 * @brief Common macros for compiler, platform, architecture detection and
 * utility macros
 */

/**
 * @brief Compiler detection macros
 */
#if defined(_MSC_VER)
#  define CPP_TOOLBOX_COMPILER_MSVC
#elif defined(__clang__)
#  define CPP_TOOLBOX_COMPILER_CLANG
#elif defined(__GNUC__)
#  define CPP_TOOLBOX_COMPILER_GCC
#endif

/**
 * @brief Platform detection macros
 */
#if defined(_WIN32) || defined(_WIN64)
#  define CPP_TOOLBOX_PLATFORM_WINDOWS
#elif defined(__linux__) || defined(__linux)
#  define CPP_TOOLBOX_PLATFORM_LINUX
#elif defined(__APPLE__) || defined(__MACH__)
#  define CPP_TOOLBOX_PLATFORM_MACOS
#elif defined(__FreeBSD__) || defined(__FreeBSD)
#  define CPP_TOOLBOX_PLATFORM_FREEBSD
#elif defined(__NetBSD__) || defined(__NetBSD)
#  define CPP_TOOLBOX_PLATFORM_NETBSD
// Android
#elif defined(__ANDROID__) || defined(_ANDROID)
#  define CPP_TOOLBOX_PLATFORM_ANDROID
#elif defined(__QNX__) || defined(_QNX)
#  define CPP_TOOLBOX_PLATFORM_QNX
#endif

/**
 * @brief Architecture detection macros
 */
#if defined(__x86_64__) || defined(__amd64__) || defined(__amd64) \
    || defined(_M_X64)
#  define CPP_TOOLBOX_ARCH_X86_64
#elif defined(__i386__) || defined(__i386) || defined(_M_IX86)
#  define CPP_TOOLBOX_ARCH_X86
#elif defined(__aarch64__) || defined(__arm64__) || defined(__arm64) \
    || defined(_M_ARM64)
#  define CPP_TOOLBOX_ARCH_ARM64
#elif defined(__arm__) || defined(__arm) || defined(_M_ARM)
#  define CPP_TOOLBOX_ARCH_ARM
#elif defined(__riscv__) || defined(__riscv) || defined(_M_RISCV)
#  define CPP_TOOLBOX_ARCH_RISCV
#elif defined(__loongarch__) || defined(__loongarch) || defined(_M_LOONGARCH)
#  define CPP_TOOLBOX_ARCH_LOONGARCH
#endif

/**
 * @brief Convert C++ version to string
 * @param year Year part of version number
 * @param month Month part of version number
 * @return String representation of version
 */
#define CPP_VERSION_STR(year, month) #year #month "L"

/**
 * @brief Convert C++ version to integer
 * @param year Year part of version number
 * @param month Month part of version number
 * @return Integer representation of version
 */
#define CPP_VERSION_TO_INT(year, month) year##month##L

/**
 * @brief C++ standard detection
 */
#if defined(__cplusplus)
#  define CPP_TOOLBOX_CXX_STANDARD __cplusplus
#endif

/**
 * @brief C++ version macros
 */
#define CPP_11 CPP_VERSION_TO_INT(201, 103)
#define CPP_14 CPP_VERSION_TO_INT(201, 402)
#define CPP_17 CPP_VERSION_TO_INT(201, 703)
#define CPP_20 CPP_VERSION_TO_INT(202, 002)

/**
 * @brief Get C++ version as string
 * @return String representation of C++ version
 *
 * @code{.cpp}
 * const char* version = get_cpp_version();
 * std::cout << "Using " << version << std::endl;
 * @endcode
 */
constexpr const char* get_cpp_version()
{
#if __cplusplus >= CPP_20
  return "C++20";
#elif __cplusplus >= CPP_17
  return "C++17";
#elif __cplusplus >= CPP_14
  return "C++14";
#elif __cplusplus >= CPP_11
  return "C++11";
#else
  return "Pre-C++11";
#endif
}

/**
 * @brief Static assert for C++17 requirement
 * @param Content Assert message
 */
#define CPP_TOOLBOX_CXX_LARGE_THAN_17(Content) \
  static_assert(__cplusplus >= CPP_17, Content)

/**
 * @brief Get C++ version string
 */
#define CPP_TOOLBOX_CXX_VERSION get_cpp_version()

/**
 * @brief Disable copy operations for a class
 * @param ClassType Class name
 */
#define CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  ClassType(const ClassType&) = delete; \
  ClassType& operator=(const ClassType&) = delete;

/**
 * @brief Disable move operations for a class
 * @param ClassType Class name
 */
#define CPP_TOOLBOX_DISABLE_MOVE(ClassType) \
  ClassType(ClassType&&) = delete; \
  ClassType& operator=(ClassType&&) = delete;

/**
 * @brief Disable both copy and move operations
 * @param ClassType Class name
 */
#define CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(ClassType) \
  CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  CPP_TOOLBOX_DISABLE_MOVE(ClassType)

/**
 * @brief Default constructor macro
 * @param ClassType Class name
 */
#define CPP_TOOLBOX_DEFAULT_CONSTRUCTOR(ClassType) ClassType() = default;

/**
 * @brief Delete copy and move constructors
 * @param ClassType Class name
 */
#define CPP_TOOLBOX_DELETE_CONSTRUCTOR(ClassType) \
  ClassType(const ClassType&) = delete; \
  ClassType(ClassType&&) = delete;

/**
 * @brief Singleton pattern macro
 * @param ClassType Class name
 */
#define CPP_TOOLBOX_SINGLETON(ClassType) \
public: \
  static ClassType& instance() \
  { \
    static ClassType instance; \
    return instance; \
  } \
\
private: \
  ClassType() = default; \
  CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  CPP_TOOLBOX_DISABLE_MOVE(ClassType)

/**
 * @brief Static assertion macro
 * @param Condition Assertion condition
 * @param Message Error message
 */
#define CPP_TOOLBOX_STATIC_ASSERT(Condition, Message) \
  static_assert(Condition, Message)

/**
 * @brief Runtime assertion macro (debug only)
 * @param Condition Assertion condition
 * @param Message Error message
 */
#ifdef CPP_TOOLBOX_DEBUG
#  define CPP_TOOLBOX_ASSERT(Condition, Message) \
    do { \
      if (!(Condition)) { \
        std::cerr << "Assertion failed: " << Message << std::endl; \
        std::abort(); \
      } \
    } while (false)
#else
#  define CPP_TOOLBOX_ASSERT(Condition, Message) ((void)0)
#endif

/**
 * @brief Unreachable code marker
 */
#define CPP_TOOLBOX_UNREACHABLE() \
  do { \
    std::cerr << "Unreachable code reached" << std::endl; \
    std::abort(); \
  } while (false)

/**
 * @brief Branch prediction optimization macros
 */
#define CPP_TOOLBOX_LIKELY(Condition) __builtin_expect(!!(Condition), true)
#define CPP_TOOLBOX_UNLIKELY(Condition) __builtin_expect(!!(Condition), false)

/**
 * @brief Force inline macro
 */
#if defined(CPP_TOOLBOX_COMPILER_MSVC)
#  define CPP_TOOLBOX_FORCE_INLINE __forceinline
#elif defined(CPP_TOOLBOX_COMPILER_CLANG) || defined(CPP_TOOLBOX_COMPILER_GCC)
#  define CPP_TOOLBOX_FORCE_INLINE __attribute__((always_inline)) inline
#else
#  define CPP_TOOLBOX_FORCE_INLINE inline
#endif

/**
 * @brief Function name macro
 */
#if defined(CPP_TOOLBOX_COMPILER_MSVC)
#  define CPP_TOOLBOX_FUNCTION_NAME __FUNCTION__
#elif defined(CPP_TOOLBOX_COMPILER_CLANG) || defined(CPP_TOOLBOX_COMPILER_GCC)
#  define CPP_TOOLBOX_FUNCTION_NAME __func__
#endif

/**
 * @brief Data alignment macro
 * @param Alignment Alignment value
 */
#define CPP_TOOLBOX_ALIGNAS(Alignment) alignas(Alignment)

/**
 * @brief Mark variable as unused
 * @param Variable Variable name
 */
#define CPP_TOOLBOX_UNUSED(Variable) ((void)(Variable))

/**
 * @brief No discard attribute
 */
#define CPP_TOOLBOX_NODISCARD [[nodiscard]]

/**
 * @brief Fallthrough attribute
 */
#define CPP_TOOLBOX_FALLTHROUGH [[fallthrough]]

/**
 * @brief Debug logging macro
 */
#if defined(CPP_TOOLBOX_DEBUG)
#  define CPP_TOOLBOX_LOG_DEBUG(fmt, ...) \
    LOG_DEBUG_S << __FILE__ << ":" << __LINE__ << " " \
                << CPP_TOOLBOX_FUNCTION_NAME << " " << fmt << __VA_ARGS__
#else
#  define CPP_TOOLBOX_LOG_DEBUG(fmt, ...) ((void)0)
#endif

/**
 * @brief Count number of arguments
 */
#define CPP_TOOLBOX_COUNT_ARGS(...) \
  CPP_TOOLBOX_COUNT_ARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define CPP_TOOLBOX_COUNT_ARGS_IMPL( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, Count, ...) \
  Count

/**
 * @brief Repeat macros
 */
#define CPP_TOOLBOX_REPEAT_2(x) x x
#define CPP_TOOLBOX_REPEAT_3(x) CPP_TOOLBOX_REPEAT_2(x) x
#define CPP_TOOLBOX_REPEAT_4(x) CPP_TOOLBOX_REPEAT_2(x) CPP_TOOLBOX_REPEAT_2(x)
#define CPP_TOOLBOX_REPEAT_5(x) CPP_TOOLBOX_REPEAT_4(x) x
#define CPP_TOOLBOX_REPEAT(n, x) CPP_TOOLBOX_REPEAT_##n(x)

/**
 * @brief String concatenation
 */
#define CPP_TOOLBOX_STRING_CONCAT(a, b) a##b

/**
 * @brief Stringize macro
 */
#define CPP_TOOLBOX_STRINGIZE(x) CPP_TOOLBOX_STRINGIZE_IMPL(x)
#define CPP_TOOLBOX_STRINGIZE_IMPL(x) #x

/**
 * @brief Safe function call with exception handling
 * @param Func Function to call
 */
#define CPP_TOOLBOX_SAFE_CALL(Func) \
  try { \
    Func(); \
  } catch (const std::exception& e) { \
    LOG_ERROR_S << __FILE__ << ":" << __LINE__ << " " \
                << CPP_TOOLBOX_FUNCTION_NAME << " " \
                << "Exception: " << e.what(); \
  }

/**
 * @brief Current function name macro
 */
#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
#  define __CURRENT_FUNCTION__ __FUNCSIG__
#else
#  define __CURRENT_FUNCTION__ __PRETTY_FUNCTION__
#endif