#pragma once

#include <cpp-toolbox/macro/class.hpp>
#include <cpp-toolbox/macro/endian.hpp>
#include <cpp-toolbox/macro/platforms.hpp>

/**
 * @file macro.hpp
 * @brief 通用的编译器、平台、架构检测和实用宏定义 / Common macros for compiler,
 * platform, architecture detection and utility macros
 */

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
#if defined(CPP_TOOLBOX_PLATFORM_WINDOWS) && defined(CPP_TOOLBOX_COMPILER_MSVC)
#  define __CURRENT_FUNCTION__ __FUNCSIG__
#else
#  define __CURRENT_FUNCTION__ __PRETTY_FUNCTION__
#endif