#pragma once

#include <atomic>
#include <cstdint>

/**
 * @brief 常用固定宽度整数类型别名/Common integer type aliases for fixed width
 * integers
 * @{
 */
using i8 = int8_t;  ///< 8位有符号整数/8-bit signed integer
using i16 = int16_t;  ///< 16位有符号整数/16-bit signed integer
using i32 = int32_t;  ///< 32位有符号整数/32-bit signed integer
using i64 = int64_t;  ///< 64位有符号整数/64-bit signed integer
/** @} */

/**
 * @brief 常用固定宽度无符号整数类型别名/Common unsigned integer type aliases
 * for fixed width integers
 * @{
 */
using u8 = uint8_t;  ///< 8位无符号整数/8-bit unsigned integer
using u16 = uint16_t;  ///< 16位无符号整数/16-bit unsigned integer
using u32 = uint32_t;  ///< 32位无符号整数/32-bit unsigned integer
using u64 = uint64_t;  ///< 64位无符号整数/64-bit unsigned integer
/** @} */

/**
 * @brief 大小类型别名/Size type alias
 */
using size_t = std::size_t;

/**
 * @brief 浮点数类型别名/Floating point type aliases
 * @{
 */
using f32 = float;  ///< 32位浮点数/32-bit floating point
using f64 = double;  ///< 64位浮点数/64-bit floating point
/** @} */

/**
 * @brief 大小断言检查/Size assertions check
 *
 * 确保类型大小符合预期/Ensure type sizes match expectations
 */
static_assert(sizeof(i8) == 1, "i8 is not 1 byte");
static_assert(sizeof(i16) == 2, "i16 is not 2 bytes");
static_assert(sizeof(i32) == 4, "i32 is not 4 bytes");
static_assert(sizeof(i64) == 8, "i64 is not 8 bytes");
static_assert(sizeof(u8) == 1, "u8 is not 1 byte");
static_assert(sizeof(u16) == 2, "u16 is not 2 bytes");
static_assert(sizeof(u32) == 4, "u32 is not 4 bytes");
static_assert(sizeof(u64) == 8, "u64 is not 8 bytes");
static_assert(sizeof(size_t) == 8, "size_t is not 8 bytes");
static_assert(sizeof(f32) == 4, "f32 is not 4 bytes");
static_assert(sizeof(f64) == 8, "f64 is not 8 bytes");

/**
 * @brief 常用类型的原子类型别名/Atomic type aliases for common types
 *
 * 这些别名为基本整数、浮点数和布尔类型提供原子版本，用于线程安全操作/
 * These aliases provide atomic versions of the basic integer, floating point
 * and boolean types for thread-safe operations.
 *
 * @code{.cpp}
 * // 原子类型使用示例/Example usage of atomic types
 *
 * // 原子计数器/Atomic counter
 * ai32 counter{0};
 * counter.fetch_add(1);
 *
 * // 原子标志位/Atomic flag
 * abool flag{false};
 * flag.store(true);
 *
 * // 原子时间戳/Atomic timestamp
 * au64 timestamp{0};
 * timestamp.store(getCurrentTime());
 *
 * // 原子浮点数/Atomic floating point
 * af64 value{1.0};
 * value.store(3.14);
 * @endcode
 * @{
 */
using ai8 = std::atomic<i8>;  ///< 原子8位有符号整数/Atomic 8-bit signed integer
using ai16 =
    std::atomic<i16>;  ///< 原子16位有符号整数/Atomic 16-bit signed integer
using ai32 =
    std::atomic<i32>;  ///< 原子32位有符号整数/Atomic 32-bit signed integer
using ai64 =
    std::atomic<i64>;  ///< 原子64位有符号整数/Atomic 64-bit signed integer
using af32 = std::atomic<f32>;  ///< 原子32位浮点数/Atomic 32-bit float
using af64 = std::atomic<f64>;  ///< 原子64位浮点数/Atomic 64-bit float
using asize_t = std::atomic<size_t>;  ///< 原子size_t类型/Atomic size_t
using abool = std::atomic<bool>;  ///< 原子布尔类型/Atomic boolean
using au8 =
    std::atomic<u8>;  ///< 原子8位无符号整数/Atomic 8-bit unsigned integer
using au16 =
    std::atomic<u16>;  ///< 原子16位无符号整数/Atomic 16-bit unsigned integer
using au32 =
    std::atomic<u32>;  ///< 原子32位无符号整数/Atomic 32-bit unsigned integer
using au64 =
    std::atomic<u64>;  ///< 原子64位无符号整数/Atomic 64-bit unsigned integer
/** @} */
