#pragma once

#include <algorithm>  ///< 用于排序和查找最值/For sorting and finding min/max
#include <cmath>  ///< 用于数学运算/For mathematical operations
#include <functional>  ///< 用于函数对象/For function objects
#include <iterator>  ///< 用于迭代器操作/For iterator operations
#include <map>  ///< 用于映射统计/For map statistics
#include <numeric>  ///< 用于累加/For accumulation
#include <stdexcept>  ///< 用于异常处理/For exception handling
#include <string>  ///< 用于字符串处理/For string handling
#include <type_traits>  ///< 用于类型特性/For type traits
#include <utility>  ///< 用于通用工具/For general utilities
#include <vector>  ///< 用于动态数组/For dynamic arrays

#include <cpp-toolbox/type_traits.hpp>

namespace toolbox::math
{

/**
 * @brief 检查容器是否为空并抛出异常/Check if the container is empty and throw
 * an exception
 *
 * @tparam TContainer 容器类型/Container type
 * @param data 输入容器/Input container
 * @param func_name 函数名（用于错误信息）/Function name (for error message)
 *
 * @note 若容器为空则抛出 std::invalid_argument 异常/Throws
 * std::invalid_argument if container is empty
 *
 * @code
 * std::vector<int> v;
 * check_empty(v, "mean"); // 抛出异常/Throws exception
 * @endcode
 */
template<typename TContainer>
void check_empty(const TContainer& data, const char* func_name)
{
  if (data.empty()) {
    std::string err_msg = "Input container for '";
    err_msg += func_name;
    err_msg += "' cannot be empty.";
    throw std::invalid_argument(err_msg);
  }
}

/**
 * @brief 计算容器中元素的平均值/Compute the mean (average) of elements in a
 * container
 *
 * @tparam TContainer 容器类型，元素需为算术类型/Container type, elements must
 * be arithmetic
 * @param data 输入容器/Input container
 * @return double 平均值/Mean value
 *
 * @throws std::invalid_argument 如果容器为空/If the container is empty
 *
 * @code
 * std::vector<int> v{1, 2, 3, 4};
 * double m = toolbox::math::mean(v); // m == 2.5
 * @endcode
 */
template<typename TContainer>
double mean(const TContainer& data)
{
  using ValueType = typename TContainer::value_type;
  static_assert(std::is_arithmetic_v<ValueType>,
                "Mean requires arithmetic type elements.");
  check_empty(data, "mean");

  double sum = 0.0;
  for (const auto& val : data) {
    sum += static_cast<double>(val);  // 使用 double 进行累加保证精度/Use double
                                      // for accumulation to ensure precision
  }
  return sum / data.size();
}

/**
 * @brief 计算容器中元素的中位数/Compute the median of elements in a container
 *
 * @tparam TContainer 容器类型，元素需为算术类型且可比较/Container type,
 * elements must be arithmetic and comparable
 * @param data 输入容器/Input container
 * @return double 中位数/Median value
 *
 * @throws std::invalid_argument 如果容器为空/If the container is empty
 *
 * @code
 * std::vector<int> v{1, 3, 2};
 * double med = toolbox::math::median(v); // med == 2
 * @endcode
 */
template<typename TContainer>
double median(const TContainer& data)
{
  using ValueType = typename TContainer::value_type;
  static_assert(std::is_arithmetic_v<ValueType>,
                "Median requires arithmetic type elements for calculation.");
  static_assert(toolbox::traits::is_less_than_comparable_v<ValueType>,
                "Median requires comparable elements for sorting.");
  check_empty(data, "median");

  std::vector<ValueType> sorted_data(
      data.begin(),
      data.end());  // 复制数据以便排序/Copy data for sorting
  std::sort(sorted_data.begin(), sorted_data.end());

  size_t n = sorted_data.size();
  if (n % 2 != 0) {
    // 奇数个元素/Odd number of elements
    return static_cast<double>(sorted_data[n / 2]);
  } else {
    // 偶数个元素/Even number of elements
    return static_cast<double>(sorted_data[n / 2 - 1] + sorted_data[n / 2])
        / 2.0;
  }
}

/**
 * @brief 计算容器中元素的众数（可能有多个）/Compute the mode(s) of elements in
 * a container (may be multiple)
 *
 * @tparam TContainer 容器类型，元素需可比较/Container type, elements must be
 * comparable
 * @param data 输入容器/Input container
 * @return std::vector<typename TContainer::value_type> 众数集合/Vector of
 * mode(s)
 *
 * @note 若容器为空，返回空集合/Returns empty vector if container is empty
 *
 * @code
 * std::vector<int> v{1, 2, 2, 3, 3};
 * auto modes = toolbox::math::mode(v); // modes == {2, 3}
 * @endcode
 */
template<typename TContainer>
std::vector<typename TContainer::value_type> mode(const TContainer& data)
{
  using ValueType = typename TContainer::value_type;
  static_assert(
      toolbox::traits::is_less_than_comparable_v<ValueType>,
      "Mode requires element type to be comparable (for std::map keys).");
  // 允许空容器的众数为空集合/Allow mode of empty container to be empty set
  if (data.empty()) {
    return {};
  }

  std::map<ValueType, size_t> counts;
  for (const auto& val : data) {
    counts[val]++;
  }

  size_t max_freq = 0;
  for (const auto& pair : counts) {
    if (pair.second > max_freq) {
      max_freq = pair.second;
    }
  }

  std::vector<ValueType> modes;
  if (max_freq > 0) {  // 确保至少有一个元素/Ensure at least one element
    for (const auto& pair : counts) {
      if (pair.second == max_freq) {
        modes.push_back(pair.first);
      }
    }
    // 为了结果的一致性，对众数进行排序 (如果它们本身是可比较的)/Sort modes for
    // consistency (if comparable) 如果 ValueType 不支持排序，但作为 map key
    // 必须支持，这里 sort 应该也没问题/If ValueType is not sortable, but as map
    // key it must be, so sort is fine here
    std::sort(modes.begin(), modes.end());
  }
  return modes;
}

/**
 * @brief 计算方差（默认样本方差 N-1）/Compute the variance (default is sample
 * variance N-1)
 *
 * @tparam TContainer 容器类型，元素需为算术类型/Container type, elements must
 * be arithmetic
 * @param data 输入容器/Input container
 * @param sample_variance 是否计算样本方差（true）或总体方差（false）/Whether to
 * compute sample variance (true) or population variance (false)
 * @return double 方差/Variance
 *
 * @throws std::invalid_argument 如果样本方差且元素少于2/If sample variance and
 * less than 2 elements
 * @throws std::invalid_argument 如果总体方差且容器为空/If population variance
 * and container is empty
 *
 * @code
 * std::vector<double> v{1.0, 2.0, 3.0};
 * double var = toolbox::math::variance(v); // 样本方差/sample variance
 * double var_pop = toolbox::math::variance(v, false); // 总体方差/population
 * variance
 * @endcode
 */
template<typename TContainer>
double variance(const TContainer& data, bool sample_variance = true)
{
  using ValueType = typename TContainer::value_type;
  static_assert(std::is_arithmetic_v<ValueType>,
                "Variance requires arithmetic type elements.");

  size_t n = data.size();
  const char* func_name_str =
      sample_variance ? "sample variance" : "population variance";

  if (sample_variance) {
    if (n < 2) {
      std::string err_msg =
          "Sample variance requires at least 2 data points. Container for '";
      err_msg += func_name_str;
      err_msg += "' is too small.";
      throw std::invalid_argument(err_msg);
    }
  } else {  // population variance/总体方差
    check_empty(data, func_name_str);  // 总体方差至少需要1个元素/Population
                                       // variance requires at least 1 element
  }
  // mean() 内部会调用 check_empty/mean() will call check_empty internally
  double m = mean(data);
  double sum_sq_diff = 0.0;
  for (const auto& val : data) {
    sum_sq_diff += std::pow(static_cast<double>(val) - m, 2);
  }

  if (sample_variance) {
    return sum_sq_diff / (n - 1);
  } else {
    return sum_sq_diff / n;
  }
}

/**
 * @brief 计算标准差（默认样本标准差）/Compute the standard deviation (default
 * is sample standard deviation)
 *
 * @tparam TContainer 容器类型，元素需为算术类型/Container type, elements must
 * be arithmetic
 * @param data 输入容器/Input container
 * @param sample_stdev 是否计算样本标准差（true）或总体标准差（false）/Whether
 * to compute sample stdev (true) or population stdev (false)
 * @return double 标准差/Standard deviation
 *
 * @throws std::invalid_argument 如果样本标准差且元素少于2/If sample stdev and
 * less than 2 elements
 * @throws std::invalid_argument 如果总体标准差且容器为空/If population stdev
 * and container is empty
 *
 * @code
 * std::vector<double> v{1.0, 2.0, 3.0};
 * double sd = toolbox::math::stdev(v); // 样本标准差/sample stdev
 * double sd_pop = toolbox::math::stdev(v, false); // 总体标准差/population
 * stdev
 * @endcode
 */
template<typename TContainer>
double stdev(const TContainer& data, bool sample_stdev = true)
{
  using ValueType = typename TContainer::value_type;
  static_assert(std::is_arithmetic_v<ValueType>,
                "Standard deviation requires arithmetic type elements.");
  // variance 函数会处理大小检查和空容器情况/variance will handle size and empty
  // checks
  return std::sqrt(variance(data, sample_stdev));
}

/**
 * @brief 计算容器中元素的总和（返回容器元素类型）/Compute the sum of elements
 * in the container (returns container element type)
 *
 * @tparam TContainer 容器类型，元素需为算术类型/Container type, elements must
 * be arithmetic
 * @param data 输入容器/Input container
 * @return ValueType 总和/Sum
 *
 * @note 空容器返回0/Returns 0 for empty container
 *
 * @code
 * std::vector<int> v{1, 2, 3};
 * int s = toolbox::math::sum(v); // s == 6
 * @endcode
 */
template<typename TContainer>
typename TContainer::value_type sum(const TContainer& data)
{
  using ValueType = typename TContainer::value_type;
  static_assert(std::is_arithmetic_v<ValueType>,
                "Sum requires arithmetic type elements.");
  if (data.empty()) {
    return static_cast<ValueType>(0);  // 空集合的和为0/Sum of empty set is 0
  }
  // 注意：对于较小整数类型和大量数据，这里可能溢出。考虑使用 sum_d。/Note: For
  // small integer types and large data, may overflow. Consider using sum_d.
  return std::accumulate(data.begin(), data.end(), static_cast<ValueType>(0));
}

/**
 * @brief 计算容器中元素的总和（返回 double 类型以保证精度和范围）/Compute the
 * sum of elements in the container (returns double for precision and range)
 *
 * @tparam TContainer 容器类型，元素需为算术类型/Container type, elements must
 * be arithmetic
 * @param data 输入容器/Input container
 * @return double 总和/Sum
 *
 * @note 空容器返回0.0/Returns 0.0 for empty container
 *
 * @code
 * std::vector<int> v{1, 2, 3};
 * double s = toolbox::math::sum_d(v); // s == 6.0
 * @endcode
 */
template<typename TContainer>
double sum_d(const TContainer& data)
{
  using ValueType = typename TContainer::value_type;
  static_assert(std::is_arithmetic_v<ValueType>,
                "Sum (double) requires arithmetic type elements.");
  if (data.empty()) {
    return 0.0;
  }
  double current_sum = 0.0;
  for (const auto& val : data) {
    current_sum += static_cast<double>(val);
  }
  return current_sum;
}

/**
 * @brief 查找容器中的最小值/Find the minimum value in the container
 *
 * @tparam TContainer 容器类型，元素需可比较/Container type, elements must be
 * comparable
 * @param data 输入容器/Input container
 * @return ValueType 最小值/Minimum value
 *
 * @throws std::invalid_argument 如果容器为空/If the container is empty
 *
 * @code
 * std::vector<int> v{3, 1, 2};
 * int mn = toolbox::math::min(v); // mn == 1
 * @endcode
 */
template<typename TContainer>
typename TContainer::value_type min(const TContainer& data)
{
  using ValueType = typename TContainer::value_type;
  static_assert(toolbox::traits::is_less_than_comparable_v<ValueType>,
                "min requires comparable elements.");
  check_empty(data, "min");
  return *std::min_element(data.begin(), data.end());
}

/**
 * @brief 查找容器中的最大值/Find the maximum value in the container
 *
 * @tparam TContainer 容器类型，元素需可比较/Container type, elements must be
 * comparable
 * @param data 输入容器/Input container
 * @return ValueType 最大值/Maximum value
 *
 * @throws std::invalid_argument 如果容器为空/If the container is empty
 *
 * @code
 * std::vector<int> v{3, 1, 2};
 * int mx = toolbox::math::max(v); // mx == 3
 * @endcode
 */
template<typename TContainer>
typename TContainer::value_type max(const TContainer& data)
{
  using ValueType = typename TContainer::value_type;
  // std::max_element 默认使用 operator</std::max_element uses operator< by
  // default
  static_assert(toolbox::traits::is_less_than_comparable_v<ValueType>,
                "max requires comparable elements.");
  check_empty(data, "max");
  return *std::max_element(data.begin(), data.end());
}

/**
 * @brief 计算容器中元素的全距（最大值-最小值）/Compute the range (max - min) of
 * elements in the container
 *
 * @tparam TContainer 容器类型，元素需为算术类型且可比较/Container type,
 * elements must be arithmetic and comparable
 * @param data 输入容器/Input container
 * @return ValueType 全距/Range
 *
 * @throws std::invalid_argument 如果容器为空/If the container is empty
 *
 * @code
 * std::vector<int> v{3, 1, 2};
 * int r = toolbox::math::range(v); // r == 2
 * @endcode
 */
template<typename TContainer>
typename TContainer::value_type range(const TContainer& data)
{
  using ValueType = typename TContainer::value_type;
  static_assert(std::is_arithmetic_v<ValueType>,
                "Range requires arithmetic type elements for subtraction.");
  static_assert(toolbox::traits::is_less_than_comparable_v<ValueType>,
                "Range requires comparable elements to find min/max.");
  check_empty(data, "range");  // min 和 max 内部也会检查/min and max will also
                               // check internally

  // C++17 中 std::minmax_element 已经可用/std::minmax_element is available in
  // C++17
  auto [min_it, max_it] = std::minmax_element(data.begin(), data.end());
  return static_cast<ValueType>(*max_it - *min_it);
}

/**
 * @brief 计算百分位数（使用最近邻等级法和线性插值）/Compute the percentile
 * (using nearest-rank and linear interpolation)
 *
 * @tparam TContainer 容器类型，元素需为算术类型且可比较/Container type,
 * elements must be arithmetic and comparable
 * @param data 输入容器/Input container
 * @param p 百分位，范围[0.0, 1.0]/Percentile, in [0.0, 1.0]
 * @return double 百分位数/Percentile value
 *
 * @throws std::invalid_argument 如果容器为空/If the container is empty
 * @throws std::out_of_range 如果p不在[0.0, 1.0]范围/If p is not in [0.0, 1.0]
 *
 * @code
 * std::vector<int> v{1, 2, 3, 4, 5};
 * double p50 = toolbox::math::percentile(v, 0.5); // p50 == 3
 * double p90 = toolbox::math::percentile(v, 0.9); // p90 == 4.6
 * @endcode
 */
template<typename TContainer>
double percentile(const TContainer& data, double p)
{
  using ValueType = typename TContainer::value_type;
  static_assert(
      std::is_arithmetic_v<ValueType>,
      "Percentile requires arithmetic type elements for calculation.");
  static_assert(toolbox::traits::is_less_than_comparable_v<ValueType>,
                "Percentile requires comparable elements for sorting.");
  check_empty(data, "percentile");

  if (p < 0.0 || p > 1.0) {
    throw std::out_of_range(
        "Percentile p must be between 0.0 and 1.0 inclusive.");
  }

  std::vector<ValueType> sorted_data(data.begin(), data.end());
  std::sort(sorted_data.begin(), sorted_data.end());

  // Avoid direct floating point equality comparison which triggers
  // -Wfloat-equal when building with strict warnings. Treat values very
  // close to the bounds as the bounds themselves.
  if (p <= 0.0)
    return static_cast<double>(sorted_data.front());
  if (p >= 1.0)
    return static_cast<double>(sorted_data.back());

  // 线性插值法 (R-7 in R, type 7 in NumPy, "Excel.Exclusive" in Excel)/Linear
  // interpolation (R-7 in R, type 7 in NumPy, "Excel.Exclusive" in Excel)
  // (N-1)p gives 0-based index./(N-1)p gives 0-based index.
  double index_double = p * (static_cast<double>(sorted_data.size()) - 1.0);
  size_t lower_index = static_cast<size_t>(std::floor(index_double));
  size_t upper_index = static_cast<size_t>(std::ceil(index_double));

  if (lower_index == upper_index) {  // 索引是整数/Index is integer
    return static_cast<double>(sorted_data[lower_index]);
  } else {  // 在两个值之间插值/Interpolate between two values
    double lower_val = static_cast<double>(sorted_data[lower_index]);
    double upper_val = static_cast<double>(sorted_data[upper_index]);
    return lower_val + (index_double - lower_index) * (upper_val - lower_val);
  }
}

/**
 * @brief 返回容器中k个最小的元素，升序排列/Return the k smallest elements in
 * the container, sorted ascending
 *
 * @tparam TContainer 容器类型，元素需可比较/Container type, elements must be
 * comparable
 * @param data 输入容器/Input container
 * @param k 需要返回的最小元素个数/Number of smallest elements to return
 * @return std::vector<ValueType> k个最小元素/k smallest elements
 *
 * @throws std::invalid_argument 如果k>0且容器为空/If k>0 and container is empty
 *
 * @note 若k为0返回空集合，若k>=容器大小则返回所有元素排序/Returns empty if
 * k==0, returns all sorted if k>=container size
 *
 * @code
 * std::vector<int> v{5, 1, 3, 2, 4};
 * auto mins = toolbox::math::min_k(v, 3); // mins == {1, 2, 3}
 * @endcode
 */
template<typename TContainer>
std::vector<typename TContainer::value_type> min_k(const TContainer& data,
                                                   size_t k)
{
  using ValueType = typename TContainer::value_type;
  static_assert(
      toolbox::traits::is_less_than_comparable_v<ValueType>,
      "min_k requires elements comparable with operator< for sorting.");

  if (k == 0) {
    return {};
  }
  check_empty(
      data,
      "min_k");  // 如果 k > 0, data 不应为空/If k > 0, data should not be empty

  std::vector<ValueType> result;
  if (k >= data.size()) {  // 如果 k
                           // 大于或等于容器大小，返回所有元素排序后的结果/If k
                           // >= container size, return all sorted
    result.assign(data.begin(), data.end());
    std::sort(result.begin(), result.end());
  } else {
    result.resize(k);  // 必须先调整大小/Must resize first
    std::partial_sort_copy(data.begin(),
                           data.end(),  // 从 data 复制并部分排序到 result/Copy
                                        // from data and partial sort to result
                           result.begin(),
                           result.end());
  }
  return result;
}

/**
 * @brief 返回容器中k个最大的元素，降序排列/Return the k largest elements in the
 * container, sorted descending
 *
 * @tparam TContainer 容器类型，元素需可比较/Container type, elements must be
 * comparable
 * @param data 输入容器/Input container
 * @param k 需要返回的最大元素个数/Number of largest elements to return
 * @return std::vector<ValueType> k个最大元素/k largest elements
 *
 * @throws std::invalid_argument 如果k>0且容器为空/If k>0 and container is empty
 *
 * @note 若k为0返回空集合，若k>=容器大小则返回所有元素降序排序/Returns empty if
 * k==0, returns all sorted descending if k>=container size
 *
 * @code
 * std::vector<int> v{5, 1, 3, 2, 4};
 * auto maxs = toolbox::math::max_k(v, 2); // maxs == {5, 4}
 * @endcode
 */
template<typename TContainer>
std::vector<typename TContainer::value_type> max_k(const TContainer& data,
                                                   size_t k)
{
  using ValueType = typename TContainer::value_type;
  // std::partial_sort_copy with std::greater
  // 需要元素可比较/std::partial_sort_copy with std::greater requires comparable
  // elements std::greater<T> 使用
  // operator>，或者等价地，operator</std::greater<T> uses operator> or
  // equivalently operator<
  static_assert(toolbox::traits::is_greater_than_comparable_v<ValueType>
                    || toolbox::traits::is_less_than_comparable_v<ValueType>,
                "max_k requires comparable elements (operator> or operator< "
                "for std::greater).");

  if (k == 0) {
    return {};
  }
  check_empty(data, "max_k");

  std::vector<ValueType> result;
  if (k >= data.size()) {
    result.assign(data.begin(), data.end());
    std::sort(result.begin(),
              result.end(),
              std::greater<ValueType>());  // 使用 std::greater 进行降序排序/Use
                                           // std::greater for descending sort
  } else {
    result.resize(k);
    std::partial_sort_copy(
        data.begin(),
        data.end(),
        result.begin(),
        result.end(),
        std::greater<ValueType>());  // 使用 std::greater 获取最大的k个/Use
                                     // std::greater to get top k
  }
  return result;
}

}  // namespace toolbox::math