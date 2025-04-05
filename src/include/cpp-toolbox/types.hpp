#pragma once

#include <atomic>
#include <cstdint>

/**
 * @brief Common integer type aliases for fixed width integers
 * @{
 */
using i8 = int8_t;  ///< 8-bit signed integer
using i16 = int16_t;  ///< 16-bit signed integer
using i32 = int32_t;  ///< 32-bit signed integer
using i64 = int64_t;  ///< 64-bit signed integer
/** @} */

/**
 * @brief Common unsigned integer type aliases for fixed width integers
 * @{
 */
using u8 = uint8_t;  ///< 8-bit unsigned integer
using u16 = uint16_t;  ///< 16-bit unsigned integer
using u32 = uint32_t;  ///< 32-bit unsigned integer
using u64 = uint64_t;  ///< 64-bit unsigned integer
/** @} */
/**
 * @brief Size type alias
 */
using size_t = std::size_t;

/**
 * @brief Floating point type aliases
 * @{
 */
using f32 = float;  ///< 32-bit floating point
using f64 = double;  ///< 64-bit floating point
/** @} */
// Size assertions
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
 * @brief Atomic type aliases for common types
 *
 * These aliases provide atomic versions of the basic integer, floating point
 * and boolean types for thread-safe operations.
 *
 * @code{.cpp}
 * // Example usage of atomic types
 * ai32 counter{0};
 * counter.fetch_add(1);
 *
 * abool flag{false};
 * flag.store(true);
 *
 * au64 timestamp{0};
 * timestamp.store(getCurrentTime());
 * @endcode
 * @{
 */
using ai8 = std::atomic<i8>;  ///< Atomic 8-bit signed integer
using ai16 = std::atomic<i16>;  ///< Atomic 16-bit signed integer
using ai32 = std::atomic<i32>;  ///< Atomic 32-bit signed integer
using ai64 = std::atomic<i64>;  ///< Atomic 64-bit signed integer
using af32 = std::atomic<f32>;  ///< Atomic 32-bit float
using af64 = std::atomic<f64>;  ///< Atomic 64-bit float
using asize_t = std::atomic<size_t>;  ///< Atomic size_t
using abool = std::atomic<bool>;  ///< Atomic boolean
using au8 = std::atomic<u8>;  ///< Atomic 8-bit unsigned integer
using au16 = std::atomic<u16>;  ///< Atomic 16-bit unsigned integer
using au32 = std::atomic<u32>;  ///< Atomic 32-bit unsigned integer
using au64 = std::atomic<u64>;  ///< Atomic 64-bit unsigned integer
/** @} */
