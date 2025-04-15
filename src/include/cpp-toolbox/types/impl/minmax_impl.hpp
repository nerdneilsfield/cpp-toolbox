#pragma once

#include <cpp-toolbox/types/minmax.hpp>  // Include the header with declarations

// Include necessary headers for implementation details
#include <algorithm>  // For std::min, std::max
#include <cmath>  // For std::ceil
#include <future>  // For std::future
#include <iostream>  // For std::cerr
#include <iterator>  // For std::begin, std::end, std::distance, std::advance
#include <limits>  // For std::numeric_limits
#include <stdexcept>  // For potential exceptions
#include <thread>  // For std::thread::hardware_concurrency
#include <vector>  // For std::vector

#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::types
{

// --- minmax_t<T> Implementations ---

// General template default constructor (for non-specialized primitive types)
template<typename T>
minmax_t<T>::minmax_t()
    : initialized_(false)
{
  // Check if T has numeric_limits specialization
  if constexpr (std::numeric_limits<T>::is_specialized) {
    // Use lowest() for floating point to handle negative infinity correctly
    if constexpr (std::is_floating_point_v<T>) {
      min = std::numeric_limits<T>::max();  // Start min high
      max =
          std::numeric_limits<T>::lowest();  // Start max low (potentially -inf)
    } else {
      // For integers, use min() and max()
      min = std::numeric_limits<T>::max();  // Start min high
      max = std::numeric_limits<T>::min();  // Start max low
    }
  } else {
    // Fallback for types without numeric_limits (likely custom types)
    min = T {};
    max = T {};
  }
}

// Constructor with initial values
template<typename T>
minmax_t<T>::minmax_t(T initial_min, T initial_max)
    : min(initial_min)
    , max(initial_max)
    , initialized_(true)  // Mark as initialized since values are provided
{
}

// Generic operator+= to update min and max
template<typename T>
minmax_t<T>& minmax_t<T>::operator+=(const T& value)
{
  if (!initialized_) {
    min = value;
    max = value;
    initialized_ = true;
  } else {
    // Use std::min and std::max for comparison
    min = std::min(min, value);
    max = std::max(max, value);
  }
  return *this;
}

// --- minmax_t<point_t<T>> Implementations ---

template<typename T>
minmax_t<point_t<T>>::minmax_t()
    : initialized_(false)
{
  static_assert(std::numeric_limits<T>::is_specialized,
                "Coordinate type T must have specialized std::numeric_limits.");

  // Initialize min components to maximum possible value
  min = point_t<T>(std::numeric_limits<T>::max(),
                   std::numeric_limits<T>::max(),
                   std::numeric_limits<T>::max());

  // Initialize max components to lowest/minimum possible value
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

template<typename T>
minmax_t<point_t<T>>::minmax_t(point_t<T> initial_min, point_t<T> initial_max)
    : min(initial_min)
    , max(initial_max)
    , initialized_(true)  // Mark as initialized
{
}

template<typename T>
minmax_t<point_t<T>>& minmax_t<point_t<T>>::operator+=(const point_t<T>& value)
{
  if (!initialized_) {
    min = value;
    max = value;
    initialized_ = true;
  } else {
    // Update min components
    min.x = std::min(min.x, value.x);
    min.y = std::min(min.y, value.y);
    min.z = std::min(min.z, value.z);

    // Update max components
    max.x = std::max(max.x, value.x);
    max.y = std::max(max.y, value.y);
    max.z = std::max(max.z, value.z);
  }
  return *this;
}

// --- Helper Function Implementations ---

template<typename T>
minmax_t<T> combine_minmax(const minmax_t<T>& a, const minmax_t<T>& b)
{
  if (!a.initialized_)
    return b;
  if (!b.initialized_)
    return a;
  // Both are initialized, combine them
  return minmax_t<T>(std::min(a.min, b.min), std::max(a.max, b.max));
}

template<typename CoordT>
minmax_t<point_t<CoordT>> combine_minmax(const minmax_t<point_t<CoordT>>& a,
                                         const minmax_t<point_t<CoordT>>& b)
{
  if (!a.initialized_)
    return b;
  if (!b.initialized_)
    return a;

  // Both are initialized, combine component-wise
  point_t<CoordT> combined_min;
  point_t<CoordT> combined_max;

  combined_min.x = std::min(a.min.x, b.min.x);
  combined_min.y = std::min(a.min.y, b.min.y);
  combined_min.z = std::min(a.min.z, b.min.z);

  combined_max.x = std::max(a.max.x, b.max.x);
  combined_max.y = std::max(a.max.y, b.max.y);
  combined_max.z = std::max(a.max.z, b.max.z);

  return minmax_t<point_t<CoordT>>(combined_min, combined_max);
}

// --- Sequential calculate_minmax Implementations ---

// For single items (non-containers)
template<typename InputType>
[[nodiscard]] auto calculate_minmax(const InputType& input)
    -> std::enable_if_t<!detail::is_calculable_container_v<InputType>,
                        minmax_t<std::decay_t<InputType>>>
{
  using ItemType = std::decay_t<InputType>;
  // Result is just the item itself as both min and max
  return minmax_t<ItemType>(input,
                            input);  // Directly construct and mark initialized
}

// For calculable containers
template<typename InputType>
[[nodiscard]] auto calculate_minmax(const InputType& input)
    -> std::enable_if_t<detail::is_calculable_container_v<InputType>,
                        minmax_t<typename InputType::value_type>>
{
  using ItemType = typename InputType::value_type;
  minmax_t<ItemType> result;  // Default constructor handles initial state

  auto it = std::begin(input);
  auto end = std::end(input);

  if (it == end) {  // Handle empty container explicitly
    result.initialized_ = false;  // Ensure it's marked uninitialized
    return result;
  }

  // Initialize with the first element - Use operator+= for correct
  // initialization and potential point_t handling
  result += *it;
  ++it;

  // Process remaining elements using operator+= which handles component-wise
  // logic correctly for point_t
  for (; it != end; ++it) {
    result += *it;  // Use operator+= for correct update logic
  }

  return result;
}

// For point_cloud_t
template<typename T>
[[nodiscard]] auto calculate_minmax(const point_cloud_t<T>& input)
    -> minmax_t<point_t<T>>
{
  // Delegate to the container version using the points vector
  return calculate_minmax(input.points);
}

// --- Parallel calculate_minmax Implementations ---

// For single items (delegates to sequential)
template<typename InputType>
[[nodiscard]] auto calculate_minmax_parallel(const InputType& input)
    -> std::enable_if_t<!detail::is_calculable_container_v<InputType>,
                        minmax_t<std::decay_t<InputType>>>
{
  return calculate_minmax(input);  // No benefit from parallel for single item
}

// For calculable containers
template<typename InputType>
[[nodiscard]] auto calculate_minmax_parallel(const InputType& input)
    -> std::enable_if_t<detail::is_calculable_container_v<InputType>,
                        minmax_t<typename InputType::value_type>>
{
  using ItemType = typename InputType::value_type;
  using ResultType = minmax_t<ItemType>;

  const auto total_size_signed =
      std::distance(std::begin(input), std::end(input));
  constexpr size_t sequential_threshold =
      1024;  // Threshold for switching to sequential

  if (total_size_signed <= 0) {  // Handle empty range
    return ResultType();  // Return default (uninitialized)
  }
  const size_t total_size = static_cast<size_t>(total_size_signed);

  if (total_size < sequential_threshold) {
    return calculate_minmax(input);  // Fallback to sequential
  }

  // --- Parallel Execution Logic ---
  auto& pool = toolbox::concurrent::default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());
  const size_t min_chunk_size = 256;
  // Determine number of tasks, aiming for reasonable granularity
  const size_t max_tasks = std::max(
      static_cast<size_t>(1), std::max(num_threads, hardware_threads) * 4);
  const size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(total_size) / max_tasks)));
  size_t num_tasks = (total_size == 0)
      ? 0
      : static_cast<size_t>(
            std::ceil(static_cast<double>(total_size) / chunk_size));
  if (num_tasks == 0 && total_size > 0)
    num_tasks = 1;  // Ensure at least one task if not empty

  std::vector<std::future<ResultType>> futures;
  if (num_tasks == 0) {
    return ResultType();
  }
  futures.reserve(num_tasks);

  auto task_lambda = [](auto chunk_begin_it, auto chunk_end_it) -> ResultType
  {
    ResultType local_result;
    if (chunk_begin_it == chunk_end_it) {  // Handle empty chunk
      local_result.initialized_ = false;
      return local_result;
    }

    // Use operator+= for the first element and subsequent elements
    auto it = chunk_begin_it;
    local_result += *it;  // Initialize using operator+=
    ++it;
    for (; it != chunk_end_it; ++it) {
      local_result += *it;  // Update using operator+=
    }
    return local_result;
  };

  auto first = std::begin(input);
  for (size_t i = 0; i < num_tasks; ++i) {
    size_t start_offset = i * chunk_size;
    size_t current_chunk_size = std::min(chunk_size, total_size - start_offset);

    if (current_chunk_size == 0)
      break;  // No more elements

    auto chunk_begin = first;
    std::advance(chunk_begin, start_offset);
    auto chunk_end = chunk_begin;
    std::advance(chunk_end, current_chunk_size);

    futures.emplace_back(pool.submit(task_lambda, chunk_begin, chunk_end));
  }

  // Reduce the results
  ResultType final_result;  // Default constructed (uninitialized)
  try {
    for (auto& fut : futures) {
      ResultType partial_result = fut.get();
      // Combine using the helper function which handles initialization state
      final_result = combine_minmax(final_result, partial_result);
    }
  } catch (const std::exception& e) {
    std::cerr << "Exception during parallel minmax reduction: " << e.what()
              << '\n';
    throw;  // Rethrow for now
  } catch (...) {
    std::cerr << "Unknown exception during parallel minmax reduction.\n";
    throw;  // Rethrow for now
  }

  return final_result;
}

// For point_cloud_t (delegates to parallel container version)
template<typename T>
[[nodiscard]] auto calculate_minmax_parallel(const point_cloud_t<T>& input)
    -> minmax_t<point_t<T>>
{
  return calculate_minmax_parallel(input.points);
}

}  // namespace toolbox::types