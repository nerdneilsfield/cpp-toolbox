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
 * @brief A 3D point/vector template class
 * @tparam T The numeric type for coordinates (e.g. float, double, int)
 *
 * This class represents a 3D point or vector with x, y, z coordinates.
 * It provides common vector operations and supports arithmetic operators.
 *
 * @code{.cpp}
 * // Create points
 * point_t<double> p1(1.0, 2.0, 3.0);
 * point_t<double> p2(4.0, 5.0, 6.0);
 *
 * // Vector operations
 * double dot_product = p1.dot(p2);
 * point_t<double> cross_product = p1.cross(p2);
 * double norm = p1.norm();
 *
 * // Arithmetic
 * point_t<double> sum = p1 + p2;
 * point_t<double> scaled = p1 * 2.0;
 * @endcode
 */
template<typename T>
struct CPP_TOOLBOX_EXPORT point_t
{
  T x;  ///< X coordinate
  T y;  ///< Y coordinate
  T z;  ///< Z coordinate

  // Define destructor (Rule of Five/Zero)
  ~point_t() = default;

  /**
   * @brief Constructs a point with given coordinates
   * @param x_coord X coordinate
   * @param y_coord Y coordinate
   * @param z_coord Z coordinate
   */
  point_t(T x_coord, T y_coord, T z_coord);

  /**
   * @brief Default constructor, initializes to origin (0,0,0)
   */
  point_t();

  point_t(const point_t& other) = default;  ///< Copy constructor
  point_t(point_t&& other) noexcept = default;  ///< Move constructor

  point_t& operator=(const point_t& other) = default;  ///< Copy assignment
  point_t& operator=(point_t&& other) noexcept = default;  ///< Move assignment

  /**
   * @brief Add another point component-wise
   * @param other Point to add
   * @return Reference to this point
   */
  point_t& operator+=(const point_t& other);

  /**
   * @brief Subtract another point component-wise
   * @param other Point to subtract
   * @return Reference to this point
   */
  point_t& operator-=(const point_t& other);

  /**
   * @brief Multiply coordinates by a scalar
   * @param scalar Value to multiply by
   * @return Reference to this point
   */
  point_t& operator*=(const T& scalar);

  /**
   * @brief Divide coordinates by a scalar
   * @param scalar Value to divide by
   * @return Reference to this point
   * @throws std::runtime_error if scalar is zero
   */
  point_t& operator/=(const T& scalar);

  /**
   * @brief Calculate dot product with another point
   * @param other Point to calculate dot product with
   * @return Dot product value
   */
  [[nodiscard]] auto dot(const point_t& other) const -> T;

  /**
   * @brief Calculate cross product with another point
   * @param other Point to calculate cross product with
   * @return New point representing cross product
   */
  [[nodiscard]] auto cross(const point_t& other) const -> point_t;

  /**
   * @brief Calculate Euclidean norm (length) of vector
   * @return Norm as double
   */
  [[nodiscard]] auto norm() const -> double;

  /**
   * @brief Return normalized vector (unit length)
   * @return Normalized vector as point_t<double>
   */
  [[nodiscard]] auto normalize() const -> point_t<double>;

  /**
   * @brief Calculate p-norm of vector
   * @param p_value The p value for norm calculation
   * @return p-norm as double
   */
  [[nodiscard]] auto p_norm(const double& p_value) const -> double;

  /**
   * @brief Return p-normalized vector
   * @param p_value The p value for normalization
   * @return p-normalized vector as point_t<double>
   */
  [[nodiscard]] auto p_normalize(const double& p_value) const
      -> point_t<double>;

  /**
   * @brief Calculate Euclidean distance to another point
   * @param other Point to calculate distance to
   * @return Distance as double
   */
  [[nodiscard]] auto distance(const point_t& other) const -> double;

  /**
   * @brief Calculate p-distance to another point
   * @param other Point to calculate distance to
   * @param p_value The p value for distance calculation
   * @return p-distance as double
   */
  [[nodiscard]] auto distance_p(const point_t& other,
                                const double& p_value) const -> double;

  // Define comparison operators
  bool operator==(const point_t& other) const;

  bool operator!=(const point_t& other) const;

  // Add a basic operator< for testing purposes
  // Note: This provides an arbitrary but consistent ordering.
  bool operator<(const point_t& other) const;

  /**
   * @brief Get point with minimum possible values
   * @return Point with minimum coordinates
   */
  [[nodiscard]] static auto min_value() -> point_t;

  /**
   * @brief Get point with maximum possible values
   * @return Point with maximum coordinates
   */
  [[nodiscard]] static auto max_value() -> point_t;
};

// Specializations for static members (keep inline definitions here)
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
 * @brief Stream output operator for point_t - Declaration Only
 * @param output_stream Output stream
 * @param pt Point to output
 * @return Reference to output stream
 */
template<typename T>
auto operator<<(std::ostream& output_stream, const point_t<T>& pt)
    -> std::ostream&;

/**
 * @brief A point cloud class containing points and associated data
 * @tparam T The numeric type for point coordinates
 *
 * This class represents a collection of 3D points with optional normals,
 * colors and intensity data.
 *
 * @code{.cpp}
 * // Create point cloud
 * point_cloud_t<double> cloud;
 *
 * // Add points
 * cloud += point_t<double>(1.0, 2.0, 3.0);
 * cloud += point_t<double>(4.0, 5.0, 6.0);
 *
 * // Access data
 * std::size_t num_points = cloud.size();
 * point_t<double> first_point = cloud.points[0];
 * double intensity = cloud.intensity;
 * @endcode
 */
template<typename T>
class CPP_TOOLBOX_EXPORT point_cloud_t : public ::toolbox::io::base_file_data_t
{
public:
  std::vector<point_t<T>> points;  ///< Point coordinates
  std::vector<point_t<T>> normals;  ///< Point normals (optional)
  std::vector<point_t<T>> colors;  ///< Point colors (optional)
  T intensity;  ///< Global intensity value

  ~point_cloud_t() = default;

  point_cloud_t();

  point_cloud_t(const point_cloud_t& other);
  point_cloud_t(point_cloud_t&& other) noexcept;
  point_cloud_t& operator=(const point_cloud_t& other);
  point_cloud_t& operator=(point_cloud_t&& other) noexcept;

  /**
   * @brief Get number of points in cloud
   * @return Number of points
   */
  [[nodiscard]] auto size() const -> std::size_t;

  /**
   * @brief Check if cloud is empty
   * @return true if empty, false otherwise
   */
  [[nodiscard]] auto empty() const -> bool;

  /**
   * @brief Clear all data from cloud
   */
  void clear();

  /**
   * @brief Reserve memory for points
   * @param required_size Number of points to reserve space for
   */
  void reserve(const std::size_t& required_size);

  /**
   * @brief Add two point clouds
   * @param other Cloud to add
   * @return New combined cloud
   */
  [[nodiscard]] auto operator+(const point_cloud_t& other) const
      -> point_cloud_t;

  /**
   * @brief Add point to cloud
   * @param point Point to add
   * @return New cloud with added point
   */
  [[nodiscard]] auto operator+(const point_t<T>& point) const -> point_cloud_t;

  /**
   * @brief Add point to cloud (move version)
   * @param point Point to add
   * @return New cloud with added point
   */
  [[nodiscard]] auto operator+(point_t<T>&& point) const -> point_cloud_t;

  /**
   * @brief Add two point clouds (move version)
   * @param other Cloud to add
   * @return New combined cloud
   */
  [[nodiscard]] auto operator+(point_cloud_t&& other) const -> point_cloud_t;

  /**
   * @brief Add point to cloud
   * @param point Point to add
   * @return Reference to this cloud
   */
  point_cloud_t& operator+=(const point_t<T>& point);

  /**
   * @brief Add point to cloud (move version)
   * @param point Point to add
   * @return Reference to this cloud
   */
  point_cloud_t& operator+=(point_t<T>&& point);

  /**
   * @brief Add another cloud to this one
   * @param other Cloud to add
   * @return Reference to this cloud
   */
  point_cloud_t& operator+=(const point_cloud_t& other);

  /**
   * @brief Add another cloud to this one (move version)
   * @param other Cloud to add
   * @return Reference to this cloud
   */
  point_cloud_t& operator+=(point_cloud_t&& other);
};

}  // namespace toolbox::types

// Include the implementation file
#include "cpp-toolbox/types/impl/point_impl.hpp"