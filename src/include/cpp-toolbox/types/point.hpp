#pragma once

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <ostream>
#include <random>
#include <stdexcept>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/io/formats/base.hpp>

namespace toolbox::types
{

/**
 * @brief 3D点/向量模板类 / A 3D point/vector template class
 * @tparam T 坐标的数值类型(如float、double、int) / The numeric type for
 * coordinates (e.g. float, double, int)
 *
 * 该类表示具有x、y、z坐标的3D点或向量。提供常见的向量运算并支持算术运算符。/
 * This class represents a 3D point or vector with x, y, z coordinates.
 * It provides common vector operations and supports arithmetic operators.
 *
 * @code{.cpp}
 * // 创建点 / Create points
 * point_t<double> p1(1.0, 2.0, 3.0);
 * point_t<double> p2(4.0, 5.0, 6.0);
 *
 * // 向量运算 / Vector operations
 * double dot_product = p1.dot(p2);
 * point_t<double> cross_product = p1.cross(p2);
 * double norm = p1.norm();
 *
 * // 标准化 / Normalization
 * point_t<double> normalized = p1.normalize();
 *
 * // 算术运算 / Arithmetic
 * point_t<double> sum = p1 + p2;
 * point_t<double> scaled = p1 * 2.0;
 * double distance = p1.distance(p2);
 * @endcode
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT point_t
{
  T x;  ///< X坐标 / X coordinate
  T y;  ///< Y坐标 / Y coordinate
  T z;  ///< Z坐标 / Z coordinate

  // 定义析构函数(五法则/零法则) / Define destructor (Rule of Five/Zero)
  ~point_t() = default;

  /**
   * @brief 使用给定坐标构造点 / Constructs a point with given coordinates
   * @param x_coord X坐标值 / X coordinate value
   * @param y_coord Y坐标值 / Y coordinate value
   * @param z_coord Z坐标值 / Z coordinate value
   */
  point_t(T x_coord, T y_coord, T z_coord);

  /**
   * @brief 默认构造函数，初始化为原点(0,0,0) / Default constructor, initializes
   * to origin (0,0,0)
   */
  point_t();

  point_t(const point_t& other) = default;  ///< 拷贝构造函数 / Copy constructor
  point_t(point_t&& other) noexcept =
      default;  ///< 移动构造函数 / Move constructor

  point_t& operator=(const point_t& other) =
      default;  ///< 拷贝赋值运算符 / Copy assignment
  point_t& operator=(point_t&& other) noexcept =
      default;  ///< 移动赋值运算符 / Move assignment

  /**
   * @brief 分量式加上另一个点 / Add another point component-wise
   * @param other 要加的点 / Point to add
   * @return 对本点的引用 / Reference to this point
   */
  point_t& operator+=(const point_t& other);

  /**
   * @brief 分量式减去另一个点 / Subtract another point component-wise
   * @param other 要减的点 / Point to subtract
   * @return 对本点的引用 / Reference to this point
   */
  point_t& operator-=(const point_t& other);

  /**
   * @brief 坐标乘以标量 / Multiply coordinates by a scalar
   * @param scalar 要乘的值 / Value to multiply by
   * @return 对本点的引用 / Reference to this point
   */
  point_t& operator*=(const T& scalar);

  /**
   * @brief 坐标除以标量 / Divide coordinates by a scalar
   * @param scalar 要除的值 / Value to divide by
   * @return 对本点的引用 / Reference to this point
   * @throws std::runtime_error 如果标量为零 / if scalar is zero
   */
  point_t& operator/=(const T& scalar);

  /**
   * @brief 计算与另一点的点积 / Calculate dot product with another point
   * @param other 要计算点积的点 / Point to calculate dot product with
   * @return 点积值 / Dot product value
   */
  [[nodiscard]] auto dot(const point_t& other) const -> T;

  /**
   * @brief 计算与另一点的叉积 / Calculate cross product with another point
   * @param other 要计算叉积的点 / Point to calculate cross product with
   * @return 表示叉积的新点 / New point representing cross product
   */
  [[nodiscard]] auto cross(const point_t& other) const -> point_t;

  /**
   * @brief 计算向量的欧几里得范数(长度) / Calculate Euclidean norm (length) of
   * vector
   * @return 以double类型返回范数 / Norm as double
   */
  [[nodiscard]] auto norm() const -> double;

  /**
   * @brief 返回归一化向量(单位长度) / Return normalized vector (unit length)
   * @return 以point_t<double>类型返回归一化向量 / Normalized vector as
   * point_t<double>
   */
  [[nodiscard]] auto normalize() const -> point_t<double>;

  /**
   * @brief 计算向量的p范数 / Calculate p-norm of vector
   * @param p_value p范数的p值 / The p value for norm calculation
   * @return 以double类型返回p范数 / p-norm as double
   */
  [[nodiscard]] auto p_norm(const double& p_value) const -> double;

  /**
   * @brief 返回p范数归一化向量 / Return p-normalized vector
   * @param p_value 归一化的p值 / The p value for normalization
   * @return 以point_t<double>类型返回p范数归一化向量 / p-normalized vector as
   * point_t<double>
   */
  [[nodiscard]] auto p_normalize(const double& p_value) const
      -> point_t<double>;

  /**
   * @brief 计算到另一点的欧几里得距离 / Calculate Euclidean distance to another
   * point
   * @param other 要计算距离的目标点 / Point to calculate distance to
   * @return 以double类型返回距离 / Distance as double
   */
  [[nodiscard]] auto distance(const point_t& other) const -> double;

  /**
   * @brief 计算到另一点的p距离 / Calculate p-distance to another point
   * @param other 要计算距离的目标点 / Point to calculate distance to
   * @param p_value p距离的p值 / The p value for distance calculation
   * @return 以double类型返回p距离 / p-distance as double
   */
  [[nodiscard]] auto distance_p(const point_t& other,
                                const double& p_value) const -> double;

  // 定义比较运算符 / Define comparison operators
  bool operator==(const point_t& other) const;

  bool operator!=(const point_t& other) const;

  // 为测试目的添加基本的operator< / Add a basic operator< for testing purposes
  // 注意：这提供了一个任意但一致的排序 / Note: This provides an arbitrary but
  // consistent ordering.
  bool operator<(const point_t& other) const;

  /**
   * @brief 获取具有最小可能值的点 / Get point with minimum possible values
   * @return 具有最小坐标的点 / Point with minimum coordinates
   */
  [[nodiscard]] static auto min_value() -> point_t;

  /**
   * @brief 获取具有最大可能值的点 / Get point with maximum possible values
   * @return 具有最大坐标的点 / Point with maximum coordinates
   */
  [[nodiscard]] static auto max_value() -> point_t;
};

// 静态成员的特化(保持内联定义) / Specializations for static members (keep
// inline definitions here)
template<>
[[nodiscard]] inline auto point_t<int>::min_value() -> point_t<int>
{
  return {std::numeric_limits<int>::min(),
          std::numeric_limits<int>::min(),
          std::numeric_limits<int>::min()};
}

template<>
[[nodiscard]] inline auto point_t<int>::max_value() -> point_t<int>
{
  return {std::numeric_limits<int>::max(),
          std::numeric_limits<int>::max(),
          std::numeric_limits<int>::max()};
}

template<>
[[nodiscard]] inline auto point_t<float>::min_value() -> point_t<float>
{
  return {std::numeric_limits<float>::lowest(),
          std::numeric_limits<float>::lowest(),
          std::numeric_limits<float>::lowest()};
}

template<>
[[nodiscard]] inline auto point_t<float>::max_value() -> point_t<float>
{
  return {std::numeric_limits<float>::max(),
          std::numeric_limits<float>::max(),
          std::numeric_limits<float>::max()};
}

/**
 * @brief point_t的流输出运算符 - 仅声明 / Stream output operator for point_t -
 * Declaration Only
 * @param output_stream 输出流 / Output stream
 * @param pt 要输出的点 / Point to output
 * @return 对输出流的引用 / Reference to output stream
 */
template<typename T>
auto operator<<(std::ostream& output_stream, const point_t<T>& pt)
    -> std::ostream&;

/**
 * @brief 包含点和相关数据的点云类 / A point cloud class containing points and
 * associated data
 * @tparam T 点坐标的数值类型 / The numeric type for point coordinates
 *
 * 该类表示3D点的集合，可选包含法线、颜色和强度数据。/
 * This class represents a collection of 3D points with optional normals,
 * colors and intensity data.
 *
 * @code{.cpp}
 * // 创建点云 / Create point cloud
 * point_cloud_t<double> cloud;
 *
 * // 添加点 / Add points
 * cloud += point_t<double>(1.0, 2.0, 3.0);
 * cloud += point_t<double>(4.0, 5.0, 6.0);
 *
 * // 访问数据 / Access data
 * std::size_t num_points = cloud.size();
 * point_t<double> first_point = cloud.points[0];
 *
 * // 合并点云 / Merge clouds
 * point_cloud_t<double> cloud2;
 * cloud2 += point_t<double>(7.0, 8.0, 9.0);
 * auto merged = cloud + cloud2;
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT point_cloud_t : public ::toolbox::io::base_file_data_t
{
public:
  std::vector<point_t<T>> points;  ///< 点坐标 / Point coordinates
  std::vector<point_t<T>> normals;  ///< 点法线(可选) / Point normals (optional)
  std::vector<point_t<T>> colors;  ///< 点颜色(可选) / Point colors (optional)
  T intensity;  ///< 全局强度值 / Global intensity value

  ~point_cloud_t() = default;

  point_cloud_t();

  point_cloud_t(const point_cloud_t& other);
  point_cloud_t(point_cloud_t&& other) noexcept;
  point_cloud_t& operator=(const point_cloud_t& other);
  point_cloud_t& operator=(point_cloud_t&& other) noexcept;

  /**
   * @brief 获取点云中的点数 / Get number of points in cloud
   * @return 点的数量 / Number of points
   */
  [[nodiscard]] auto size() const -> std::size_t;

  /**
   * @brief 检查点云是否为空 / Check if cloud is empty
   * @return 如果为空返回true，否则返回false / true if empty, false otherwise
   */
  [[nodiscard]] auto empty() const -> bool;

  /**
   * @brief 清除点云中的所有数据 / Clear all data from cloud
   */
  void clear();

  /**
   * @brief 为点预留内存 / Reserve memory for points
   * @param required_size 要预留空间的点数 / Number of points to reserve space
   * for
   */
  void reserve(const std::size_t& required_size);

  /**
   * @brief 合并两个点云 / Add two point clouds
   * @param other 要合并的点云 / Cloud to add
   * @return 新的合并后的点云 / New combined cloud
   */
  [[nodiscard]] auto operator+(const point_cloud_t& other) const
      -> point_cloud_t;

  /**
   * @brief 向点云添加点 / Add point to cloud
   * @param point 要添加的点 / Point to add
   * @return 添加点后的新点云 / New cloud with added point
   */
  [[nodiscard]] auto operator+(const point_t<T>& point) const -> point_cloud_t;

  /**
   * @brief 向点云添加点(移动版本) / Add point to cloud (move version)
   * @param point 要添加的点 / Point to add
   * @return 添加点后的新点云 / New cloud with added point
   */
  [[nodiscard]] auto operator+(point_t<T>&& point) const -> point_cloud_t;

  /**
   * @brief 合并两个点云(移动版本) / Add two point clouds (move version)
   * @param other 要合并的点云 / Cloud to add
   * @return 新的合并后的点云 / New combined cloud
   */
  [[nodiscard]] auto operator+(point_cloud_t&& other) const -> point_cloud_t;

  /**
   * @brief 向点云添加点 / Add point to cloud
   * @param point 要添加的点 / Point to add
   * @return 对本点云的引用 / Reference to this cloud
   */
  point_cloud_t& operator+=(const point_t<T>& point);

  /**
   * @brief 向点云添加点(移动版本) / Add point to cloud (move version)
   * @param point 要添加的点 / Point to add
   * @return 对本点云的引用 / Reference to this cloud
   */
  point_cloud_t& operator+=(point_t<T>&& point);

  /**
   * @brief 合并另一个点云到当前点云 / Add another cloud to this one
   * @param other 要合并的点云 / Cloud to add
   * @return 对本点云的引用 / Reference to this cloud
   */
  point_cloud_t& operator+=(const point_cloud_t& other);

  /**
   * @brief 合并另一个点云到当前点云(移动版本) / Add another cloud to this one
   * (move version)
   * @param other 要合并的点云 / Cloud to add
   * @return 对本点云的引用 / Reference to this cloud
   */
  point_cloud_t& operator+=(point_cloud_t&& other);
};

}  // namespace toolbox::types

// 包含实现文件 / Include the implementation file
#include "cpp-toolbox/types/impl/point_impl.hpp"