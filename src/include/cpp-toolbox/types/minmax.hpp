#pragma once

#include <algorithm>
#include <cmath>
#include <future>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>  // For std::string, std::wstring
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/io/formats/base.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::types::detail
{

// Helper trait - Corrected for C++17
template<typename T, typename = void>
struct is_calculable_container : std::false_type
{
};

template<typename T>
struct is_calculable_container<
    T,
    std::void_t<decltype(std::begin(std::declval<const T&>())),
                decltype(std::end(std::declval<const T&>())),
                typename T::value_type>>
    : std::bool_constant<
          !std::is_same_v<
              std::decay_t<T>,
              std::string> && !std::is_same_v<std::decay_t<T>, std::wstring>>
{
};

template<typename T>
inline constexpr bool is_calculable_container_v =
    is_calculable_container<T>::value;

}  // namespace toolbox::types::detail

namespace toolbox::types
{

/**
 * @brief Primary template for minmax_t.
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT minmax_t
{
  T min;
  T max;
  bool initialized_ = false;

  // Constructor declarations
  minmax_t(T initial_min, T initial_max);
  minmax_t();  // Declaration - Generic definition is in impl

  // Rule of Five members (defaulted)
  minmax_t(const minmax_t& other) = default;
  minmax_t(minmax_t&& other) noexcept = default;
  minmax_t& operator=(const minmax_t& other) = default;
  minmax_t& operator=(minmax_t&& other) noexcept = default;
  ~minmax_t() = default;

  // Operator declaration
  minmax_t& operator+=(const T& value);
};

// --- Full specializations for primitive types' default constructors (Keep
// Definitions Here) --- These are small and crucial for usability, defined
// inline in the header.
template<>
inline minmax_t<int>::minmax_t()
    : min(std::numeric_limits<int>::max())
    , max(std::numeric_limits<int>::min())
    , initialized_(false)
{
}

template<>
inline minmax_t<float>::minmax_t()
    : min(std::numeric_limits<float>::max())
    , max(std::numeric_limits<float>::lowest())
    , initialized_(false)
{
}

template<>
inline minmax_t<double>::minmax_t()
    : min(std::numeric_limits<double>::max())
    , max(std::numeric_limits<double>::lowest())
    , initialized_(false)
{
}

template<>
inline minmax_t<long>::minmax_t()
    : min(std::numeric_limits<long>::max())
    , max(std::numeric_limits<long>::min())
    , initialized_(false)
{
}

template<>
inline minmax_t<unsigned int>::minmax_t()
    : min(std::numeric_limits<unsigned int>::max())
    , max(std::numeric_limits<unsigned int>::min())
    , initialized_(false)
{
}

template<>
inline minmax_t<unsigned long>::minmax_t()
    : min(std::numeric_limits<unsigned long>::max())
    , max(std::numeric_limits<unsigned long>::min())
    , initialized_(false)
{
}

template<>
inline minmax_t<unsigned long long>::minmax_t()
    : min(std::numeric_limits<unsigned long long>::max())
    , max(std::numeric_limits<unsigned long long>::min())
    , initialized_(false)
{
}

template<>
inline minmax_t<unsigned char>::minmax_t()
    : min(std::numeric_limits<unsigned char>::max())
    , max(std::numeric_limits<unsigned char>::min())
    , initialized_(false)
{
}

template<>
inline minmax_t<unsigned short>::minmax_t()
    : min(std::numeric_limits<unsigned short>::max())
    , max(std::numeric_limits<unsigned short>::min())
    , initialized_(false)
{
}

/**
 * @brief Partial specialization of minmax_t for point_t<T>.
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT minmax_t<point_t<T>>
{
  point_t<T> min;
  point_t<T> max;
  bool initialized_ = false;

  // Constructor declarations
  minmax_t();  // Definition in impl
  minmax_t(point_t<T> initial_min,
           point_t<T> initial_max);  // Definition in impl

  // Rule of Five members (defaulted)
  minmax_t(const minmax_t&) = default;
  minmax_t(minmax_t&&) noexcept = default;
  minmax_t& operator=(const minmax_t&) = default;
  minmax_t& operator=(minmax_t&&) noexcept = default;
  ~minmax_t() = default;

  // Operator declaration
  minmax_t& operator+=(const point_t<T>& value);  // Definition in impl
};

// --- Helper function declarations ---

template<typename T>
[[nodiscard]] minmax_t<T> combine_minmax(
    const minmax_t<T>& a, const minmax_t<T>& b);  // Definition in impl

template<typename CoordT>
[[nodiscard]] minmax_t<point_t<CoordT>> combine_minmax(
    const minmax_t<point_t<CoordT>>& a,
    const minmax_t<point_t<CoordT>>& b);  // Definition in impl

// --- Sequential calculate_minmax declarations ---
// Implementations moved to impl file.

template<typename InputType>
[[nodiscard]] auto calculate_minmax(const InputType& input)
    -> std::enable_if_t<!detail::is_calculable_container_v<InputType>,
                        minmax_t<std::decay_t<InputType>>>;

template<typename InputType>
[[nodiscard]] auto calculate_minmax(const InputType& input)
    -> std::enable_if_t<detail::is_calculable_container_v<InputType>,
                        minmax_t<typename InputType::value_type>>;

template<typename T>
[[nodiscard]] auto calculate_minmax(const point_cloud_t<T>& input)
    -> minmax_t<point_t<T>>;

// --- Parallel calculate_minmax declarations ---
// Implementations moved to impl file.

template<typename InputType>
[[nodiscard]] auto calculate_minmax_parallel(const InputType& input)
    -> std::enable_if_t<!detail::is_calculable_container_v<InputType>,
                        minmax_t<std::decay_t<InputType>>>;

template<typename InputType>
[[nodiscard]] auto calculate_minmax_parallel(const InputType& input)
    -> std::enable_if_t<detail::is_calculable_container_v<InputType>,
                        minmax_t<typename InputType::value_type>>;

template<typename T>
[[nodiscard]] auto calculate_minmax_parallel(const point_cloud_t<T>& input)
    -> minmax_t<point_t<T>>;

}  // namespace toolbox::types

// Include the implementation file at the end
#include "cpp-toolbox/types/impl/minmax_impl.hpp"