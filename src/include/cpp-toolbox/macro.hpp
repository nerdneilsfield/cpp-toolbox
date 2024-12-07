#pragma once

// Compiler detection
#if defined(_MSC_VER)
#  define CPP_TOOLBOX_COMPILER_MSVC
#elif defined(__clang__)
#  define CPP_TOOLBOX_COMPILER_CLANG
#elif defined(__GNUC__)
#  define CPP_TOOLBOX_COMPILER_GCC
#endif

// Platform detection
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

// Architecture detection
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

#define CPP_VERSION_STR(year, month) #year #month "L"
#define CPP_VERSION_TO_INT(year, month) year##month##L

// C++ standard detection
#if defined(__cplusplus)
#  define CPP_TOOLBOX_CXX_STANDARD __cplusplus
#endif

// Check C++ version
#define CPP_11 CPP_VERSION_TO_INT(201, 103)
#define CPP_14 CPP_VERSION_TO_INT(201, 402)
#define CPP_17 CPP_VERSION_TO_INT(201, 703)
#define CPP_20 CPP_VERSION_TO_INT(202, 002)

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

#define CPP_TOOLBOX_CXX_LARGE_THAN_17(Content) \
  static_assert(__cplusplus >= CPP_17, Content)

#define CPP_TOOLBOX_CXX_VERSION get_cpp_version()

#define CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  ClassType(const ClassType&) = delete; \
  ClassType& operator=(const ClassType&) = delete;

#define CPP_TOOLBOX_DISABLE_MOVE(ClassType) \
  ClassType(ClassType&&) = delete; \
  ClassType& operator=(ClassType&&) = delete;

#define CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(ClassType) \
  CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  CPP_TOOLBOX_DISABLE_MOVE(ClassType)

#define CPP_TOOLBOX_DEFAULT_CONSTRUCTOR(ClassType) ClassType() = default;

#define CPP_TOOLBOX_DELETE_CONSTRUCTOR(ClassType) \
  ClassType(const ClassType&) = delete; \
  ClassType(ClassType&&) = delete;

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

#define CPP_TOOLBOX_STATIC_ASSERT(Condition, Message) \
  static_assert(Condition, Message)

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

#define CPP_TOOLBOX_UNREACHABLE() \
  do { \
    std::cerr << "Unreachable code reached" << std::endl; \
    std::abort(); \
  } while (false)

// optimization branch prediction
#define CPP_TOOLBOX_LIKELY(Condition) __builtin_expect(!!(Condition), true)

#define CPP_TOOLBOX_UNLIKELY(Condition) __builtin_expect(!!(Condition), false)

// force inline
#if defined(CPP_TOOLBOX_COMPILER_MSVC)
#  define CPP_TOOLBOX_FORCE_INLINE __forceinline
#elif defined(CPP_TOOLBOX_COMPILER_CLANG) || defined(CPP_TOOLBOX_COMPILER_GCC)
#  define CPP_TOOLBOX_FORCE_INLINE __attribute__((always_inline)) inline
#else
#  define CPP_TOOLBOX_FORCE_INLINE inline
#endif

// function name
#if defined(CPP_TOOLBOX_COMPILER_MSVC)
#  define CPP_TOOLBOX_FUNCTION_NAME __FUNCTION__
#elif defined(CPP_TOOLBOX_COMPILER_CLANG) || defined(CPP_TOOLBOX_COMPILER_GCC)
#  define CPP_TOOLBOX_FUNCTION_NAME __func__
#endif

#define CPP_TOOLBOX_ALIGNAS(Alignment) alignas(Alignment)

#define CPP_TOOLBOX_UNUSED(Variable) ((void)Variable)

#define CPP_TOOLBOX_NODISCARD [[nodiscard]]

#define CPP_TOOLBOX_FALLTHROUGH [[fallthrough]]

// logging with given logger
#if defined(CPP_TOOLBOX_DEBUG)
#  define CPP_TOOLBOX_LOG_DEBUG(fmt, ...) \
    LOG_DEBUG_S << __FILE__ << ":" << __LINE__ << " " \
                << CPP_TOOLBOX_FUNCTION_NAME << " " << fmt << __VA_ARGS__
#else
#  define CPP_TOOLBOX_LOG_DEBUG(fmt, ...) ((void)0)
#endif

#define CPP_TOOLBOX_COUNT_ARGS(...) \
  CPP_TOOLBOX_COUNT_ARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define CPP_TOOLBOX_COUNT_ARGS_IMPL( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, Count, ...) \
  Count

#define CPP_TOOLBOX_REPEAT_2(x) x x
#define CPP_TOOLBOX_REPEAT_3(x) CPP_TOOLBOX_REPEAT_2(x) x
#define CPP_TOOLBOX_REPEAT_4(x) CPP_TOOLBOX_REPEAT_2(x) CPP_TOOLBOX_REPEAT_2(x)
#define CPP_TOOLBOX_REPEAT_5(x) CPP_TOOLBOX_REPEAT_4(x) x
#define CPP_TOOLBOX_REPEAT(n, x) CPP_TOOLBOX_REPEAT_##n(x)

// string concat
#define CPP_TOOLBOX_STRING_CONCAT(a, b) a##b

// stringize
#define CPP_TOOLBOX_STRINGIZE(x) CPP_TOOLBOX_STRINGIZE_IMPL(x)
#define CPP_TOOLBOX_STRINGIZE_IMPL(x) #x

// // function call
// #define CPP_TOOLBOX_FUNCTION_NAME CPP_TOOLBOX_STRINGIZE(__func__)

#define CPP_TOOLBOX_SAFE_CALL(Func) \
  try { \
    Func(); \
  } catch (const std::exception& e) { \
    LOG_ERROR_S << __FILE__ << ":" << __LINE__ << " " \
                << CPP_TOOLBOX_FUNCTION_NAME << " " \
                << "Exception: " << e.what(); \
  }

// export
#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS)
#  define CPP_TOOLBOX_EXPORT \
    __declspec(dllexport) __attribute__((visibility("default")))
#else
#  define CPP_TOOLBOX_EXPORT __attribute__((visibility("default")))
#endif


#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
    #define __CURRENT_FUNCTION__ __FUNCSIG__
#else
    #define __CURRENT_FUNCTION__ __PRETTY_FUNCTION__
#endif