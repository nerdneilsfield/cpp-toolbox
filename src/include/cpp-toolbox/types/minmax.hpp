#pragma once

#include <algorithm>  // For std::min and std::max
#include <limits>
#include <string>
#include <utility>
#include <wstring>  // For std::wstring

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/io/formats/base.hpp>
#include <cpp-toolbox/types/point.hpp>  // Include point header needed for specialization

namespace toolbox::types::detail
{

/// @brief Helper trait to check if a type T is a container suitable for min/max
/// calculation.
/// @tparam T The type to check.
/// @tparam U Dummy type for SFINAE.
template<typename T, typename = void>
struct is_calculable_container : std::false_type
{
};

/// @brief Specialization for types that have begin(), end(), and a value_type,
///        and are not standard string types.
/// @tparam T The type to check.
template<typename T>
struct is_calculable_container<
    T,
    std::void_t<decltype(std::begin(std::declval<const T&>())),
                decltype(std::end(std::declval<const T&>())),
                typename T::value_type>>
    : std::bool_constant<!std::is_same_v<std::decay_t<T>, std::string>
                         && !std::is_same_v<std::decay_t<T>, std::wstring>>
{
};

/// @brief Helper variable template that is true if T is a calculable container.
template<typename T>
inline constexpr bool is_calculable_container_v =
    is_calculable_container<T>::value;

}  // namespace toolbox::types::detail

namespace toolbox::types
{

/**
 * @brief Primary template for minmax_t.
 *
 * This template struct tracks the minimum and maximum values for a given type
 * T. It provides a generic operator+= to update the min and max values.
 *
 * @tparam T The type for which minimum and maximum values are tracked.
 *
 * @code
 * // Example usage for minmax_t with primitive types:
 * #include <limits>
 * #include <cpp-toolbox/types/minmax.hpp>
 *
 * int main() {
 *   // For int, the default constructor sets min to max int and max to min int.
 *   toolbox::types::minmax_t<int> mm;
 *   mm += 10;
 *   mm += 5;
 *   // Now, mm.min should be 5 and mm.max should be 10.
 *   return 0;
 * }
 * @endcode
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT minmax_t
{
  T min;  ///< The minimum value.
  T max;  ///< The maximum value.

  /**
   * @brief Constructor with initial values.
   *
   * @param initial_min The initial minimum value.
   * @param initial_max The initial maximum value.
   *
   * @code
   * // Direct initialization of minmax_t:
   * toolbox::types::minmax_t<double> mm(0.0, 100.0);
   * @endcode
   */
  minmax_t(T initial_min, T initial_max)
      : min(initial_min)
      , max(initial_max)
  {
  }

  /**
   * @brief Default constructor.
   *
   * This constructor is defined below or specialized for particular types.
   */
  minmax_t();

  // Rule of Five members (defaulted for the primary template)
  minmax_t(const minmax_t& other) = default;
  minmax_t(minmax_t&& other) noexcept = default;
  minmax_t& operator=(const minmax_t& other) = default;
  minmax_t& operator=(minmax_t&& other) noexcept = default;
  ~minmax_t() = default;

  /**
   * @brief Generic operator+= to update min and max.
   *
   * Updates the minimum and maximum values using a given value.
   * Requires T to be comparable using operator<.
   *
   * @param value The new value to consider.
   * @return Reference to the updated minmax_t.
   *
   * @code
   * // Example of using operator+=:
   * toolbox::types::minmax_t<int> mm(100, 0);
   * mm += 42;
   * // After this, mm.min becomes 42 and mm.max becomes 100.
   * @endcode
   */
  minmax_t& operator+=(const T& value)
  {
    if (value < min)
      min = value;
    if (value > max)
      max = value;
    return *this;
  }
};

// Generic default constructor definition (for types without specialization)
template<typename T>
CPP_TOOLBOX_EXPORT minmax_t<T>::minmax_t()
    : min(T {})
    , max(T {})
{
}

// --- Full specializations for primitive types' default constructors ---
template<>
CPP_TOOLBOX_EXPORT minmax_t<int>::minmax_t()
    : min(std::numeric_limits<int>::max())
    , max(std::numeric_limits<int>::min())
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<float>::minmax_t()
    : min(std::numeric_limits<float>::max())
    , max(std::numeric_limits<float>::lowest())
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<double>::minmax_t()
    : min(std::numeric_limits<double>::max())
    , max(std::numeric_limits<double>::lowest())
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<long>::minmax_t()
    : min(std::numeric_limits<long>::max())
    , max(std::numeric_limits<long>::min())
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned int>::minmax_t()
    : min(std::numeric_limits<unsigned int>::max())
    , max(std::numeric_limits<unsigned int>::min())
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned long>::minmax_t()
    : min(std::numeric_limits<unsigned long>::max())
    , max(std::numeric_limits<unsigned long>::min())
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned long long>::minmax_t()
    : min(std::numeric_limits<unsigned long long>::max())
    , max(std::numeric_limits<unsigned long long>::min())
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned char>::minmax_t()
    : min(std::numeric_limits<unsigned char>::max())
    , max(std::numeric_limits<unsigned char>::min())
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned short>::minmax_t()
    : min(std::numeric_limits<unsigned short>::max())
    , max(std::numeric_limits<unsigned short>::min())
{
}

/**
 * @brief Partial specialization of minmax_t for point_t<T>.
 *
 * This specialization tracks the minimum and maximum points in 3D space,
 * performing component-wise comparisons.
 *
 * @tparam T The coordinate type for point_t.
 *
 * @code
 * // Example usage for minmax_t with point_t:
 * #include <cpp-toolbox/types/minmax.hpp>
 * #include <cpp-toolbox/types/point.hpp>
 *
 * int main() {
 *   // Initialize with specific points, marking the object as initialized.
 *   toolbox::types::point_t<float> p1 { 1.0f, 2.0f, 3.0f };
 *   toolbox::types::point_t<float> p2 { 4.0f, 1.0f, 5.0f };
 *   toolbox::types::minmax_t<toolbox::types::point_t<float>> mm(p1, p2);
 *
 *   // Update with a new point.
 *   toolbox::types::point_t<float> p3 { 0.0f, 3.0f, 2.0f };
 *   mm += p3;
 *   // mm.min and mm.max are updated component-wise.
 *   return 0;
 * }
 * @endcode
 */
template<typename T>  // T is the coordinate type for point_t
struct CPP_TOOLBOX_EXPORT minmax_t<point_t<T>>
{
  point_t<T> min;  ///< The minimum point.
  point_t<T> max;  ///< The maximum point.
  bool initialized_ = false;  ///< Flag to track if the first point was added.

  /**
   * @brief Default constructor.
   *
   * Initializes using point_t's static min_value() and max_value() functions.
   *
   * @code
   * // Example of default construction:
   * toolbox::types::minmax_t<toolbox::types::point_t<double>> mm;
   * // mm.min is set to the point_t<double>::min_value()
   * // mm.max is set to the point_t<double>::max_value()
   * @endcode
   */
  minmax_t()
      : min(point_t<T>::min_value())
      , max(point_t<T>::max_value())
      , initialized_(false)
  {
  }

  /**
   * @brief Constructor with initial values.
   *
   * Marks the instance as initialized.
   *
   * @param initial_min The initial minimum point.
   * @param initial_max The initial maximum point.
   *
   * @code
   * // Example of constructed initialization:
   * toolbox::types::point_t<float> a {1.0f, 2.0f, 3.0f};
   * toolbox::types::point_t<float> b {4.0f, 5.0f, 6.0f};
   * toolbox::types::minmax_t<toolbox::types::point_t<float>> mm(a, b);
   * @endcode
   */
  minmax_t(point_t<T> initial_min, point_t<T> initial_max)
      : min(initial_min)
      , max(initial_max)
      , initialized_(true)
  {
  }

  // Rule of Five members (defaulted)
  minmax_t(const minmax_t&) = default;
  minmax_t(minmax_t&&) noexcept = default;
  minmax_t& operator=(const minmax_t&) = default;
  minmax_t& operator=(minmax_t&&) noexcept = default;
  ~minmax_t() = default;

  /**
   * @brief Updates the minmax_t with a new point.
   *
   * If the instance is not yet initialized, the first point sets both min and
   * max. Otherwise, min and max are updated component-wise.
   *
   * @param value The point to incorporate.
   * @return Reference to the updated minmax_t.
   *
   * @code
   * // Example usage of operator+= for point_t:
   * toolbox::types::minmax_t<toolbox::types::point_t<float>> mm;
   * toolbox::types::point_t<float> point {2.0f, 3.0f, 1.0f};
   * mm += point;
   * @endcode
   */
  minmax_t& operator+=(const point_t<T>& value)
  {
    if (!initialized_) {
      // First point added, set both min and max to this point.
      min = value;
      max = value;
      initialized_ = true;
    } else {
      // Update each component individually.
      min.x = std::min(min.x, value.x);
      min.y = std::min(min.y, value.y);
      min.z = std::min(min.z, value.z);
      max.x = std::max(max.x, value.x);
      max.y = std::max(max.y, value.y);
      max.z = std::max(max.z, value.z);
    }
    return *this;
  }
};

/**
 * @brief Calculates the minimum and maximum values from an input.
 *
 * The input can be a single comparable item or a container of comparable items.
 *
 * @tparam InputType The type of the input (single item or container).
 * @param input The item or container to process.
 * @return minmax_t containing the minimum and maximum values.
 *         For containers, returns a default-constructed minmax_t if empty.
 *
 * @code
 * // Example for calculate_minmax with a container:
 * #include <vector>
 * #include <iostream>
 * #include <cpp-toolbox/types/minmax.hpp>
 *
 * int main() {
 *   std::vector<int> values = {5, 2, 8, 3};
 *   auto result = toolbox::types::calculate_minmax(values);
 *   // Expected output: result.min == 2, result.max == 8
 *   std::cout << "Min: " << result.min << ", Max: " << result.max << std::endl;
 *   return 0;
 * }
 * @endcode
 */
template<typename InputType>
[[nodiscard]] inline auto calculate_minmax(const InputType& input)
{
  // Check if the input is a container we should iterate over.
  if constexpr (detail::is_calculable_container_v<InputType>) {
    // --- Container Logic ---
    using ItemType = typename InputType::value_type;
    minmax_t<ItemType> result;  // Use default constructor.

    // Handle empty container.
    if (std::begin(input) == std::end(input)) {
      return result;
    }

    // Iterate and update using operator+=.
    for (const auto& item : input) {
      result += item;
    }
    return result;
  } else {
    // --- Single Item Logic ---
    // Assume 'input' is a single item. Use decay_t to get the base type.
    using ItemType = std::decay_t<InputType>;
    // Initialize min and max to the same value.
    return minmax_t<ItemType>(input, input);
  }
}

/**
 * @brief Calculates the minimum and maximum values from a point_cloud_t.
 *
 * This function processes a point_cloud_t by calculating the min and max values
 * based on its points.
 *
 * @tparam T The coordinate type of the point_cloud_t.
 * @param input The point_cloud_t to process.
 * @return minmax_t containing the minimum and maximum values of the points.
 *
 * @code
 * // Example for calculate_minmax with point_cloud_t:
 * #include <vector>
 * #include <cpp-toolbox/types/minmax.hpp>
 * #include <cpp-toolbox/types/point.hpp>
 *
 * int main() {
 *   toolbox::types::point_cloud_t<float> cloud;
 *   // Assume cloud.points is a public member vector of point_t<float>
 *   cloud.points.push_back({1.0f, 2.0f, 3.0f});
 *   cloud.points.push_back({4.0f, 1.0f, 5.0f});
 *   auto result = toolbox::types::calculate_minmax(cloud);
 *   // result.min and result.max now hold the respective minimum and maximum
 * points. return 0;
 * }
 * @endcode
 */
template<typename T>
[[nodiscard]] inline auto calculate_minmax(const point_cloud_t<T>& input)
{
  // Directly process the contained points.
  return calculate_minmax(input.points);
}

}  // namespace toolbox::types
