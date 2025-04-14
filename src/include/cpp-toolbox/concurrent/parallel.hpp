#include <algorithm>  // for std::min
#include <cmath>  // for std::ceil
#include <future>
#include <iterator>
#include <numeric>  // for std::accumulate (in reduce example)
#include <stdexcept>  // for exceptions
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/base/thread_pool_singleton.hpp"  // Thread pool required

namespace toolbox::concurrent
{

/**
 * @brief Get the default thread pool instance
 * @return Reference to the singleton thread pool instance
 */
inline base::thread_pool_singleton_t& default_pool()
{
  return base::thread_pool_singleton_t::instance();
}

//--------------------------------------------------------------------------
// parallel_for_each
//--------------------------------------------------------------------------

/**
 * @brief Applies a function to each element in range [begin, end) in parallel
 * @details Uses default thread pool for manual task division and submission
 * @tparam Iterator Input iterator type (must support random access for
 * efficient chunking)
 * @tparam Function Unary function object type
 * @param begin Start iterator of range
 * @param end End iterator of range
 * @param func Function object to apply to each element
 *
 * @code{.cpp}
 * std::vector<int> vec = {1, 2, 3, 4, 5};
 *
 * // Double each element in parallel
 * parallel_for_each(vec.begin(), vec.end(), [](int& x) {
 *   x *= 2;
 * });
 * // vec now contains {2, 4, 6, 8, 10}
 * @endcode
 */
template<typename Iterator, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(Iterator begin,
                                          Iterator end,
                                          Function func)
{
  using traits = std::iterator_traits<Iterator>;
  static_assert(std::is_base_of<std::random_access_iterator_tag,
                                typename traits::iterator_category>::value,
                "parallel_for_each currently requires random access iterators "
                "for efficient chunking.");

  const auto total_size = std::distance(begin, end);
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

  Iterator chunk_begin = begin;
  for (size_t i = 0; i < num_tasks; ++i) {
    Iterator chunk_end = chunk_begin;
    size_t current_chunk_actual_size = std::min(
        chunk_size, static_cast<size_t>(std::distance(chunk_begin, end)));
    if (current_chunk_actual_size == 0)
      break;
    std::advance(chunk_end, current_chunk_actual_size);

    futures.emplace_back(pool.submit(
        [chunk_begin, chunk_end, func]() mutable
        {
          for (auto it = chunk_begin; it != chunk_end; ++it) {
            func(*it);
          }
        }));

    chunk_begin = chunk_end;

    if (chunk_begin == end) {
      break;
    }
  }

  try {
    for (auto& fut : futures) {
      fut.get();
    }
  } catch (...) {
    throw;
  }
}

/**
 * @brief Convenience overload for vectors
 * @tparam T Vector element type
 * @tparam Alloc Vector allocator type
 * @tparam Function Function object type
 * @param vec Vector to process
 * @param func Function to apply
 */
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(std::vector<T, Alloc>& vec,
                                          Function func)
{
  parallel_for_each(vec.begin(), vec.end(), std::move(func));
}

/**
 * @brief Convenience overload for const vectors
 */
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(const std::vector<T, Alloc>& vec,
                                          Function func)
{
  parallel_for_each(vec.cbegin(), vec.cend(), std::move(func));
}

/**
 * @brief Convenience overload for arrays
 */
template<typename T, size_t N, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(std::array<T, N>& arr, Function func)
{
  parallel_for_each(arr.begin(), arr.end(), std::move(func));
}

/**
 * @brief Convenience overload for const arrays
 */
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
 * @brief Transforms elements from input range to output range in parallel
 * @details Applies unary_op to [first1, last1) storing results in [d_first,...)
 * @warning Input and output iterators must be random access iterators
 * @warning Output range [d_first, d_first + distance(first1, last1)) must have
 * sufficient allocated capacity
 * @tparam InputIt Input iterator type (std::random_access_iterator)
 * @tparam OutputIt Output iterator type (std::random_access_iterator)
 * @tparam UnaryOperation Unary operation type, signature should be
 * OutType(const InType&)
 * @param first1 Start of input range
 * @param last1 End of input range
 * @param d_first Start of output range
 * @param unary_op Operation to apply to each element
 *
 * @code{.cpp}
 * std::vector<int> input = {1, 2, 3, 4, 5};
 * std::vector<int> output(input.size());
 *
 * // Square each element in parallel
 * parallel_transform(input.begin(), input.end(), output.begin(),
 *                   [](int x) { return x * x; });
 * // output now contains {1, 4, 9, 16, 25}
 * @endcode
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
      std::is_base_of<std::random_access_iterator_tag,
                      typename InputTraits::iterator_category>::value,
      "parallel_transform currently requires random access input iterators.");
  static_assert(
      std::is_base_of<std::random_access_iterator_tag,
                      typename OutputTraits::iterator_category>::value,
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
      break;

    futures.emplace_back(pool.submit(
        [first1, d_first, chunk_start_index, chunk_end_index, unary_op]()
        {
          using diff_t =
              typename std::iterator_traits<InputIt>::difference_type;
          for (size_t k = chunk_start_index; k < chunk_end_index; ++k) {
            *(d_first + static_cast<diff_t>(k)) =
                unary_op(*(first1 + static_cast<diff_t>(k)));
          }
        }));

    chunk_start_index = chunk_end_index;
    if (chunk_start_index >= static_cast<size_t>(total_size))
      break;
  }

  try {
    for (auto& fut : futures) {
      fut.get();
    }
  } catch (...) {
    throw;
  }
}

/**
 * @brief Convenience overload for vectors
 */
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(std::vector<T, Alloc>& vec,
                                           Function func)
{
  parallel_transform(vec.begin(), vec.end(), std::move(func));
}

/**
 * @brief Convenience overload for const vectors
 */
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(const std::vector<T, Alloc>& vec,
                                           Function func)
{
  parallel_transform(vec.cbegin(), vec.cend(), std::move(func));
}

/**
 * @brief Convenience overload for arrays
 */
template<typename T, size_t N, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(std::array<T, N>& arr, Function func)
{
  parallel_transform(arr.begin(), arr.end(), std::move(func));
}

/**
 * @brief Convenience overload for const arrays
 */
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
 * @brief Performs parallel reduction on range [begin, end)
 * @details Uses default thread pool for task division. Performs local reduction
 * then merges results. reduce_op should be associative. identity must be
 * identity element for reduce_op.
 * @tparam Iterator Input iterator type (must support random access)
 * @tparam T Type of reduction result and identity element
 * @tparam BinaryOperation Binary operation type, signature should be T(const
 * T&, const ElementType&) or T(T,T)
 * @param begin Start iterator
 * @param end End iterator
 * @param identity Identity element for reduction
 * @param reduce_op Binary operation to merge two T values or T with element
 * type
 * @return Result of parallel reduction
 *
 * @code{.cpp}
 * std::vector<int> vec = {1, 2, 3, 4, 5};
 *
 * // Sum elements in parallel
 * int sum = parallel_reduce(vec.begin(), vec.end(), 0,
 *                          std::plus<int>());
 * // sum == 15
 *
 * // Find maximum element
 * int max = parallel_reduce(vec.begin(), vec.end(),
 *                          std::numeric_limits<int>::min(),
 *                          [](int a, int b) { return std::max(a,b); });
 * // max == 5
 * @endcode
 */
template<typename Iterator, typename T, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(Iterator begin,
                                     Iterator end,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  using traits = std::iterator_traits<Iterator>;
  static_assert(std::is_base_of<std::random_access_iterator_tag,
                                typename traits::iterator_category>::value,
                "parallel_reduce currently requires random access iterators "
                "for efficient chunking.");

  const auto total_size = std::distance(begin, end);
  if (total_size <= 0) {
    return identity;
  }

  auto& pool = default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t min_chunk_size = 256;
  const size_t hardware_threads =
      std::max(1u, std::thread::hardware_concurrency());
  const size_t num_tasks = std::max(static_cast<size_t>(1ul),
                                    std::min(num_threads, hardware_threads));

  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(total_size) / num_tasks)));
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

    futures.emplace_back(pool.submit(
        [chunk_begin, chunk_end, reduce_op, identity]() -> T
        {
          if (chunk_begin == chunk_end) {
            return identity;
          }

          auto it = chunk_begin;
          T local_result = *it;
          ++it;

          for (; it != chunk_end; ++it) {
            local_result = reduce_op(local_result, *it);
          }
          return local_result;
        }));

    chunk_begin = chunk_end;
    if (chunk_begin == end)
      break;
  }

  T final_result = identity;
  try {
    for (auto& fut : futures) {
      T partial_result = fut.get();
      final_result = reduce_op(final_result, partial_result);
    }
  } catch (...) {
    throw;
  }

  return final_result;
}

/**
 * @brief Convenience overload for vectors
 */
template<typename T, typename Alloc, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(std::vector<T, Alloc>& vec,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      vec.begin(), vec.end(), identity, std::move(reduce_op));
}

/**
 * @brief Convenience overload for const vectors
 */
template<typename T, typename Alloc, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(const std::vector<T, Alloc>& vec,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      vec.cbegin(), vec.cend(), identity, std::move(reduce_op));
}

/**
 * @brief Convenience overload for arrays
 */
template<typename T, size_t N, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(std::array<T, N>& arr,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      arr.begin(), arr.end(), identity, std::move(reduce_op));
}

/**
 * @brief Convenience overload for const arrays
 */
template<typename T, size_t N, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(const std::array<T, N>& arr,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      arr.cbegin(), arr.cend(), identity, std::move(reduce_op));
}

}  // namespace toolbox::concurrent