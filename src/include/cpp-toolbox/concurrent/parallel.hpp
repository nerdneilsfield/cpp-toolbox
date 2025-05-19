#pragma once

#include <algorithm>  // for std::min
#include <cmath>  // for std::ceil
#include <future>
#include <iterator>
#include <numeric>  // for std::accumulate (in reduce example)
#include <stdexcept>  // for exceptions
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

#include "cpp-toolbox/base/thread_pool_singleton.hpp"  // 需要线程池/Thread pool required

namespace toolbox::concurrent
{

/**
 * @brief 获取默认线程池实例/Get the default thread pool instance
 * @return 单例线程池实例的引用/Reference to the singleton thread pool instance
 */
inline base::thread_pool_singleton_t& default_pool()
{
  return base::thread_pool_singleton_t::instance();
}

//--------------------------------------------------------------------------
// parallel_for_each
//--------------------------------------------------------------------------

/**
 * @brief 并行对范围[begin, end)中的每个元素应用函数/Applies a function to each
 * element in range [begin, end) in parallel
 * @details 使用默认线程池进行手动任务分割和提交/Uses default thread pool for
 * manual task division and submission
 * @tparam Iterator 输入迭代器类型(必须支持随机访问以实现高效分块)/Input
 * iterator type (must support random access for efficient chunking)
 * @tparam Function 一元函数对象类型/Unary function object type
 * @param begin 范围起始迭代器/Start iterator of range
 * @param end 范围结束迭代器/End iterator of range
 * @param func 应用于每个元素的函数对象/Function object to apply to each element
 *
 * @code{.cpp}
 * std::vector<int> vec = {1, 2, 3, 4, 5};
 *
 * // 并行将每个元素翻倍/Double each element in parallel
 * parallel_for_each(vec.begin(), vec.end(), [](int& x) {
 *   x *= 2;
 * });
 * // vec 现在包含 {2, 4, 6, 8, 10}/vec now contains {2, 4, 6, 8, 10}
 *
 * // 并行打印每个元素/Print each element in parallel
 * parallel_for_each(vec.begin(), vec.end(), [](const int& x) {
 *   std::cout << x << " ";
 * });
 * @endcode
 */
template<typename Iterator, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(Iterator begin,
                                          Iterator end,
                                          Function func);

/**
 * @brief 向量的便捷重载/Convenience overload for vectors
 * @tparam T 向量元素类型/Vector element type
 * @tparam Alloc 向量分配器类型/Vector allocator type
 * @tparam Function 函数对象类型/Function object type
 * @param vec 要处理的向量/Vector to process
 * @param func 要应用的函数/Function to apply
 */
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(std::vector<T, Alloc>& vec,
                                          Function func)
{
  parallel_for_each(vec.begin(), vec.end(), std::move(func));
}

/**
 * @brief 常量向量的便捷重载/Convenience overload for const vectors
 */
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(const std::vector<T, Alloc>& vec,
                                          Function func)
{
  parallel_for_each(vec.cbegin(), vec.cend(), std::move(func));
}

/**
 * @brief 数组的便捷重载/Convenience overload for arrays
 */
template<typename T, size_t N, typename Function>
CPP_TOOLBOX_EXPORT void parallel_for_each(std::array<T, N>& arr, Function func)
{
  parallel_for_each(arr.begin(), arr.end(), std::move(func));
}

/**
 * @brief 常量数组的便捷重载/Convenience overload for const arrays
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
 * @brief 并行将输入范围的元素转换到输出范围/Transforms elements from input
 * range to output range in parallel
 * @details 将一元操作应用于[first1, last1)并将结果存储在[d_first,...)/Applies
 * unary_op to [first1, last1) storing results in [d_first,...)
 * @warning 输入和输出迭代器必须是随机访问迭代器/Input and output iterators must
 * be random access iterators
 * @warning 输出范围[d_first, d_first + distance(first1,
 * last1))必须有足够的分配容量/Output range [d_first, d_first + distance(first1,
 * last1)) must have sufficient allocated capacity
 * @tparam InputIt 输入迭代器类型(std::random_access_iterator)/Input iterator
 * type (std::random_access_iterator)
 * @tparam OutputIt 输出迭代器类型(std::random_access_iterator)/Output iterator
 * type (std::random_access_iterator)
 * @tparam UnaryOperation 一元操作类型，签名应为OutType(const InType&)/Unary
 * operation type, signature should be OutType(const InType&)
 * @param first1 输入范围起始/Start of input range
 * @param last1 输入范围结束/End of input range
 * @param d_first 输出范围起始/Start of output range
 * @param unary_op 应用于每个元素的操作/Operation to apply to each element
 *
 * @code{.cpp}
 * std::vector<int> input = {1, 2, 3, 4, 5};
 * std::vector<int> output(input.size());
 *
 * // 并行计算每个元素的平方/Square each element in parallel
 * parallel_transform(input.begin(), input.end(), output.begin(),
 *                   [](int x) { return x * x; });
 * // output 现在包含 {1, 4, 9, 16, 25}/output now contains {1, 4, 9, 16, 25}
 *
 * // 并行将每个元素转换为字符串/Transform each element to string in parallel
 * std::vector<std::string> str_output(input.size());
 * parallel_transform(input.begin(), input.end(), str_output.begin(),
 *                   [](int x) { return std::to_string(x); });
 * @endcode
 */
template<typename InputIt, typename OutputIt, typename UnaryOperation>
CPP_TOOLBOX_EXPORT void parallel_transform(InputIt first1,
                                           InputIt last1,
                                           OutputIt d_first,
                                           UnaryOperation unary_op);

/**
 * @brief 向量的便捷重载/Convenience overload for vectors
 */
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(std::vector<T, Alloc>& vec,
                                           Function func)
{
  parallel_transform(vec.begin(), vec.end(), std::move(func));
}

/**
 * @brief 常量向量的便捷重载/Convenience overload for const vectors
 */
template<typename T, typename Alloc, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(const std::vector<T, Alloc>& vec,
                                           Function func)
{
  parallel_transform(vec.cbegin(), vec.cend(), std::move(func));
}

/**
 * @brief 数组的便捷重载/Convenience overload for arrays
 */
template<typename T, size_t N, typename Function>
CPP_TOOLBOX_EXPORT void parallel_transform(std::array<T, N>& arr, Function func)
{
  parallel_transform(arr.begin(), arr.end(), std::move(func));
}

/**
 * @brief 常量数组的便捷重载/Convenience overload for const arrays
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
 * @brief 对范围[begin, end)执行并行归约操作/Performs parallel reduction on
 * range [begin, end)
 * @details
 * 使用默认线程池进行任务分割。执行局部归约然后合并结果。reduce_op应该是可结合的。identity必须是reduce_op的单位元素/Uses
 * default thread pool for task division. Performs local reduction then merges
 * results. reduce_op should be associative. identity must be identity element
 * for reduce_op.
 * @tparam Iterator 输入迭代器类型(必须支持随机访问)/Input iterator type (must
 * support random access)
 * @tparam T 归约结果和单位元素的类型/Type of reduction result and identity
 * element
 * @tparam BinaryOperation 二元操作类型，签名应为T(const T&, const
 * ElementType&)或T(T,T)/Binary operation type, signature should be T(const T&,
 * const ElementType&) or T(T,T)
 * @param begin 起始迭代器/Start iterator
 * @param end 结束迭代器/End iterator
 * @param identity 归约操作的单位元素/Identity element for reduction
 * @param reduce_op 用于合并两个T值或T与元素类型的二元操作/Binary operation to
 * merge two T values or T with element type
 * @return 并行归约的结果/Result of parallel reduction
 *
 * @code{.cpp}
 * std::vector<int> vec = {1, 2, 3, 4, 5};
 *
 * // 并行求和/Sum elements in parallel
 * int sum = parallel_reduce(vec.begin(), vec.end(), 0,
 *                          std::plus<int>());
 * // sum == 15
 *
 * // 并行查找最大元素/Find maximum element in parallel
 * int max = parallel_reduce(vec.begin(), vec.end(),
 *                          std::numeric_limits<int>::min(),
 *                          [](int a, int b) { return std::max(a,b); });
 * // max == 5
 *
 * // 并行计算字符串连接/Concatenate strings in parallel
 * std::vector<std::string> strings = {"Hello", " ", "World", "!"};
 * std::string result = parallel_reduce(strings.begin(), strings.end(),
 *                                     std::string(),
 *                                     std::plus<std::string>());
 * // result == "Hello World!"
 * @endcode
 */
template<typename Iterator, typename T, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(Iterator begin,
                                     Iterator end,
                                     T identity,
                                     BinaryOperation reduce_op);

/**
 * @brief 向量的便捷重载/Convenience overload for vectors
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
 * @brief 常量向量的便捷重载/Convenience overload for const vectors
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
 * @brief 数组的便捷重载/Convenience overload for arrays
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
 * @brief 常量数组的便捷重载/Convenience overload for const arrays
 */
template<typename T, size_t N, typename BinaryOperation>
CPP_TOOLBOX_EXPORT T parallel_reduce(const std::array<T, N>& arr,
                                     T identity,
                                     BinaryOperation reduce_op)
{
  return parallel_reduce(
      arr.cbegin(), arr.cend(), identity, std::move(reduce_op));
}

//--------------------------------------------------------------------------
// parallel_inclusive_scan
//--------------------------------------------------------------------------

/**
 * @brief 并行前缀和(包含式)/Parallel inclusive scan (prefix sum)
 * @tparam InputIt 输入迭代器类型(随机访问)/Input iterator type
 * @tparam OutputIt 输出迭代器类型(随机访问)/Output iterator type
 * @tparam T 值类型/Value type
 * @tparam BinaryOperation 二元操作类型/Binary operation type
 * @param first 输入范围起始/Start of input range
 * @param last 输入范围结束/End of input range
 * @param d_first 输出范围起始/Start of output range
 * @param init 起始值/Initial value
 * @param binary_op 用于累加的二元操作/Binary operation for accumulation
 * @param identity binary_op 的单位元素/Identity element for binary_op
 */
template<typename InputIt,
         typename OutputIt,
         typename T,
         typename BinaryOperation>
CPP_TOOLBOX_EXPORT void parallel_inclusive_scan(InputIt first,
                                                InputIt last,
                                                OutputIt d_first,
                                                T init,
                                                BinaryOperation binary_op,
                                                T identity = T {});

/**
 * @brief 向量便捷重载/Convenience overload for vector
 */
template<typename T, typename Alloc, typename BinaryOperation>
CPP_TOOLBOX_EXPORT void parallel_inclusive_scan(
    const std::vector<T, Alloc>& input,
    std::vector<T, Alloc>& out,
    T init,
    BinaryOperation binary_op,
    T identity = T {})
{
  if (out.size() < input.size()) {
    out.resize(input.size());
  }
  parallel_inclusive_scan(input.cbegin(),
                          input.cend(),
                          out.begin(),
                          init,
                          std::move(binary_op),
                          identity);
}

//--------------------------------------------------------------------------
// parallel_merge_sort
//--------------------------------------------------------------------------

/**
 * @brief 并行合并排序/Parallel merge sort (chunked)
 * @tparam RandomIt 随机访问迭代器类型/Random access iterator type
 * @tparam Compare 比较器类型/Comparator type
 */
template<typename RandomIt, typename Compare = std::less<>>
CPP_TOOLBOX_EXPORT void parallel_merge_sort(RandomIt begin,
                                            RandomIt end,
                                            Compare comp = Compare());

//--------------------------------------------------------------------------
// parallel_tim_sort
//--------------------------------------------------------------------------

/**
 * @brief A simplified parallel TimSort implementation
 * @tparam RandomIt Random access iterator type
 * @tparam Compare Comparator type
 */
template<typename RandomIt, typename Compare = std::less<>>
CPP_TOOLBOX_EXPORT void parallel_tim_sort(RandomIt begin,
                                          RandomIt end,
                                          Compare comp = Compare());

}  // namespace toolbox::concurrent

// 包含实现文件
#if defined(CPP_TOOLBOX_USE_TBB)
#  include "impl/parallel_tbb.hpp"
#else
#  include "impl/parallel_raw.hpp"
#endif