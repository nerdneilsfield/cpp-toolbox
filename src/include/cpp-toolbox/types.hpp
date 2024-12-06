#pragma once

#include <atomic>
#include <cstdint>

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using size_t = std::size_t;

using f32 = float;
using f64 = double;

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

using ai8 = std::atomic<i8>;
using ai16 = std::atomic<i16>;
using ai32 = std::atomic<i32>;
using ai64 = std::atomic<i64>;
using af32 = std::atomic<f32>;
using af64 = std::atomic<f64>;
using asize_t = std::atomic<size_t>;
using abool = std::atomic<bool>;
using au8 = std::atomic<u8>;
using au16 = std::atomic<u16>;
using au32 = std::atomic<u32>;
using au64 = std::atomic<u64>;
