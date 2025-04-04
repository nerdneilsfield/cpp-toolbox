#include <algorithm>  // for std::min
#include <cmath>  // for std::ceil
#include <future>
#include <iterator>
#include <numeric>  // for std::accumulate (in reduce example)
#include <stdexcept>  // for exceptions
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/base/thread_pool_singleton.hpp"  // 需要线程池

namespace toolbox::concurrent
{

// --- 辅助函数：获取默认线程池实例 ---
inline base::thread_pool_singleton_t& default_pool()
{
  // 使用单例模式获取线程池
  return base::thread_pool_singleton_t::instance();
}

//--------------------------------------------------------------------------
// parallel_for_each
//--------------------------------------------------------------------------

/**
 * @brief 并行地对范围 `[begin, end)` 中的每个元素应用函数 `func`。
 * @details 使用默认的线程池进行手动任务划分和提交。
 * @tparam Iterator 输入迭代器类型（需要支持随机访问以进行高效划分）。
 * @tparam Function 一元函数对象类型。
 * @param begin 范围的起始迭代器。
 * @param end 范围的结束迭代器。
 * @param func 应用于每个元素的函数对象。
 */
template<typename Iterator, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(Iterator begin,
                                          Iterator end,
                                          Function func)
{
  using traits = std::iterator_traits<Iterator>;
  // 基础检查，随机访问迭代器效率最高
  static_assert(std::is_base_of_v<std::random_access_iterator_tag,
                                  typename traits::iterator_category>,
                "parallel_for_each currently requires random access iterators "
                "for efficient chunking.");

  const auto total_size = std::distance(begin, end);
  if (total_size <= 0) {
    return;
  }

  auto& pool = default_pool();
  const size_t num_threads = pool.get_thread_count();
  // 确定合适的任务数量，例如线程数的 2-4 倍，或根据数据量决定
  // 简单的策略：每个线程至少处理一个块，尽量多分一些块
  const size_t min_chunk_size = 1;  // 或者更大，避免任务太小开销过大
  const size_t hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());  // 保证至少为1
  const size_t max_tasks = std::max(num_threads, hardware_threads)
      * 4;  // 简单策略：4倍线程数任务上限

  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(total_size) / max_tasks)));
  size_t num_tasks = static_cast<size_t>(
      std::ceil(static_cast<double>(total_size) / chunk_size));

  std::vector<std::future<void>> futures;
  futures.reserve(num_tasks);

  Iterator chunk_begin = begin;
  for (size_t i = 0; i < num_tasks; ++i) {
    Iterator chunk_end = chunk_begin;
    // 计算当前块的结束位置，注意不要越过总结束位置 end
    size_t current_chunk_actual_size = std::min(
        chunk_size, static_cast<size_t>(std::distance(chunk_begin, end)));
    if (current_chunk_actual_size == 0)
      break;  // 如果剩余元素不足一个 chunk_size，可能导致死循环或计算错误
    std::advance(chunk_end, current_chunk_actual_size);

    // 提交处理当前块的任务
    futures.emplace_back(pool.submit([chunk_begin, chunk_end, func]() mutable { // mutable for func if needed
            for (auto it = chunk_begin; it != chunk_end; ++it) {
                func(*it);
            }
        }));

    chunk_begin = chunk_end;  // 移动到下一个块的开始

    if (chunk_begin == end) {  // 如果已经处理到末尾，则停止创建任务
      break;
    }
  }

  // 等待所有任务完成并处理异常
  try {
    for (auto& fut : futures) {
      fut.get();  // get() 会重新抛出任务中未捕获的异常
    }
  } catch (...) {
    // 可以选择记录日志，然后重新抛出，或者根据策略处理
    // LOG_ERROR_S << "Exception caught during parallel_for_each execution.";
    // std::throw_with_nested(std::runtime_error("Parallel execution failed"));
    throw;  // 简单地重新抛出第一个遇到的异常
  }
}

// 为 vector/array 提供便捷重载
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(std::vector<T, Alloc>& vec,
                                          Function func)
{
  parallel_for_each(vec.begin(), vec.end(), std::move(func));
}
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(const std::vector<T, Alloc>& vec,
                                          Function func)
{
  parallel_for_each(vec.cbegin(), vec.cend(), std::move(func));
}

template<typename T, size_t N, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(std::array<T, N>& arr, Function func)
{
  parallel_for_each(arr.begin(), arr.end(), std::move(func));
}
template<typename T, size_t N, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(const std::array<T, N>& arr,
                                          Function func)
{
  parallel_for_each(arr.cbegin(), arr.cend(), std::move(func));
}

//--------------------------------------------------------------------------
// parallel_transform
//--------------------------------------------------------------------------

/**
 * @brief 并行地对输入范围 `[first1, last1)` 应用 `unary_op`，并将结果存储到
 * `[d_first, ...)`。
 * @warning 要求输入和输出迭代器都必须是【随机访问迭代器】。
 * @warning 输出范围 `[d_first, d_first + distance(first1, last1))`
 * 必须具有足够的、已分配的容量，并且并行写入必须是安全的（例如，写入
 * `std::vector` 前需 `resize()`）。
 * @tparam InputIt 输入迭代器类型 (std::random_access_iterator)。
 * @tparam OutputIt 输出迭代器类型 (std::random_access_iterator)。
 * @tparam UnaryOperation 一元操作函数对象类型, 签名应类似 OutType(const
 * InType&)。
 * @param first1 输入范围的起始。
 * @param last1 输入范围的结束。
 * @param d_first 输出范围的起始。
 * @param unary_op 应用于每个输入元素的操作。
 */
template<typename InputIt, typename OutputIt, typename UnaryOperation>
CPP_TOOLBOX_EXPORT void parallel_transform(InputIt first1,
                                           InputIt last1,
                                           OutputIt d_first,
                                           UnaryOperation unary_op)
{
  using InputTraits = std::iterator_traits<InputIt>;
  using OutputTraits = std::iterator_traits<OutputIt>;

  static_assert(
      std::is_base_of_v<std::random_access_iterator_tag,
                        typename InputTraits::iterator_category>,
      "parallel_transform currently requires random access input iterators.");
  static_assert(
      std::is_base_of_v<std::random_access_iterator_tag,
                        typename OutputTraits::iterator_category>,
      "parallel_transform currently requires random access output iterators.");

  const auto total_size = std::distance(first1, last1);
  if (total_size <= 0) {
    return;
  }

  auto& pool = default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t min_chunk_size = 1;
  const size_t hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());
  const size_t max_tasks = std::max(num_threads, hardware_threads) * 4;

  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(total_size) / max_tasks)));
  size_t num_tasks = static_cast<size_t>(
      std::ceil(static_cast<double>(total_size) / chunk_size));

  std::vector<std::future<void>> futures;
  futures.reserve(num_tasks);

  size_t chunk_start_index = 0;
  for (size_t i = 0; i < num_tasks; ++i) {
    size_t chunk_end_index = std::min(chunk_start_index + chunk_size,
                                      static_cast<size_t>(total_size));

    if (chunk_start_index >= chunk_end_index)
      break;  // 没有更多元素了

    // 提交处理当前块的任务
    // 捕获必要的变量，注意 unary_op 按值捕获通常更安全简单
    futures.emplace_back(pool.submit(
        [first1, d_first, chunk_start_index, chunk_end_index, unary_op]()
        {
          for (size_t k = chunk_start_index; k < chunk_end_index; ++k) {
            // 使用迭代器偏移（需要随机访问迭代器）
            *(d_first + k) = unary_op(*(first1 + k));
          }
        }));

    chunk_start_index = chunk_end_index;  // 移动到下一个块的开始
    if (chunk_start_index >= static_cast<size_t>(total_size))
      break;  // 已处理完所有元素
  }

  // 等待所有任务完成并处理异常
  try {
    for (auto& fut : futures) {
      fut.get();
    }
  } catch (...) {
    // LOG_ERROR_S << "Exception caught during parallel_transform execution.";
    // std::throw_with_nested(std::runtime_error("Parallel execution failed"));
    throw;
  }
}

// 为 vector/array 提供便捷重载
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(std::vector<T, Alloc>& vec,
                                           Function func)
{
  parallel_transform(vec.begin(), vec.end(), std::move(func));
}

template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(const std::vector<T, Alloc>& vec,
                                           Function func)
{
  parallel_transform(vec.cbegin(), vec.cend(), std::move(func));
}

template<typename T, size_t N, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(std::array<T, N>& arr, Function func)
{
  parallel_transform(arr.begin(), arr.end(), std::move(func));
}

template<typename T, size_t N, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(const std::array<T, N>& arr,
                                           Function func)
{
  parallel_transform(arr.cbegin(), arr.cend(), std::move(func));
}

//--------------------------------------------------------------------------
// parallel_reduce
//--------------------------------------------------------------------------

/**
 * @brief 并行地对范围 `[begin, end)` 进行归约操作。
 * @details 使用默认线程池进行手动任务划分，先进行局部归约，然后合并结果。
 * `reduce_op` 最好满足结合律 (associative)。`identity` 必须是 `reduce_op`
 * 的幺元。
 * @tparam Iterator 输入迭代器类型（需要支持随机访问以进行高效划分）。
 * @tparam T 归约结果和幺元的类型。
 * @tparam BinaryOperation 归约操作的二元函数对象类型，签名应类似 `T(const T&,
 * const ElementType&)` 或 `T(T, T)`。
 * @param begin 范围的起始迭代器。
 * @param end 范围的结束迭代器。
 * @param identity 归约操作的幺元。
 * @param reduce_op 用于合并两个 T 类型值或 T 类型值与元素类型值的二元操作。
 * @return 并行归约的结果。
 */
template<typename Iterator, typename T, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(Iterator begin,
                                     Iterator end,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  using traits = std::iterator_traits<Iterator>;
  static_assert(std::is_base_of_v<std::random_access_iterator_tag,
                                  typename traits::iterator_category>,
                "parallel_reduce currently requires random access iterators "
                "for efficient chunking.");

  const auto total_size = std::distance(begin, end);
  if (total_size <= 0) {
    return identity;  // 空范围直接返回幺元
  }

  auto& pool = default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t min_chunk_size =
      256;  // 对于 reduce，块太小可能不划算，设个稍大的下限
  const size_t hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());
  // reduce 的任务数可以少一些，避免最后合并过多结果
  const size_t num_tasks = std::max(
      1ul,
      std::min(num_threads, hardware_threads));  // 简单策略：任务数等于线程数

  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(total_size) / num_tasks)));
  // 重新计算实际任务数
  size_t actual_num_tasks = static_cast<size_t>(
      std::ceil(static_cast<double>(total_size) / chunk_size));

  std::vector<std::future<T>> futures;
  futures.reserve(actual_num_tasks);

  Iterator chunk_begin = begin;
  for (size_t i = 0; i < actual_num_tasks; ++i) {
    Iterator chunk_end = chunk_begin;
    size_t current_chunk_actual_size = std::min(
        chunk_size, static_cast<size_t>(std::distance(chunk_begin, end)));
    if (current_chunk_actual_size == 0)
      break;
    std::advance(chunk_end, current_chunk_actual_size);

    // 提交计算局部归约值的任务
    futures.emplace_back(pool.submit(
        [chunk_begin, chunk_end, reduce_op, identity]() -> T
        {
          // 检查块是否为空，为空则返回 identity (作为此空块的归约结果)
          if (chunk_begin == chunk_end) {
            return identity;
          }

          // 从块的第一个元素开始归约
          auto it = chunk_begin;
          T local_result = *it;  // 使用第一个元素初始化
          ++it;  // 移动到第二个元素

          // 累加块内剩余元素
          for (; it != chunk_end; ++it) {
            local_result = reduce_op(local_result, *it);
          }
          return local_result;  // 返回这个块内元素的归约结果
        }));

    chunk_begin = chunk_end;
    if (chunk_begin == end)
      break;
  }

  // 合并所有局部结果
  T final_result = identity;
  try {
    for (auto& fut : futures) {
      T partial_result = fut.get();  // 等待并获取局部结果
      final_result = reduce_op(final_result, partial_result);  // 合并
    }
  } catch (...) {
    // LOG_ERROR_S << "Exception caught during parallel_reduce execution.";
    // std::throw_with_nested(std::runtime_error("Parallel execution failed"));
    throw;
  }

  return final_result;
}

// 为 vector/array 提供便捷重载
template<typename T, typename Alloc, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(std::vector<T, Alloc>& vec,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      vec.begin(), vec.end(), identity, std::move(reduce_op));
}

template<typename T, typename Alloc, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(const std::vector<T, Alloc>& vec,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      vec.cbegin(), vec.cend(), identity, std::move(reduce_op));
}

template<typename T, size_t N, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(std::array<T, N>& arr,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      arr.begin(), arr.end(), identity, std::move(reduce_op));
}

template<typename T, size_t N, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(const std::array<T, N>& arr,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      arr.cbegin(), arr.cend(), identity, std::move(reduce_op));
}

}  // namespace toolbox::concurrent