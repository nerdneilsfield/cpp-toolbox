#pragma once

/**
 * @file macro.hpp
 * @brief 通用的编译器、平台、架构检测和实用宏定义 / Common macros for compiler,
 * platform, architecture detection and utility macros
 */

/**
 * @brief 编译器检测宏 / Compiler detection macros
 *
 * @details 用于检测当前使用的编译器类型 / Used to detect the current compiler
 * type:
 * - MSVC: Microsoft Visual C++ 编译器 / Microsoft Visual C++ Compiler
 * - Clang: LLVM Clang 编译器 / LLVM Clang Compiler
 * - GCC: GNU C++ 编译器 / GNU C++ Compiler
 */
#if defined(_MSC_VER)
#  define CPP_TOOLBOX_COMPILER_MSVC
#elif defined(__clang__)
#  define CPP_TOOLBOX_COMPILER_CLANG
#elif defined(__GNUC__)
#  define CPP_TOOLBOX_COMPILER_GCC
#endif

/**
 * @brief 平台检测宏 / Platform detection macros
 *
 * @details 用于检测当前运行平台 / Used to detect the current platform:
 * - Windows: Windows 操作系统 / Windows Operating System
 * - Linux: Linux 操作系统 / Linux Operating System
 * - macOS: Apple macOS 操作系统 / Apple macOS Operating System
 * - FreeBSD: FreeBSD 操作系统 / FreeBSD Operating System
 * - NetBSD: NetBSD 操作系统 / NetBSD Operating System
 * - Android: Android 操作系统 / Android Operating System
 * - QNX: QNX 实时操作系统 / QNX Real-time Operating System
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
 * @brief 架构检测宏 / Architecture detection macros
 *
 * @details 用于检测当前CPU架构 / Used to detect the current CPU architecture:
 * - x86_64: 64位x86架构 / 64-bit x86 architecture
 * - x86: 32位x86架构 / 32-bit x86 architecture
 * - ARM64: 64位ARM架构 / 64-bit ARM architecture
 * - ARM: 32位ARM架构 / 32-bit ARM architecture
 * - RISC-V: RISC-V架构 / RISC-V architecture
 * - LoongArch: 龙芯架构 / LoongArch architecture
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
 * @brief 将C++版本转换为字符串 / Convert C++ version to string
 * @param year 版本号中的年份部分 / Year part of version number
 * @param month 版本号中的月份部分 / Month part of version number
 * @return 版本的字符串表示 / String representation of version
 *
 * @code{.cpp}
 * // 将C++17版本号转换为字符串
 * // Convert C++17 version to string
 * const char* version = CPP_VERSION_STR(201, 703);
 * std::cout << version << std::endl; // 输出: "201703L" / Outputs: "201703L"
 * @endcode
 */
#define CPP_VERSION_STR(year, month) #year #month "L"

/**
 * @brief 将C++版本转换为整数 / Convert C++ version to integer
 * @param year 版本号中的年份部分 / Year part of version number
 * @param month 版本号中的月份部分 / Month part of version number
 * @return 版本的整数表示 / Integer representation of version
 *
 * @code{.cpp}
 * // 将C++17版本号转换为整数
 * // Convert C++17 version to integer
 * constexpr auto version = CPP_VERSION_TO_INT(201, 703);
 * static_assert(version == 201703L);
 * @endcode
 */
#define CPP_VERSION_TO_INT(year, month) year##month##L

/**
 * @brief C++标准检测 / C++ standard detection
 */
#if defined(__cplusplus)
#  define CPP_TOOLBOX_CXX_STANDARD __cplusplus
#endif

/**
 * @brief C++版本宏定义 / C++ version macros
 */
#define CPP_11 CPP_VERSION_TO_INT(201, 103)
#define CPP_14 CPP_VERSION_TO_INT(201, 402)
#define CPP_17 CPP_VERSION_TO_INT(201, 703)
#define CPP_20 CPP_VERSION_TO_INT(202, 002)

/**
 * @brief 获取C++版本字符串 / Get C++ version as string
 * @return C++版本的字符串表示 / String representation of C++ version
 *
 * @code{.cpp}
 * const char* version = get_cpp_version();
 * std::cout << "Using " << version << std::endl; // 输出当前C++版本 / Outputs
 * current C++ version
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
 * @brief C++17要求的静态断言 / Static assert for C++17 requirement
 * @param Content 断言消息 / Assert message
 *
 * @code{.cpp}
 * CPP_TOOLBOX_CXX_LARGE_THAN_17("This feature requires C++17 or later");
 * @endcode
 */
#define CPP_TOOLBOX_CXX_LARGE_THAN_17(Content) \
  static_assert(__cplusplus >= CPP_17, Content)

/**
 * @brief 获取C++版本字符串 / Get C++ version string
 */
#define CPP_TOOLBOX_CXX_VERSION get_cpp_version()

/**
 * @brief 禁用类的拷贝操作 / Disable copy operations for a class
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 *   CPP_TOOLBOX_DISABLE_COPY(MyClass)
 * public:
 *   MyClass() = default;
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  ClassType(const ClassType&) = delete; \
  ClassType& operator=(const ClassType&) = delete;

/**
 * @brief 禁用类的移动操作 / Disable move operations for a class
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 *   CPP_TOOLBOX_DISABLE_MOVE(MyClass)
 * public:
 *   MyClass() = default;
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DISABLE_MOVE(ClassType) \
  ClassType(ClassType&&) = delete; \
  ClassType& operator=(ClassType&&) = delete;

/**
 * @brief 禁用类的拷贝和移动操作 / Disable both copy and move operations
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 *   CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(MyClass)
 * public:
 *   MyClass() = default;
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DISABLE_COPY_AND_MOVE(ClassType) \
  CPP_TOOLBOX_DISABLE_COPY(ClassType) \
  CPP_TOOLBOX_DISABLE_MOVE(ClassType)

/**
 * @brief 默认构造函数宏 / Default constructor macro
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 * public:
 *   CPP_TOOLBOX_DEFAULT_CONSTRUCTOR(MyClass)
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DEFAULT_CONSTRUCTOR(ClassType) ClassType() = default;

/**
 * @brief 删除拷贝和移动构造函数 / Delete copy and move constructors
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MyClass {
 * public:
 *   CPP_TOOLBOX_DELETE_CONSTRUCTOR(MyClass)
 * };
 * @endcode
 */
#define CPP_TOOLBOX_DELETE_CONSTRUCTOR(ClassType) \
  ClassType(const ClassType&) = delete; \
  ClassType(ClassType&&) = delete;

/**
 * @brief 单例模式宏 / Singleton pattern macro
 * @param ClassType 类名 / Class name
 *
 * @code{.cpp}
 * class MySingleton {
 *   CPP_TOOLBOX_SINGLETON(MySingleton)
 * public:
 *   void doSomething() {}
 * };
 *
 * // 使用单例 / Using singleton
 * MySingleton::instance().doSomething();
 * @endcode
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
 * @brief 静态断言宏 / Static assertion macro
 * @param Condition 断言条件 / Assertion condition
 * @param Message 错误消息 / Error message
 *
 * @code{.cpp}
 * CPP_TOOLBOX_STATIC_ASSERT(sizeof(int) == 4, "Int must be 32 bits");
 * @endcode
 */
#define CPP_TOOLBOX_STATIC_ASSERT(Condition, Message) \
  static_assert(Condition, Message)

/**
 * @brief 运行时断言宏(仅在调试模式下) / Runtime assertion macro (debug only)
 * @param Condition 断言条件 / Assertion condition
 * @param Message 错误消息 / Error message
 *
 * @code{.cpp}
 * void process(int* ptr) {
 *   CPP_TOOLBOX_ASSERT(ptr != nullptr, "Null pointer not allowed");
 *   // 处理指针 / Process pointer
 * }
 * @endcode
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
 * @brief 不可达代码标记 / Unreachable code marker
 *
 * @code{.cpp}
 * switch(value) {
 *   case 1: return "one";
 *   case 2: return "two";
 *   default: CPP_TOOLBOX_UNREACHABLE();
 * }
 * @endcode
 */
#define CPP_TOOLBOX_UNREACHABLE() \
  do { \
    std::cerr << "Unreachable code reached" << std::endl; \
    std::abort(); \
  } while (false)

/**
 * @brief 分支预测优化宏 / Branch prediction optimization macros
 *
 * @code{.cpp}
 * if (CPP_TOOLBOX_LIKELY(ptr != nullptr)) {
 *   // 常见情况 / Common case
 * } else {
 *   // 罕见情况 / Rare case
 * }
 * @endcode
 */
#define CPP_TOOLBOX_LIKELY(Condition) __builtin_expect(!!(Condition), true)
#define CPP_TOOLBOX_UNLIKELY(Condition) __builtin_expect(!!(Condition), false)

/**
 * @brief 强制内联宏 / Force inline macro
 *
 * @code{.cpp}
 * CPP_TOOLBOX_FORCE_INLINE int add(int a, int b) {
 *   return a + b;
 * }
 * @endcode
 */
#if defined(CPP_TOOLBOX_COMPILER_MSVC)
#  define CPP_TOOLBOX_FORCE_INLINE __forceinline
#elif defined(CPP_TOOLBOX_COMPILER_CLANG) || defined(CPP_TOOLBOX_COMPILER_GCC)
#  define CPP_TOOLBOX_FORCE_INLINE __attribute__((always_inline)) inline
#else
#  define CPP_TOOLBOX_FORCE_INLINE inline
#endif

/**
 * @brief 函数名宏 / Function name macro
 */
#if defined(CPP_TOOLBOX_COMPILER_MSVC)
#  define CPP_TOOLBOX_FUNCTION_NAME __FUNCTION__
#elif defined(CPP_TOOLBOX_COMPILER_CLANG) || defined(CPP_TOOLBOX_COMPILER_GCC)
#  define CPP_TOOLBOX_FUNCTION_NAME __func__
#endif

/**
 * @brief 数据对齐宏 / Data alignment macro
 * @param Alignment 对齐值 / Alignment value
 *
 * @code{.cpp}
 * CPP_TOOLBOX_ALIGNAS(16) float matrix[4][4];
 * @endcode
 */
#define CPP_TOOLBOX_ALIGNAS(Alignment) alignas(Alignment)

/**
 * @brief 标记变量为未使用 / Mark variable as unused
 * @param Variable 变量名 / Variable name
 *
 * @code{.cpp}
 * void callback(int unused_param) {
 *   CPP_TOOLBOX_UNUSED(unused_param);
 * }
 * @endcode
 */
#define CPP_TOOLBOX_UNUSED(Variable) ((void)(Variable))

/**
 * @brief 不可丢弃属性 / No discard attribute
 *
 * @code{.cpp}
 * CPP_TOOLBOX_NODISCARD int getValue() {
 *   return 42;
 * }
 * @endcode
 */
#define CPP_TOOLBOX_NODISCARD [[nodiscard]]

/**
 * @brief 贯穿属性 / Fallthrough attribute
 *
 * @code{.cpp}
 * switch(value) {
 *   case 1:
 *     doSomething();
 *     CPP_TOOLBOX_FALLTHROUGH;
 *   case 2:
 *     doMore();
 *     break;
 * }
 * @endcode
 */
#define CPP_TOOLBOX_FALLTHROUGH [[fallthrough]]

/**
 * @brief 调试日志宏 / Debug logging macro
 *
 * @code{.cpp}
 * CPP_TOOLBOX_LOG_DEBUG("Processing item %d", itemId);
 * @endcode
 */
#if defined(CPP_TOOLBOX_DEBUG)
#  define CPP_TOOLBOX_LOG_DEBUG(fmt, ...) \
    LOG_DEBUG_S << __FILE__ << ":" << __LINE__ << " " \
                << CPP_TOOLBOX_FUNCTION_NAME << " " << fmt << __VA_ARGS__
#else
#  define CPP_TOOLBOX_LOG_DEBUG(fmt, ...) ((void)0)
#endif

/**
 * @brief 计算参数数量 / Count number of arguments
 *
 * @code{.cpp}
 * int count = CPP_TOOLBOX_COUNT_ARGS(a, b, c); // count = 3
 * @endcode
 */
#define CPP_TOOLBOX_COUNT_ARGS(...) \
  CPP_TOOLBOX_COUNT_ARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define CPP_TOOLBOX_COUNT_ARGS_IMPL( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, Count, ...) \
  Count

/**
 * @brief 重复宏 / Repeat macros
 *
 * @code{.cpp}
 * CPP_TOOLBOX_REPEAT(3, "Hello ") // 输出: "Hello Hello Hello " / Outputs:
 * "Hello Hello Hello "
 * @endcode
 */
#define CPP_TOOLBOX_REPEAT_2(x) x x
#define CPP_TOOLBOX_REPEAT_3(x) CPP_TOOLBOX_REPEAT_2(x) x
#define CPP_TOOLBOX_REPEAT_4(x) CPP_TOOLBOX_REPEAT_2(x) CPP_TOOLBOX_REPEAT_2(x)
#define CPP_TOOLBOX_REPEAT_5(x) CPP_TOOLBOX_REPEAT_4(x) x
#define CPP_TOOLBOX_REPEAT(n, x) CPP_TOOLBOX_REPEAT_##n(x)

/**
 * @brief 字符串连接 / String concatenation
 *
 * @code{.cpp}
 * #define PREFIX "my_"
 * #define NAME "variable"
 * char* full_name = CPP_TOOLBOX_STRING_CONCAT(PREFIX, NAME); // "my_variable"
 * @endcode
 */
#define CPP_TOOLBOX_STRING_CONCAT(a, b) a##b

/**
 * @brief 字符串化宏 / Stringize macro
 *
 * @code{.cpp}
 * #define VERSION 1.0
 * const char* version = CPP_TOOLBOX_STRINGIZE(VERSION); // "1.0"
 * @endcode
 */
#define CPP_TOOLBOX_STRINGIZE(x) CPP_TOOLBOX_STRINGIZE_IMPL(x)
#define CPP_TOOLBOX_STRINGIZE_IMPL(x) #x

/**
 * @brief 带异常处理的安全函数调用 / Safe function call with exception handling
 * @param Func 要调用的函数 / Function to call
 *
 * @code{.cpp}
 * void riskyOperation() {
 *   CPP_TOOLBOX_SAFE_CALL([]() {
 *     // 可能抛出异常的代码 / Code that might throw
 *     throw std::runtime_error("Error");
 *   });
 * }
 * @endcode
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
 * @brief 当前函数名宏 / Current function name macro
 */
#ifdef CPP_TOOLBOX_PLATFORM_WINDOWS
#  define __CURRENT_FUNCTION__ __FUNCSIG__
#else
#  define __CURRENT_FUNCTION__ __PRETTY_FUNCTION__
#endif