#pragma once

#include <algorithm>
#include <cmath>
#include <future>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>  // For std::string, std::wstring
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/io/formats/base.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::types::detail
{

/**
 * @brief 辅助类型特征,用于检查类型是否为可计算容器 / Helper type trait to check
 * if a type is a calculable container
 *
 * @tparam T 要检查的类型 / Type to check
 * @tparam void SFINAE辅助参数 / SFINAE helper parameter
 */
template<typename T, typename = void>
struct is_calculable_container : std::false_type
{
};

/**
 * @brief 辅助类型特征的特化版本,用于检查容器类型 / Specialization of helper
 * type trait for container types
 *
 * @details 检查类型是否具有begin(),end()方法和value_type,并排除string类型 /
 * Checks if type has begin(), end() methods and value_type, excluding string
 * types
 *
 * @tparam T 要检查的容器类型 / Container type to check
 */
template<typename T>
struct is_calculable_container<
    T,
    std::void_t<decltype(std::begin(std::declval<const T&>())),
                decltype(std::end(std::declval<const T&>())),
                typename T::value_type>>
    : std::bool_constant<!std::is_same_v<std::decay_t<T>, std::string>
                         && !std::is_same_v<std::decay_t<T>, std::wstring>>
{
};

/**
 * @brief 辅助变量模板,用于快速检查类型是否为可计算容器 / Helper variable
 * template for quick checking if type is calculable container
 *
 * @tparam T 要检查的类型 / Type to check
 */
template<typename T>
inline constexpr bool is_calculable_container_v =
    is_calculable_container<T>::value;

}  // namespace toolbox::types::detail

namespace toolbox::types
{

/**
 * @brief 存储和计算最小最大值的主模板类 / Primary template class for storing
 * and calculating minimum and maximum values
 *
 * @tparam T 值的类型 / Type of the values
 *
 * @code{.cpp}
 * // 基本使用 / Basic usage
 * minmax_t<int> mm;
 * mm += 5;
 * mm += 3;
 * std::cout << mm.min << ", " << mm.max; // 输出: 3, 5
 *
 * // 初始化构造 / Initialize with values
 * minmax_t<double> mm2(1.0, 10.0);
 * mm2 += 5.5;
 * @endcode
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT minmax_t
{
  T min;  ///< 最小值 / Minimum value
  T max;  ///< 最大值 / Maximum value
  bool initialized_ = false;  ///< 是否已初始化 / Whether initialized

  /**
   * @brief 使用初始最小和最大值构造 / Construct with initial minimum and
   * maximum values
   * @param initial_min 初始最小值 / Initial minimum value
   * @param initial_max 初始最大值 / Initial maximum value
   */
  minmax_t(T initial_min, T initial_max);

  /**
   * @brief 默认构造函数 / Default constructor
   */
  minmax_t();

  // Rule of Five members (defaulted)
  minmax_t(const minmax_t& other) = default;
  minmax_t(minmax_t&& other) noexcept = default;
  minmax_t& operator=(const minmax_t& other) = default;
  minmax_t& operator=(minmax_t&& other) noexcept = default;
  ~minmax_t() = default;

  /**
   * @brief 添加新值并更新最小最大值 / Add new value and update min/max
   * @param value 要添加的值 / Value to add
   * @return 对象的引用 / Reference to this object
   */
  minmax_t& operator+=(const T& value);
};

// --- Full specializations for primitive types' default constructors ---

/**
 * @brief int类型的minmax_t特化 / Specialization of minmax_t for int type
 */
template<>
inline minmax_t<int>::minmax_t()
    : min(std::numeric_limits<int>::max())
    , max(std::numeric_limits<int>::min())
    , initialized_(false)
{
}

/**
 * @brief float类型的minmax_t特化 / Specialization of minmax_t for float type
 */
template<>
inline minmax_t<float>::minmax_t()
    : min(std::numeric_limits<float>::max())
    , max(std::numeric_limits<float>::lowest())
    , initialized_(false)
{
}

/**
 * @brief double类型的minmax_t特化 / Specialization of minmax_t for double type
 */
template<>
inline minmax_t<double>::minmax_t()
    : min(std::numeric_limits<double>::max())
    , max(std::numeric_limits<double>::lowest())
    , initialized_(false)
{
}

/**
 * @brief long类型的minmax_t特化 / Specialization of minmax_t for long type
 */
template<>
inline minmax_t<long>::minmax_t()
    : min(std::numeric_limits<long>::max())
    , max(std::numeric_limits<long>::min())
    , initialized_(false)
{
}

/**
 * @brief unsigned int类型的minmax_t特化 / Specialization of minmax_t for
 * unsigned int type
 */
template<>
inline minmax_t<unsigned int>::minmax_t()
    : min(std::numeric_limits<unsigned int>::max())
    , max(std::numeric_limits<unsigned int>::min())
    , initialized_(false)
{
}

/**
 * @brief unsigned long类型的minmax_t特化 / Specialization of minmax_t for
 * unsigned long type
 */
template<>
inline minmax_t<unsigned long>::minmax_t()
    : min(std::numeric_limits<unsigned long>::max())
    , max(std::numeric_limits<unsigned long>::min())
    , initialized_(false)
{
}

/**
 * @brief unsigned long long类型的minmax_t特化 / Specialization of minmax_t for
 * unsigned long long type
 */
template<>
inline minmax_t<unsigned long long>::minmax_t()
    : min(std::numeric_limits<unsigned long long>::max())
    , max(std::numeric_limits<unsigned long long>::min())
    , initialized_(false)
{
}

/**
 * @brief unsigned char类型的minmax_t特化 / Specialization of minmax_t for
 * unsigned char type
 */
template<>
inline minmax_t<unsigned char>::minmax_t()
    : min(std::numeric_limits<unsigned char>::max())
    , max(std::numeric_limits<unsigned char>::min())
    , initialized_(false)
{
}

/**
 * @brief unsigned short类型的minmax_t特化 / Specialization of minmax_t for
 * unsigned short type
 */
template<>
inline minmax_t<unsigned short>::minmax_t()
    : min(std::numeric_limits<unsigned short>::max())
    , max(std::numeric_limits<unsigned short>::min())
    , initialized_(false)
{
}

/**
 * @brief point_t类型的minmax_t偏特化 / Partial specialization of minmax_t for
 * point_t type
 *
 * @tparam T 点坐标的类型 / Type of point coordinates
 *
 * @code{.cpp}
 * // 使用示例 / Usage example
 * minmax_t<point_t<float>> bounds;
 * bounds += point_t<float>{1.0f, 2.0f};
 * bounds += point_t<float>{-1.0f, 5.0f};
 * // bounds.min = {-1.0f, 2.0f}, bounds.max = {1.0f, 5.0f}
 * @endcode
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT minmax_t<point_t<T>>
{
  point_t<T> min;  ///< 最小点坐标 / Minimum point coordinates
  point_t<T> max;  ///< 最大点坐标 / Maximum point coordinates
  bool initialized_ = false;  ///< 是否已初始化 / Whether initialized

  minmax_t();
  minmax_t(point_t<T> initial_min, point_t<T> initial_max);

  // Rule of Five members (defaulted)
  minmax_t(const minmax_t&) = default;
  minmax_t(minmax_t&&) noexcept = default;
  minmax_t& operator=(const minmax_t&) = default;
  minmax_t& operator=(minmax_t&&) noexcept = default;
  ~minmax_t() = default;

  minmax_t& operator+=(const point_t<T>& value);
};

/**
 * @brief 合并两个minmax_t对象 / Combine two minmax_t objects
 *
 * @tparam T 值的类型 / Type of values
 * @param a 第一个minmax_t对象 / First minmax_t object
 * @param b 第二个minmax_t对象 / Second minmax_t object
 * @return 合并后的minmax_t对象 / Combined minmax_t object
 *
 * @code{.cpp}
 * minmax_t<int> m1(0, 5), m2(3, 8);
 * auto combined = combine_minmax(m1, m2); // min = 0, max = 8
 * @endcode
 */
template<typename T>
[[nodiscard]] minmax_t<T> combine_minmax(const minmax_t<T>& a,
                                         const minmax_t<T>& b);

/**
 * @brief 合并两个point_t类型的minmax_t对象 / Combine two minmax_t objects of
 * point_t type
 *
 * @tparam CoordT 坐标类型 / Coordinate type
 * @param a 第一个minmax_t对象 / First minmax_t object
 * @param b 第二个minmax_t对象 / Second minmax_t object
 * @return 合并后的minmax_t对象 / Combined minmax_t object
 */
template<typename CoordT>
[[nodiscard]] minmax_t<point_t<CoordT>> combine_minmax(
    const minmax_t<point_t<CoordT>>& a, const minmax_t<point_t<CoordT>>& b);

/**
 * @brief 计算非容器类型的最小最大值 / Calculate minmax for non-container type
 *
 * @tparam InputType 输入类型 / Input type
 * @param input 输入值 / Input value
 * @return 包含最小最大值的minmax_t对象 / minmax_t object containing min and max
 * values
 */
template<typename InputType>
[[nodiscard]] auto calculate_minmax(const InputType& input)
    -> std::enable_if_t<!detail::is_calculable_container_v<InputType>,
                        minmax_t<std::decay_t<InputType>>>;

/**
 * @brief 计算容器类型的最小最大值 / Calculate minmax for container type
 *
 * @tparam InputType 容器类型 / Container type
 * @param input 输入容器 / Input container
 * @return 包含最小最大值的minmax_t对象 / minmax_t object containing min and max
 * values
 *
 * @code{.cpp}
 * std::vector<int> vec{1, 5, 3, 8, 2};
 * auto mm = calculate_minmax(vec); // min = 1, max = 8
 * @endcode
 */
template<typename InputType>
[[nodiscard]] auto calculate_minmax(const InputType& input)
    -> std::enable_if_t<detail::is_calculable_container_v<InputType>,
                        minmax_t<typename InputType::value_type>>;

/**
 * @brief 计算点云的最小最大值 / Calculate minmax for point cloud
 *
 * @tparam T 点坐标类型 / Point coordinate type
 * @param input 输入点云 / Input point cloud
 * @return 包含最小最大点的minmax_t对象 / minmax_t object containing min and max
 * points
 */
template<typename T>
[[nodiscard]] auto calculate_minmax(const point_cloud_t<T>& input)
    -> minmax_t<point_t<T>>;

/**
 * @brief 并行计算非容器类型的最小最大值 / Calculate minmax for non-container
 * type in parallel
 *
 * @tparam InputType 输入类型 / Input type
 * @param input 输入值 / Input value
 * @return 包含最小最大值的minmax_t对象 / minmax_t object containing min and max
 * values
 */
template<typename InputType>
[[nodiscard]] auto calculate_minmax_parallel(const InputType& input)
    -> std::enable_if_t<!detail::is_calculable_container_v<InputType>,
                        minmax_t<std::decay_t<InputType>>>;

/**
 * @brief 并行计算容器类型的最小最大值 / Calculate minmax for container type in
 * parallel
 *
 * @tparam InputType 容器类型 / Container type
 * @param input 输入容器 / Input container
 * @return 包含最小最大值的minmax_t对象 / minmax_t object containing min and max
 * values
 *
 * @code{.cpp}
 * std::vector<double> large_vec = get_large_vector();
 * auto mm = calculate_minmax_parallel(large_vec); // 并行计算最小最大值 /
 * Calculate min/max in parallel
 * @endcode
 */
template<typename InputType>
[[nodiscard]] auto calculate_minmax_parallel(const InputType& input)
    -> std::enable_if_t<detail::is_calculable_container_v<InputType>,
                        minmax_t<typename InputType::value_type>>;

/**
 * @brief 并行计算点云的最小最大值 / Calculate minmax for point cloud in
 * parallel
 *
 * @tparam T 点坐标类型 / Point coordinate type
 * @param input 输入点云 / Input point cloud
 * @return 包含最小最大点的minmax_t对象 / minmax_t object containing min and max
 * points
 */
template<typename T>
[[nodiscard]] auto calculate_minmax_parallel(const point_cloud_t<T>& input)
    -> minmax_t<point_t<T>>;

}  // namespace toolbox::types

// Include the implementation file at the end
#include "cpp-toolbox/types/impl/minmax_impl.hpp"