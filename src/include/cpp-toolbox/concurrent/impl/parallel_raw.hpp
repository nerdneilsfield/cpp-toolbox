#pragma once

#include <algorithm>  // for std::min
#include <cmath>  // for std::ceil
#include <future>
#include <iterator>
#include <numeric>  // for std::accumulate (in reduce example)
#include <stdexcept>  // for exceptions
#include <vector>

#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::concurrent
{

// 声明 default_pool 函数
inline base::thread_pool_singleton_t& default_pool();

template<typename Iterator, typename Function>
void parallel_for_each(Iterator begin, Iterator end, Function func)
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
  const size_t num_tasks = std::max(num_threads, hardware_threads);

  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(total_size) / num_tasks)));

  std::vector<std::future<void>> futures;
  futures.reserve(num_tasks);

  for (size_t i = 0; i < total_size; i += chunk_size) {
    size_t end_idx = std::min(i + chunk_size, static_cast<size_t>(total_size));
    futures.emplace_back(pool.submit(
        [begin, i, end_idx, func]() mutable
        {
          auto chunk_begin = std::next(begin, static_cast<long>(i));
          auto chunk_end = std::next(begin, static_cast<long>(end_idx));
          std::for_each(chunk_begin, chunk_end, func);
        }));
  }

  try {
    for (auto& fut : futures) {
      fut.get();
    }
  } catch (...) {
    throw;
  }
}

template<typename InputIt, typename OutputIt, typename UnaryOperation>
void parallel_transform(InputIt first1,
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
      std::max(1U, std::thread::hardware_concurrency());
  const size_t num_tasks = std::max(num_threads, hardware_threads);

  size_t chunk_size =
      std::max(min_chunk_size,
               static_cast<size_t>(
                   std::ceil(static_cast<double>(total_size) / num_tasks)));

  std::vector<std::future<void>> futures;
  futures.reserve(num_tasks);

  for (size_t i = 0; i < total_size; i += chunk_size) {
    size_t end_idx = std::min(i + chunk_size, static_cast<size_t>(total_size));
    futures.emplace_back(pool.submit(
        [first1, d_first, i, end_idx, unary_op]() mutable
        {
          auto chunk_begin = std::next(first1, static_cast<long>(i));
          auto chunk_end = std::next(first1, static_cast<long>(end_idx));
          auto chunk_d_first = std::next(d_first, static_cast<long>(i));
          std::transform(chunk_begin, chunk_end, chunk_d_first, unary_op);
        }));
  }

  try {
    for (auto& fut : futures) {
      fut.get();
    }
  } catch (...) {
    throw;
  }
}

template<typename Iterator, typename T, typename BinaryOperation>
T parallel_reduce(Iterator begin,
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
    return identity;
  }

  auto& pool = default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t min_chunk_size = 256;
  const size_t hardware_threads =
      std::max(1U, std::thread::hardware_concurrency());
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

  for (size_t i = 0; i < total_size; i += chunk_size) {
    size_t end_idx = std::min(i + chunk_size, static_cast<size_t>(total_size));
    futures.emplace_back(pool.submit(
        [begin, i, end_idx, identity, reduce_op]() mutable
        {
          auto chunk_begin = std::next(begin, static_cast<long>(i));
          auto chunk_end = std::next(begin, static_cast<long>(end_idx));
          return std::accumulate(chunk_begin, chunk_end, identity, reduce_op);
        }));
  }

  std::vector<T> partial_results;
  partial_results.reserve(futures.size());

  try {
    for (auto& fut : futures) {
      partial_results.push_back(fut.get());
    }
  } catch (...) {
    throw;
  }

  // 如果只有一个部分结果，直接返回它，避免再次应用 identity
  if (partial_results.size() == 1) {
    return partial_results[0];
  }

  // 否则，使用第一个部分结果作为初始值，而不是 identity
  return std::accumulate(partial_results.begin() + 1,
                         partial_results.end(),
                         partial_results[0],
                         reduce_op);
}

template<typename InputIt,
         typename OutputIt,
         typename T,
         typename BinaryOperation>
void parallel_inclusive_scan(InputIt first,
                             InputIt last,
                             OutputIt d_first,
                             T init,
                             BinaryOperation binary_op,
                             T identity)
{
  using traits = std::iterator_traits<InputIt>;
  static_assert(std::is_base_of_v<std::random_access_iterator_tag,
                                  typename traits::iterator_category>,
                "parallel_inclusive_scan requires random access iterators.");

  const auto total_size = std::distance(first, last);
  if (total_size <= 0) {
    return;
  }

  auto& pool = default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t hardware_threads =
      std::max(1U, std::thread::hardware_concurrency());
  size_t num_tasks = std::max(num_threads, hardware_threads);

  auto chunk_size = static_cast<size_t>(std::ceil(
      static_cast<double>(total_size) / static_cast<double>(num_tasks)));
  num_tasks = static_cast<size_t>(std::ceil(static_cast<double>(total_size)
                                            / static_cast<double>(chunk_size)));

  std::vector<T> chunk_sums(num_tasks);
  std::vector<std::pair<size_t, size_t>> ranges;
  ranges.reserve(num_tasks);

  std::vector<std::future<T>> sum_futures;
  sum_futures.reserve(num_tasks);

  InputIt chunk_begin = first;
  for (size_t i = 0; i < num_tasks; ++i) {
    InputIt chunk_end = chunk_begin;
    size_t current_size = std::min(
        chunk_size, static_cast<size_t>(std::distance(chunk_begin, last)));
    if (current_size == 0) {
      break;
    }
    std::advance(chunk_end, current_size);
    ranges.emplace_back(static_cast<size_t>(std::distance(first, chunk_begin)),
                        current_size);

    sum_futures.emplace_back(pool.submit(
        [chunk_begin, chunk_end, identity, binary_op]()
        {
          T local_sum = identity;
          for (auto it = chunk_begin; it != chunk_end; ++it) {
            local_sum = binary_op(local_sum, *it);
          }
          return local_sum;
        }));

    chunk_begin = chunk_end;
    if (chunk_begin == last) {
      break;
    }
  }

  size_t actual_tasks = ranges.size();
  for (size_t i = 0; i < actual_tasks; ++i) {
    chunk_sums[i] = sum_futures[i].get();
  }

  std::vector<T> offsets(actual_tasks);
  T running = init;
  for (size_t i = 0; i < actual_tasks; ++i) {
    offsets[i] = running;
    running = binary_op(running, chunk_sums[i]);
  }

  std::vector<std::future<void>> futures;
  futures.reserve(actual_tasks);
  for (size_t i = 0; i < actual_tasks; ++i) {
    size_t start_index = ranges[i].first;
    size_t len = ranges[i].second;

    InputIt chunk_begin_it =
        first + static_cast<typename traits::difference_type>(start_index);
    OutputIt dest_begin_it =
        d_first + static_cast<typename traits::difference_type>(start_index);
    T offset = offsets[i];

    futures.emplace_back(pool.submit(
        [chunk_begin_it, dest_begin_it, len, offset, binary_op]() mutable
        {
          T local = offset;
          for (size_t j = 0; j < len; ++j) {
            local = binary_op(
                local,
                *(chunk_begin_it
                  + static_cast<typename traits::difference_type>(j)));
            *(dest_begin_it
              + static_cast<typename traits::difference_type>(j)) = local;
          }
        }));
  }

  for (auto& fut : futures) {
    fut.get();
  }
}

template<typename RandomIt, typename Compare>
void parallel_merge_sort(RandomIt begin, RandomIt end, Compare comp)
{
  const auto total_size = std::distance(begin, end);
  if (total_size <= 1) {
    return;
  }

  auto& pool = default_pool();
  const size_t num_threads = pool.get_thread_count();
  const size_t hardware_threads =
      std::max(1U, std::thread::hardware_concurrency());
  size_t num_tasks = std::max(num_threads, hardware_threads);

  auto chunk_size = static_cast<size_t>(std::ceil(
      static_cast<double>(total_size) / static_cast<double>(num_tasks)));
  num_tasks = static_cast<size_t>(std::ceil(static_cast<double>(total_size)
                                            / static_cast<double>(chunk_size)));

  std::vector<std::future<void>> futures;
  futures.reserve(num_tasks);
  std::vector<std::pair<RandomIt, RandomIt>> ranges;
  ranges.reserve(num_tasks);

  RandomIt chunk_begin = begin;
  for (size_t i = 0; i < num_tasks; ++i) {
    RandomIt chunk_end = chunk_begin;
    size_t current_size = std::min(
        chunk_size, static_cast<size_t>(std::distance(chunk_begin, end)));
    if (current_size == 0) {
      break;
    }
    std::advance(chunk_end, current_size);
    ranges.emplace_back(chunk_begin, chunk_end);
    futures.emplace_back(
        pool.submit([chunk_begin, chunk_end, comp]()
                    { std::sort(chunk_begin, chunk_end, comp); }));
    chunk_begin = chunk_end;
    if (chunk_begin == end) {
      break;
    }
  }

  for (auto& fut : futures) {
    fut.get();
  }

  while (ranges.size() > 1) {
    std::vector<std::pair<RandomIt, RandomIt>> new_ranges;
    std::vector<std::future<void>> merge_futs;
    for (size_t i = 0; i + 1 < ranges.size(); i += 2) {
      auto begin1 = ranges[i].first;
      auto mid = ranges[i].second;
      auto end2 = ranges[i + 1].second;
      merge_futs.emplace_back(
          pool.submit([begin1, mid, end2, comp]() mutable
                      { std::inplace_merge(begin1, mid, end2, comp); }));
      new_ranges.emplace_back(begin1, end2);
    }

    for (auto& fut : merge_futs) {
      fut.get();
    }

    if (ranges.size() % 2 == 1) {
      new_ranges.push_back(ranges.back());
    }
    ranges.swap(new_ranges);
  }
}

template<typename RandomIt, typename Compare>
void parallel_tim_sort(RandomIt begin, RandomIt end, Compare comp)
{
  const auto total_size = std::distance(begin, end);
  if (total_size <= 1) {
    return;
  }

  constexpr size_t kRun = 32;
  std::vector<std::pair<RandomIt, RandomIt>> ranges;
  ranges.reserve(
      static_cast<size_t>(std::ceil(static_cast<double>(total_size) / kRun)));

  RandomIt run_begin = begin;
  while (run_begin != end) {
    RandomIt run_end = run_begin;
    size_t len =
        std::min(static_cast<size_t>(std::distance(run_begin, end)), kRun);
    std::advance(run_end, len);
    std::sort(run_begin, run_end, comp);  // small run sort
    ranges.emplace_back(run_begin, run_end);
    run_begin = run_end;
  }

  auto& pool = default_pool();
  while (ranges.size() > 1) {
    std::vector<std::pair<RandomIt, RandomIt>> new_ranges;
    std::vector<std::future<void>> futures;
    for (size_t i = 0; i + 1 < ranges.size(); i += 2) {
      auto begin1 = ranges[i].first;
      auto mid = ranges[i].second;
      auto end2 = ranges[i + 1].second;
      futures.emplace_back(
          pool.submit([begin1, mid, end2, comp]()
                      { std::inplace_merge(begin1, mid, end2, comp); }));
      new_ranges.emplace_back(begin1, end2);
    }
    for (auto& fut : futures) {
      fut.get();
    }
    if (ranges.size() % 2 == 1) {
      new_ranges.push_back(ranges.back());
    }
    ranges.swap(new_ranges);
  }
}

}  // namespace toolbox::concurrent