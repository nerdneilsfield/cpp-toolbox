#pragma once

#include <algorithm>  // For std::generate_n, std::min, std::max
#include <cmath>  // For std::ceil
#include <functional>  // For std::ref if needed
#include <future>  // For std::future
#include <iterator>  // For std::back_inserter
#include <random>
#include <thread>  // For std::thread::hardware_concurrency (used indirectly via default_pool)
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>  // For CPP_TOOLBOX_EXPORT
#include <cpp-toolbox/logger/thread_logger.hpp>  // For LOG_* macros
#include <cpp-toolbox/types/minmax.hpp>  // Needs minmax_t definition (and includes parallel.hpp)
#include <cpp-toolbox/types/point.hpp>  // Needs point_t definition

namespace toolbox::types
{

/**
 * @brief Generates random points within given bounds sequentially.
 *
 * @tparam T The coordinate type (e.g., float, double).
 * @param num_points Total number of points to generate.
 * @param minmax The minimum and maximum bounds for coordinates.
 * @return std::vector<point_t<T>> A vector containing the generated points.
 */
template<typename T>
CPP_TOOLBOX_EXPORT auto generate_random_points(
    const std::size_t& num_points, const minmax_t<point_t<T>>& minmax)
    -> std::vector<point_t<T>>
{
  std::vector<point_t<T>> points;
  points.reserve(num_points);

  std::random_device rd;
  std::mt19937 gen(rd());

  std::uniform_real_distribution<T> dist_x(minmax.min.x, minmax.max.x);
  std::uniform_real_distribution<T> dist_y(minmax.min.y, minmax.max.y);
  std::uniform_real_distribution<T> dist_z(minmax.min.z, minmax.max.z);

  std::generate_n(
      std::back_inserter(points),
      num_points,
      [&]() { return point_t<T>(dist_x(gen), dist_y(gen), dist_z(gen)); });

  return points;
}

/**
 * @brief Generates random points within given bounds in parallel.
 *
 * Divides the work among threads in the default thread pool.
 * Each thread uses its own random number generator seeded uniquely.
 *
 * @tparam T The coordinate type (e.g., float, double).
 * @param num_points Total number of points to generate.
 * @param minmax The minimum and maximum bounds for coordinates.
 * @return std::vector<point_t<T>> A vector containing the generated points.
 */
template<typename T>
CPP_TOOLBOX_EXPORT auto generate_random_points_parallel(
    const std::size_t& num_points, const minmax_t<point_t<T>>& minmax)
    -> std::vector<point_t<T>>
{
  LOG_DEBUG_S << "Generating " << num_points << " points in parallel.";
  if (num_points == 0) {
    return {};
  }

  // Pre-allocate the entire vector to avoid resizing in parallel tasks
  std::vector<point_t<T>> points(num_points);

  // --- Parallel execution logic using thread pool ---
  auto& pool = toolbox::concurrent::default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());

  // Define chunking strategy (adjust parameters based on performance testing)
  const size_t min_chunk_size = 1024;  // Minimum points per task
  const size_t max_tasks = std::max(
      static_cast<size_t>(1), std::max(num_threads, hardware_threads) * 4);
  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(num_points) / max_tasks)));
  size_t num_tasks = static_cast<size_t>(
      std::ceil(static_cast<double>(num_points) / chunk_size));
  if (num_tasks == 0 && num_points > 0)
    num_tasks = 1;  // Ensure at least one task if num_points > 0

  std::vector<std::future<void>> futures;
  futures.reserve(num_tasks);

  std::random_device rd;
  unsigned int base_seed =
      rd();  // Generate a base seed for reproducibility across threads

  LOG_DEBUG_S << "Parallel generation using " << num_tasks
              << " tasks with chunk size ~" << chunk_size;

  size_t start_idx = 0;
  for (size_t i = 0; i < num_tasks; ++i) {
    size_t remaining_size = num_points - start_idx;
    size_t current_chunk_actual_size = std::min(chunk_size, remaining_size);
    if (current_chunk_actual_size == 0)
      break;  // Should not happen if logic is correct
    size_t end_idx = start_idx + current_chunk_actual_size;

    // Submit task to the thread pool
    futures.emplace_back(pool.submit(
        // Capture necessary variables by value/reference.
        // 'points' is captured by reference - safe due to pre-allocation and
        // indexed access. 'minmax' captured by reference (const).
        [start_idx, end_idx, &points, &minmax, base_seed, task_id = i]()
        {
          // --- Thread-local setup ---
          // Distributions must be created per task/thread as they might hold
          // state
          std::uniform_real_distribution<T> dist_x(minmax.min.x, minmax.max.x);
          std::uniform_real_distribution<T> dist_y(minmax.min.y, minmax.max.y);
          std::uniform_real_distribution<T> dist_z(minmax.min.z, minmax.max.z);

          // Per-task/thread random number generator seeded uniquely
          // Combining base seed and task ID provides variation.
          std::mt19937 gen(base_seed + static_cast<unsigned int>(task_id));

          // Optional: Log task start (can be verbose)
          // LOG_TRACE_S << "Task " << task_id << ": Generating points [" <<
          // start_idx << ", " << end_idx << ")";

          // --- Generate points for this chunk ---
          for (size_t k = start_idx; k < end_idx; ++k) {
            // Direct assignment to the pre-allocated vector element
            points[k] = point_t<T>(dist_x(gen), dist_y(gen), dist_z(gen));
          }
        }));
    start_idx = end_idx;  // Move to the next chunk start
  }

  // Wait for all tasks to complete and handle potential exceptions
  try {
    for (auto& fut : futures) {
      fut.get();  // `.get()` waits and rethrows exceptions if any occurred in
                  // the task
    }
  } catch (const std::exception& e) {
    LOG_ERROR_S << "Exception during parallel point generation: " << e.what();
    // Depending on error handling strategy, could clear points, rethrow, etc.
    // Rethrowing is often appropriate.
    throw;
  } catch (...) {
    LOG_ERROR_S << "Unknown exception during parallel point generation.";
    throw;
  }

  LOG_DEBUG_S << "Finished parallel generation of " << points.size()
              << " points.";
  return points;
}

}  // namespace toolbox::types