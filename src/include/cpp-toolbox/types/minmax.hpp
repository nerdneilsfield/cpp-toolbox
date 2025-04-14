#pragma once

#include <algorithm>  // For std::min and std::max
#include <cmath>  // For std::ceil
#include <future>  // For std::future
#include <iostream>  // For std::cerr in parallel exception handling
#include <iterator>  // For std::begin, std::end, std::distance, std::advance
#include <limits>
#include <string>
#include <thread>  // For std::thread::hardware_concurrency
#include <type_traits>  // For type traits used in detail namespace
#include <utility>
#include <vector>  // For std::vector used in parallel logic

#include <cpp-toolbox/base/thread_pool_singleton.hpp>  // Thread pool required
#include <cpp-toolbox/concurrent/parallel.hpp>  // For default_pool()
#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/io/formats/base.hpp>
#include <cpp-toolbox/types/point.hpp>

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
    : std::bool_constant<
          !std::is_same_v<std::decay_t<T>, std::string>
          /* && !std::is_same_v<std::decay_t<T>, std::wstring> */>
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
  bool initialized_ = false;  ///< Flag to track if the first point was added.

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
      , initialized_(true)
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
    if (!initialized_) {
      min = value;
      max = value;
      initialized_ = true;
    } else {
      min = std::min(min, value);
      max = std::max(max, value);
    }
    return *this;
  }
};

// Generic default constructor definition (for types without specialization)
template<typename T>
CPP_TOOLBOX_EXPORT minmax_t<T>::minmax_t()
    : initialized_(false)
{
  if constexpr (std::numeric_limits<T>::is_specialized) {
    if constexpr (std::is_floating_point_v<T>) {
      min = std::numeric_limits<T>::lowest();
      max = std::numeric_limits<T>::max();
    } else {
      min = std::numeric_limits<T>::min();
      max = std::numeric_limits<T>::max();
    }
  } else {
    min = T {};
    max = T {};
  }
}

// --- Full specializations for primitive types' default constructors ---
template<>
CPP_TOOLBOX_EXPORT minmax_t<int>::minmax_t()
    : min(std::numeric_limits<int>::max())
    , max(std::numeric_limits<int>::min())
    , initialized_(false)
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<float>::minmax_t()
    : min(std::numeric_limits<float>::max())
    , max(std::numeric_limits<float>::lowest())
    , initialized_(false)
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<double>::minmax_t()
    : min(std::numeric_limits<double>::max())
    , max(std::numeric_limits<double>::lowest())
    , initialized_(false)
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<long>::minmax_t()
    : min(std::numeric_limits<long>::max())
    , max(std::numeric_limits<long>::min())
    , initialized_(false)
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned int>::minmax_t()
    : min(std::numeric_limits<unsigned int>::max())
    , max(std::numeric_limits<unsigned int>::min())
    , initialized_(false)
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned long>::minmax_t()
    : min(std::numeric_limits<unsigned long>::max())
    , max(std::numeric_limits<unsigned long>::min())
    , initialized_(false)
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned long long>::minmax_t()
    : min(std::numeric_limits<unsigned long long>::max())
    , max(std::numeric_limits<unsigned long long>::min())
    , initialized_(false)
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned char>::minmax_t()
    : min(std::numeric_limits<unsigned char>::max())
    , max(std::numeric_limits<unsigned char>::min())
    , initialized_(false)
{
}

template<>
CPP_TOOLBOX_EXPORT minmax_t<unsigned short>::minmax_t()
    : min(std::numeric_limits<unsigned short>::max())
    , max(std::numeric_limits<unsigned short>::min())
    , initialized_(false)
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
   * Initializes min components to the maximum possible value of T,
   * and max components to the lowest possible value of T (or min for integers).
   * This ensures correct comparison behavior when combining results.
   *
   * @code
   * // Example of default construction:
   * toolbox::types::minmax_t<toolbox::types::point_t<double>> mm;
   * // mm.min components are initialized to std::numeric_limits<double>::max()
   * // mm.max components are initialized to
   * std::numeric_limits<double>::lowest()
   * @endcode
   */
  minmax_t()
      : initialized_(false)
  {
    // Ensure the coordinate type has numeric limits defined.
    static_assert(
        std::numeric_limits<T>::is_specialized,
        "Coordinate type T must have specialized std::numeric_limits.");

    // Initialize min components to the highest possible value.
    min = point_t<T>(std::numeric_limits<T>::max(),
                     std::numeric_limits<T>::max(),
                     std::numeric_limits<T>::max());

    // Initialize max components to the lowest possible value.
    if constexpr (std::is_floating_point_v<T>) {
      max = point_t<T>(std::numeric_limits<T>::lowest(),
                       std::numeric_limits<T>::lowest(),
                       std::numeric_limits<T>::lowest());
    } else {  // Assuming integral type otherwise
      max = point_t<T>(std::numeric_limits<T>::min(),
                       std::numeric_limits<T>::min(),
                       std::numeric_limits<T>::min());
    }
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
  minmax_t& operator+=(const point_t<T>& value);  // Declaration only
};

// --- Definition of operator+= for minmax_t<point_t<T>> ---
template<typename T>
CPP_TOOLBOX_EXPORT minmax_t<point_t<T>>& minmax_t<point_t<T>>::operator+=(
    const point_t<T>& value)
{
  if (!initialized_) {
    min = value;
    max = value;
    initialized_ = true;
  } else {
    // Component-wise update
    min.x = std::min(min.x, value.x);
    min.y = std::min(min.y, value.y);
    min.z = std::min(min.z, value.z);

    max.x = std::max(max.x, value.x);
    max.y = std::max(max.y, value.y);
    max.z = std::max(max.z, value.z);
  }
  return *this;
}

// --- Helper functions for combining minmax results (Now in main namespace) ---

// Helper function to combine two minmax results, handling initialization state
template<typename T>
minmax_t<T> combine_minmax(const minmax_t<T>& a, const minmax_t<T>& b)
{
  if (!a.initialized_)
    return b;
  if (!b.initialized_)
    return a;
  return minmax_t<T>(std::min(a.min, b.min), std::max(a.max, b.max));
}

// Specialization for point_t - combine component-wise
template<typename CoordT>
minmax_t<point_t<CoordT>> combine_minmax(const minmax_t<point_t<CoordT>>& a,
                                         const minmax_t<point_t<CoordT>>& b)
{
  if (!a.initialized_)
    return b;
  if (!b.initialized_)
    return a;

  // Start with 'a' as the base result
  minmax_t<point_t<CoordT>> result = a;

  // Update min components
  result.min.x = std::min(result.min.x, b.min.x);
  result.min.y = std::min(result.min.y, b.min.y);
  result.min.z = std::min(result.min.z, b.min.z);

  // Update max components
  result.max.x = std::max(result.max.x, b.max.x);
  result.max.y = std::max(result.max.y, b.max.y);
  result.max.z = std::max(result.max.z, b.max.z);

  // result.initialized_ remains true as 'a' was initialized.
  return result;
}

// --- Sequential calculate_minmax --- (This is the always-sequential version)

/**
 * @brief Calculates min/max for a single item (sequential).
 *
 * This function calculates the minimum and maximum values for a single input
 * value. It simply initializes both the min and max with the given input.
 *
 * Example:
 * @code{.cpp}
 * int value = 10;
 * auto mm = calculate_minmax(value);
 * // mm.min == 10 and mm.max == 10
 * @endcode
 */
template<typename InputType>
[[nodiscard]] CPP_TOOLBOX_EXPORT inline auto calculate_minmax(
    const InputType& input)
    -> std::enable_if_t<!detail::is_calculable_container_v<InputType>,
                        minmax_t<std::decay_t<InputType>>>
{
  using ItemType = std::decay_t<InputType>;
  return minmax_t<ItemType>(input, input);
}

/**
 * @brief Calculates min/max for a container sequentially.
 *
 * This function iterates over a container and computes the minimum and maximum
 * by combining each element using the overloaded operator +=.
 *
 * Example:
 * @code{.cpp}
 * std::vector<int> numbers = {3, 7, 2, 9, -1};
 * auto mm = calculate_minmax(numbers);
 * // Expected output: mm.min == -1 and mm.max == 9
 * @endcode
 */
template<typename InputType>
[[nodiscard]] CPP_TOOLBOX_EXPORT inline auto calculate_minmax(
    const InputType& input)
    -> std::enable_if_t<detail::is_calculable_container_v<InputType>,
                        minmax_t<typename InputType::value_type>>
{
  using ItemType = typename InputType::value_type;
  minmax_t<ItemType> result;
  // No need to check for empty; the operator += handles initialization.
  for (const auto& item : input) {
    result += item;
  }
  return result;
}

/**
 * @brief Calculates min/max for a point_cloud_t sequentially.
 *
 * This function retrieves the points from a point_cloud_t structure and
 * calculates the minimum and maximum values by calling the sequential container
 * version.
 *
 * Example:
 * @code{.cpp}
 * point_cloud_t<double> cloud;
 * cloud.points.push_back(point_t<double>(1.0, 5.0, -2.0));
 * cloud.points.push_back(point_t<double>(-3.0, 6.0, 4.0));
 * auto mm = calculate_minmax(cloud);
 * // The min and max values are determined based on the points in the cloud.
 * @endcode
 */
template<typename T>
[[nodiscard]] CPP_TOOLBOX_EXPORT inline auto calculate_minmax(
    const point_cloud_t<T>& input)
{
  return calculate_minmax(input.points);  // Calls sequential container version
}

/**
 * @brief Calculates min/max for a single item (parallel - calls sequential).
 *
 * Since processing a single item does not benefit from parallel execution,
 * this function simply forwards the call to the sequential calculate_minmax.
 *
 * Example:
 * @code{.cpp}
 * float value = 3.14f;
 * auto mm = calculate_minmax_parallel(value);
 * // mm.min == 3.14f and mm.max == 3.14f
 * @endcode
 */
template<typename InputType>
[[nodiscard]] CPP_TOOLBOX_EXPORT inline auto calculate_minmax_parallel(
    const InputType& input)
    -> std::enable_if_t<!detail::is_calculable_container_v<InputType>,
                        minmax_t<std::decay_t<InputType>>>
{
  return calculate_minmax(input);  // Single item is inherently sequential
}

/**
 * @brief Calculates min/max for a container in parallel.
 *
 * This function divides the input container into chunks and processes each
 * chunk in parallel. If the total size of the container is less than a preset
 * threshold, it falls back to the sequential version.
 *
 * Example:
 * @code{.cpp}
 * std::vector<int> vec = {5, -3, 8, 0};
 * auto mm = calculate_minmax_parallel(vec);
 * // For a small container, the sequential version is used.
 * // Expected output: mm.min == -3 and mm.max == 8
 * @endcode
 */
template<typename InputType>
[[nodiscard]] CPP_TOOLBOX_EXPORT inline auto calculate_minmax_parallel(
    const InputType& input)
    -> std::enable_if_t<detail::is_calculable_container_v<InputType>,
                        minmax_t<typename InputType::value_type>>
{
  using ItemType = typename InputType::value_type;
  using ResultType = minmax_t<ItemType>;

  const auto total_size = std::distance(std::begin(input), std::end(input));
  constexpr size_t sequential_threshold = 1024;

  // Use sequential version if empty or below threshold
  if (total_size < sequential_threshold) {
    // std::cout << "[DEBUG] Parallel fallback to sequential for size: " <<
    // total_size << std::endl;
    return calculate_minmax(input);  // Fallback to sequential execution
  }

  // --- Parallel execution logic (Map-Reduce style) ---
  // std::cout << "[DEBUG] Parallel execution for size: " << total_size <<
  // std::endl;
  auto& pool = toolbox::concurrent::default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());
  const size_t min_chunk_size = 256;
  const size_t max_tasks = std::max(
      static_cast<size_t>(1), std::max(num_threads, hardware_threads) * 4);
  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(total_size) / max_tasks)));
  size_t num_tasks = static_cast<size_t>(
      std::ceil(static_cast<double>(total_size) / chunk_size));
  if (num_tasks == 0)
    num_tasks = 1;

  std::vector<std::future<ResultType>> futures;
  futures.reserve(num_tasks);

  auto chunk_begin_it = std::begin(input);
  for (size_t i = 0; i < num_tasks && chunk_begin_it != std::end(input); ++i) {
    auto chunk_end_it = chunk_begin_it;
    size_t current_chunk_actual_size = std::min(
        chunk_size,
        static_cast<size_t>(std::distance(chunk_begin_it, std::end(input))));
    if (current_chunk_actual_size == 0)
      break;
    std::advance(chunk_end_it, current_chunk_actual_size);
    // std::cout << "[DEBUG] Submitting task " << i << " for range size " <<
    // current_chunk_actual_size << std::endl;

    futures.emplace_back(pool.submit(
        [begin = chunk_begin_it, end = chunk_end_it, task_id = i]()
            -> ResultType
        {
          ResultType local_result;
          for (auto it = begin; it != end; ++it) {
            const auto& value = *it;
            // Simply use the appropriate operator+=, which is specialized for
            // point_t
            local_result += value;
          }

          // Keep the debug output section if needed, adjust if ItemType might
          // not be point_t<double>
          using ItemType =
              decltype(local_result.min);  // Determine ItemType for debug check
          if constexpr (std::is_same_v<ItemType, point_t<double>>) {
            if (local_result.initialized_) {
              std::cout << "[DEBUG] Task " << task_id
                        << " Local Max: (y=" << local_result.max.y
                        << ", z=" << local_result.max.z << ")" << std::endl;
            } else {
              std::cout << "[DEBUG] Task " << task_id
                        << " Local result not initialized." << std::endl;
            }
          }
          return local_result;
        }));
    chunk_begin_it = chunk_end_it;
  }

  ResultType final_result;
  try {
    for (int fut_idx = 0; fut_idx < futures.size(); ++fut_idx)
    {  // Use index for clarity
      ResultType partial_result = futures[fut_idx].get();
      // DEBUG: Print results before and after combining
      if constexpr (std::is_same_v<ItemType, point_t<double>>) {
        std::cout << "[DEBUG] Reducing future " << fut_idx << ":" << std::endl;
        std::cout << "  Final (Before): initialized="
                  << final_result.initialized_
                  << " max=(y=" << final_result.max.y
                  << ", z=" << final_result.max.z << ")" << std::endl;
        std::cout << "  Partial:        initialized="
                  << partial_result.initialized_
                  << " max=(y=" << partial_result.max.y
                  << ", z=" << partial_result.max.z << ")" << std::endl;
      }
      final_result = combine_minmax(final_result, partial_result);
      if constexpr (std::is_same_v<ItemType, point_t<double>>) {
        std::cout << "  Final (After):  initialized="
                  << final_result.initialized_
                  << " max=(y=" << final_result.max.y
                  << ", z=" << final_result.max.z << ")" << std::endl;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception during parallel minmax reduction: " << e.what()
              << '\n';
    throw;
  } catch (...) {
    std::cerr << "Unknown exception during parallel minmax reduction.\n";
    throw;
  }
  // std::cout << "[DEBUG] Parallel execution finished." << std::endl;
  return final_result;
}

/**
 * @brief Calculates min/max for a point_cloud_t in parallel.
 *
 * This function extracts the underlying points from a point_cloud_t and
 * applies the parallel container version of calculate_minmax on them.
 *
 * Example:
 * @code{.cpp}
 * point_cloud_t<float> cloud;
 * cloud.points.push_back(point_t<float>(1.0f, 2.0f, 3.0f));
 * cloud.points.push_back(point_t<float>(-1.0f, 4.0f, 0.0f));
 * auto mm = calculate_minmax_parallel(cloud);
 * // The min and max values are calculated using parallel processing on
 * cloud.points.
 * @endcode
 */
template<typename T>
[[nodiscard]] CPP_TOOLBOX_EXPORT inline auto calculate_minmax_parallel(
    const point_cloud_t<T>& input)
{
  // Calls the parallel container version by using the points member.
  return calculate_minmax_parallel(input.points);
}

}  // namespace toolbox::types
