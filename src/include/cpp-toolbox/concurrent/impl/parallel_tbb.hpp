#pragma once

#include <algorithm>  // for std::min
#include <cmath>  // for std::ceil
#include <future>
#include <iterator>
#include <numeric>  // for std::accumulate (in reduce example)
#include <stdexcept>  // for exceptions
#include <vector>

// TBB 头文件
#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_for_each.h>
#include <tbb/parallel_reduce.h>
#include <tbb/parallel_scan.h>
#include <tbb/parallel_sort.h>

namespace toolbox::concurrent
{

// 声明 default_pool 函数
inline base::thread_pool_singleton_t& default_pool();

/**
 * @brief 使用TBB并行对范围[begin, end)中的每个元素应用函数
 * @details 直接使用TBB的parallel_for_each实现高效并行处理
 * @tparam Iterator 输入迭代器类型(必须支持随机访问以实现高效分块)
 * @tparam Function 一元函数对象类型
 * @param begin 范围起始迭代器
 * @param end 范围结束迭代器
 * @param func 应用于每个元素的函数对象
 */
template<typename Iterator, typename Function>
void parallel_for_each(Iterator begin, Iterator end, Function func)
{
  using traits = std::iterator_traits<Iterator>;
  static_assert(std::is_base_of_v<std::random_access_iterator_tag,
                                  typename traits::iterator_category>,
                "parallel_for_each currently requires random access iterators "
                "for efficient chunking.");

  const auto total_size = std::distance(begin, end);
  if (total_size <= 0) {
    return;
  }

  // 使用TBB的parallel_for_each直接处理
  tbb::parallel_for_each(begin, end, func);
}

/**
 * @brief 使用TBB并行转换范围[first1, last1)中的元素并存储到从d_first开始的范围
 * @details 使用TBB的parallel_for实现高效并行转换
 * @tparam InputIt 输入迭代器类型
 * @tparam OutputIt 输出迭代器类型
 * @tparam UnaryOperation 一元操作类型
 * @param first1 输入范围起始
 * @param last1 输入范围结束
 * @param d_first 输出范围起始
 * @param unary_op 应用于每个元素的操作
 */
template<typename InputIt, typename OutputIt, typename UnaryOperation>
void parallel_transform(InputIt first1,
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

  // 使用TBB的parallel_for实现transform
  tbb::parallel_for(
      tbb::blocked_range<size_t>(0, static_cast<size_t>(total_size)),
      [&](const tbb::blocked_range<size_t>& range)
      {
        for (size_t i = range.begin(); i != range.end(); ++i) {
          *(d_first + static_cast<typename OutputTraits::difference_type>(i)) =
              unary_op(
                  *(first1
                    + static_cast<typename InputTraits::difference_type>(i)));
        }
      });
}

/**
 * @brief 使用TBB对范围[begin, end)执行并行归约操作
 * @details 直接使用TBB的parallel_reduce实现高效并行归约
 * @tparam Iterator 输入迭代器类型(必须支持随机访问)
 * @tparam T 归约结果和单位元素的类型
 * @tparam BinaryOperation 二元操作类型
 * @param begin 起始迭代器
 * @param end 结束迭代器
 * @param identity 归约操作的单位元素
 * @param reduce_op 用于合并两个T值或T与元素类型的二元操作
 * @return 并行归约的结果
 */
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

  // 使用TBB的parallel_reduce
  return tbb::parallel_reduce(
      tbb::blocked_range<size_t>(0, static_cast<size_t>(total_size)),
      identity,
      [&](const tbb::blocked_range<size_t>& range, T init)
      {
        for (size_t i = range.begin(); i != range.end(); ++i) {
          init = reduce_op(
              init,
              *(begin + static_cast<typename traits::difference_type>(i)));
        }
        return init;
      },
      reduce_op);
}

/**
 * @brief 使用TBB执行并行包含扫描操作
 * @details 使用TBB的parallel_scan实现高效并行扫描
 * @tparam InputIt 输入迭代器类型
 * @tparam OutputIt 输出迭代器类型
 * @tparam T 扫描结果和初始值的类型
 * @tparam BinaryOperation 二元操作类型
 * @param first 输入范围起始
 * @param last 输入范围结束
 * @param d_first 输出范围起始
 * @param init 初始值
 * @param binary_op 二元操作
 * @param identity 二元操作的单位元素
 */
template<typename InputIt,
         typename OutputIt,
         typename T,
         typename BinaryOperation>
void parallel_inclusive_scan(InputIt first,
                             InputIt last,
                             OutputIt d_first,
                             T init,
                             BinaryOperation binary_op,
                             [[maybe_unused]] T identity)
{
  using traits = std::iterator_traits<InputIt>;
  static_assert(std::is_base_of_v<std::random_access_iterator_tag,
                                  typename traits::iterator_category>,
                "parallel_inclusive_scan requires random access iterators.");

  const auto total_size = std::distance(first, last);
  if (total_size <= 0) {
    return;
  }

  // 使用TBB的parallel_scan实现inclusive_scan
  class ScanBody
  {
    InputIt input_begin_;
    OutputIt output_begin_;
    T init_;
    BinaryOperation binary_op_;
    T sum_;
    bool is_final_scan_;

  public:
    ScanBody(InputIt input_begin,
             OutputIt output_begin,
             T init,
             BinaryOperation binary_op)
        : input_begin_(input_begin)
        , output_begin_(output_begin)
        , init_(init)
        , binary_op_(binary_op)
        , sum_(init)
        , is_final_scan_(false)
    {
    }

    // 拷贝构造函数，用于TBB内部分割
    ScanBody(const ScanBody& other, tbb::split)
        : input_begin_(other.input_begin_)
        , output_begin_(other.output_begin_)
        , init_(other.init_)
        , binary_op_(other.binary_op_)
        , sum_(other.init_)
        , is_final_scan_(false)
    {
    }

    // 预扫描，计算范围内的累积和
    void pre_scan(const tbb::blocked_range<size_t>& range, T& sum)
    {
      T temp = sum;
      for (size_t i = range.begin(); i != range.end(); ++i) {
        temp = binary_op_(
            temp,
            *(input_begin_ + static_cast<typename traits::difference_type>(i)));
      }
      sum = temp;
    }

    // 最终扫描，计算并存储结果
    void final_scan(const tbb::blocked_range<size_t>& range, T& sum)
    {
      T temp = sum;
      for (size_t i = range.begin(); i != range.end(); ++i) {
        temp = binary_op_(
            temp,
            *(input_begin_ + static_cast<typename traits::difference_type>(i)));
        *(output_begin_ + static_cast<typename traits::difference_type>(i)) =
            temp;
      }
      sum = temp;
    }

    // 合并两个body
    void reverse_join(ScanBody& left) { sum_ = binary_op_(left.sum_, sum_); }

    // 根据is_final_scan_标志选择执行pre_scan或final_scan
    void operator()(const tbb::blocked_range<size_t>& range,
                    tbb::pre_scan_tag /* tag */)
    {
      pre_scan(range, sum_);
    }

    void operator()(const tbb::blocked_range<size_t>& range,
                    tbb::final_scan_tag /* tag */)
    {
      final_scan(range, sum_);
    }

    // 设置为最终扫描模式
    void assign(T value) { sum_ = value; }
  };

  ScanBody body(first, d_first, init, binary_op);
  tbb::parallel_scan(
      tbb::blocked_range<size_t>(0, static_cast<size_t>(total_size)), body);
}

/**
 * @brief 使用TBB实现并行合并排序
 * @details 直接使用TBB的parallel_sort实现高效并行排序
 * @tparam RandomIt 随机访问迭代器类型
 * @tparam Compare 比较器类型
 * @param begin 起始迭代器
 * @param end 结束迭代器
 * @param comp 比较器
 */
template<typename RandomIt, typename Compare>
void parallel_merge_sort(RandomIt begin, RandomIt end, Compare comp)
{
  const auto total_size = std::distance(begin, end);
  if (total_size <= 1) {
    return;
  }

  // 直接使用TBB的parallel_sort
  tbb::parallel_sort(begin, end, comp);
}

/**
 * @brief 使用TBB实现并行TimSort
 * @details 由于TBB的parallel_sort已经是高度优化的并行排序算法，
 *          我们直接使用它作为TimSort的实现
 * @tparam RandomIt 随机访问迭代器类型
 * @tparam Compare 比较器类型
 * @param begin 起始迭代器
 * @param end 结束迭代器
 * @param comp 比较器
 */
template<typename RandomIt, typename Compare>
void parallel_tim_sort(RandomIt begin, RandomIt end, Compare comp)
{
  const auto total_size = std::distance(begin, end);
  if (total_size <= 1) {
    return;
  }

  // 直接使用TBB的parallel_sort，它已经是高度优化的并行排序算法
  tbb::parallel_sort(begin, end, comp);
}

}  // namespace toolbox::concurrent
