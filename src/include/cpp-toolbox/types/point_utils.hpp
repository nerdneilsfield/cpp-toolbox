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
 * @brief 在给定边界内顺序生成随机点 / Generates random points within given
 * bounds sequentially
 *
 * @tparam T 坐标类型(如float、double) / The coordinate type (e.g., float,
 * double)
 * @param num_points 要生成的点的总数 / Total number of points to generate
 * @param minmax 坐标的最小和最大边界 / The minimum and maximum bounds for
 * coordinates
 * @return std::vector<point_t<T>> 包含生成点的向量 / A vector containing the
 * generated points
 *
 * @code{.cpp}
 * // 生成1000个float类型的随机点 / Generate 1000 random points of float type
 * minmax_t<point_t<float>> bounds({0,0,0}, {10,10,10});
 * auto points = generate_random_points<float>(1000, bounds);
 *
 * // 生成double类型的随机点,带自定义边界 / Generate random points of double
 * type with custom bounds minmax_t<point_t<double>> custom_bounds({-5,-5,-5},
 * {5,5,5}); auto custom_points = generate_random_points<double>(500,
 * custom_bounds);
 * @endcode
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
 * @brief 在给定边界内并行生成随机点 / Generates random points within given
 * bounds in parallel
 *
 * @details 在默认线程池中分配工作,每个线程使用唯一种子的随机数生成器 / Divides
 * the work among threads in the default thread pool, each thread uses its own
 * random number generator seeded uniquely
 *
 * @tparam T 坐标类型(如float、double) / The coordinate type (e.g., float,
 * double)
 * @param num_points 要生成的点的总数 / Total number of points to generate
 * @param minmax 坐标的最小和最大边界 / The minimum and maximum bounds for
 * coordinates
 * @return std::vector<point_t<T>> 包含生成点的向量 / A vector containing the
 * generated points
 *
 * @code{.cpp}
 * // 并行生成大量点 / Generate large number of points in parallel
 * minmax_t<point_t<float>> bounds({0,0,0}, {100,100,100});
 * auto points = generate_random_points_parallel<float>(1000000, bounds);
 *
 * // 使用自定义边界并行生成点 / Generate points in parallel with custom bounds
 * minmax_t<point_t<double>> custom_bounds({-10,-10,-10}, {10,10,10});
 * auto custom_points = generate_random_points_parallel<double>(500000,
 * custom_bounds);
 * @endcode
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

  // 预分配整个向量以避免并行任务中的调整大小 / Pre-allocate the entire vector
  // to avoid resizing in parallel tasks
  std::vector<point_t<T>> points(num_points);

  // --- 使用线程池的并行执行逻辑 / Parallel execution logic using thread pool
  // ---
  auto& pool = toolbox::concurrent::default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());

  // 定义分块策略(根据性能测试调整参数) / Define chunking strategy (adjust
  // parameters based on performance testing)
  const size_t min_chunk_size =
      1024;  // 每个任务的最小点数 / Minimum points per task
  const size_t max_tasks = std::max(
      static_cast<size_t>(1), std::max(num_threads, hardware_threads) * 4);
  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(num_points) / max_tasks)));
  size_t num_tasks = static_cast<size_t>(
      std::ceil(static_cast<double>(num_points) / chunk_size));
  if (num_tasks == 0 && num_points > 0)
    num_tasks = 1;  // 如果num_points > 0,确保至少有一个任务 / Ensure at least
                    // one task if num_points > 0

  std::vector<std::future<void>> futures;
  futures.reserve(num_tasks);

  std::random_device rd;
  unsigned int base_seed =
      rd();  // 生成基础种子以确保线程间的可重现性 / Generate a base seed for
             // reproducibility across threads

  LOG_DEBUG_S << "Parallel generation using " << num_tasks
              << " tasks with chunk size ~" << chunk_size;

  size_t start_idx = 0;
  for (size_t i = 0; i < num_tasks; ++i) {
    size_t remaining_size = num_points - start_idx;
    size_t current_chunk_actual_size = std::min(chunk_size, remaining_size);
    if (current_chunk_actual_size == 0)
      break;  // 如果逻辑正确则不应发生 / Should not happen if logic is correct
    size_t end_idx = start_idx + current_chunk_actual_size;

    // 向线程池提交任务 / Submit task to the thread pool
    futures.emplace_back(pool.submit(
        // 通过值/引用捕获必要的变量 / Capture necessary variables by
        // value/reference 'points'通过引用捕获 - 由于预分配和索引访问是安全的 /
        // 'points' is captured by reference - safe due to pre-allocation and
        // indexed access 'minmax'通过引用捕获(const) / 'minmax' captured by
        // reference (const)
        [start_idx, end_idx, &points, &minmax, base_seed, task_id = i]()
        {
          // --- 线程本地设置 / Thread-local setup ---
          // 必须为每个任务/线程创建分布,因为它们可能保持状态 / Distributions
          // must be created per task/thread as they might hold state
          std::uniform_real_distribution<T> dist_x(minmax.min.x, minmax.max.x);
          std::uniform_real_distribution<T> dist_y(minmax.min.y, minmax.max.y);
          std::uniform_real_distribution<T> dist_z(minmax.min.z, minmax.max.z);

          // 每个任务/线程的随机数生成器使用唯一种子 / Per-task/thread random
          // number generator seeded uniquely 组合基础种子和任务ID提供变化 /
          // Combining base seed and task ID provides variation
          std::mt19937 gen(base_seed + static_cast<unsigned int>(task_id));

          // 可选:记录任务开始(可能很详细) / Optional: Log task start (can be
          // verbose) LOG_TRACE_S << "Task " << task_id << ": Generating points
          // [" << start_idx << ", " << end_idx << ")";

          // --- 为这个块生成点 / Generate points for this chunk ---
          for (size_t k = start_idx; k < end_idx; ++k) {
            // 直接赋值到预分配的向量元素 / Direct assignment to the
            // pre-allocated vector element
            points[k] = point_t<T>(dist_x(gen), dist_y(gen), dist_z(gen));
          }
        }));
    start_idx = end_idx;  // 移动到下一个块的开始 / Move to the next chunk start
  }

  // 等待所有任务完成并处理潜在的异常 / Wait for all tasks to complete and
  // handle potential exceptions
  try {
    for (auto& fut : futures) {
      fut.get();  // `.get()`等待并重新抛出任务中发生的异常 / `.get()` waits and
                  // rethrows exceptions if any occurred in the task
    }
  } catch (const std::exception& e) {
    LOG_ERROR_S << "Exception during parallel point generation: " << e.what();
    // 根据错误处理策略,可以清除点、重新抛出等 / Depending on error handling
    // strategy, could clear points, rethrow, etc. 重新抛出通常是合适的 /
    // Rethrowing is often appropriate
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