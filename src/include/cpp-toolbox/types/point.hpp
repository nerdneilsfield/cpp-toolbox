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
  point_t(T x_coord, T y_coord, T z_coord)
      : x(x_coord)
      , y(y_coord)
      , z(z_coord)
  {
  }

  /**
   * @brief Default constructor, initializes to origin (0,0,0)
   */
  point_t()
      : x(T {})
      , y(T {})
      , z(T {})
  {
  }

  point_t(const point_t& other) = default;  ///< Copy constructor
  point_t(point_t&& other) noexcept = default;  ///< Move constructor

  point_t& operator=(const point_t& other) = default;  ///< Copy assignment
  point_t& operator=(point_t&& other) noexcept = default;  ///< Move assignment

  /**
   * @brief Add another point component-wise
   * @param other Point to add
   * @return Reference to this point
   */
  point_t& operator+=(const point_t& other)
  {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  /**
   * @brief Subtract another point component-wise
   * @param other Point to subtract
   * @return Reference to this point
   */
  point_t& operator-=(const point_t& other)
  {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  /**
   * @brief Multiply coordinates by a scalar
   * @param scalar Value to multiply by
   * @return Reference to this point
   */
  point_t& operator*=(const T& scalar)
  {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
  }

  /**
   * @brief Divide coordinates by a scalar
   * @param scalar Value to divide by
   * @return Reference to this point
   * @throws std::runtime_error if scalar is zero
   */
  point_t& operator/=(const T& scalar)
  {
    if constexpr (std::is_floating_point_v<T>) {
      constexpr T epsilon = std::numeric_limits<T>::epsilon();
      if (std::abs(scalar) < epsilon) {
        throw std::runtime_error("Division by zero in point_t::operator/=");
      }
    } else if constexpr (std::is_integral_v<T>) {
      if (scalar == T {0}) {
        throw std::runtime_error("Division by zero in point_t::operator/=");
      }
    }

    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
  }

  /**
   * @brief Calculate dot product with another point
   * @param other Point to calculate dot product with
   * @return Dot product value
   */
  [[nodiscard]] auto dot(const point_t& other) const -> T
  {
    return (x * other.x) + (y * other.y) + (z * other.z);
  }

  /**
   * @brief Calculate cross product with another point
   * @param other Point to calculate cross product with
   * @return New point representing cross product
   */
  [[nodiscard]] auto cross(const point_t& other) const -> point_t
  {
    return point_t((y * other.z) - (z * other.y),
                   (z * other.x) - (x * other.z),
                   (x * other.y) - (y * other.x));
  }

  /**
   * @brief Calculate Euclidean norm (length) of vector
   * @return Norm as double
   */
  [[nodiscard]] auto norm() const -> double
  {
    const double dx = static_cast<double>(x);
    const double dy = static_cast<double>(y);
    const double dz = static_cast<double>(z);
    return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
  }

  /**
   * @brief Return normalized vector (unit length)
   * @return Normalized vector as point_t<double>
   */
  [[nodiscard]] auto normalize() const -> point_t<double>
  {
    const double length = this->norm();
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    if (std::abs(length) < epsilon) {
      return point_t<double>(0.0, 0.0, 0.0);
    }
    return point_t<double>(static_cast<double>(x) / length,
                           static_cast<double>(y) / length,
                           static_cast<double>(z) / length);
  }

  /**
   * @brief Calculate p-norm of vector
   * @param p_value The p value for norm calculation
   * @return p-norm as double
   */
  [[nodiscard]] auto p_norm(const double& p_value) const -> double
  {
    const double dx = static_cast<double>(x);
    const double dy = static_cast<double>(y);
    const double dz = static_cast<double>(z);
    return std::pow(std::pow(std::abs(dx), p_value)
                        + std::pow(std::abs(dy), p_value)
                        + std::pow(std::abs(dz), p_value),
                    1.0 / p_value);
  }

  /**
   * @brief Return p-normalized vector
   * @param p_value The p value for normalization
   * @return p-normalized vector as point_t<double>
   */
  [[nodiscard]] auto p_normalize(const double& p_value) const -> point_t<double>
  {
    const double length = this->p_norm(p_value);
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    if (std::abs(length) < epsilon) {
      return point_t<double>(0.0, 0.0, 0.0);
    }
    return point_t<double>(static_cast<double>(x) / length,
                           static_cast<double>(y) / length,
                           static_cast<double>(z) / length);
  }

  /**
   * @brief Calculate Euclidean distance to another point
   * @param other Point to calculate distance to
   * @return Distance as double
   */
  [[nodiscard]] auto distance(const point_t& other) const -> double
  {
    const point_t diff(other.x - x, other.y - y, other.z - z);
    return diff.norm();
  }

  /**
   * @brief Calculate p-distance to another point
   * @param other Point to calculate distance to
   * @param p_value The p value for distance calculation
   * @return p-distance as double
   */
  [[nodiscard]] auto distance_p(const point_t& other,
                                const double& p_value) const -> double
  {
    const point_t diff(other.x - x, other.y - y, other.z - z);
    return diff.p_norm(p_value);
  }

  // Define comparison operators
  bool operator==(const point_t& other) const
  {
    constexpr T epsilon = std::is_floating_point_v<T>
        ? std::numeric_limits<T>::epsilon() * 100
        : T {0};
    return (std::abs(x - other.x) <= epsilon)
        && (std::abs(y - other.y) <= epsilon)
        && (std::abs(z - other.z) <= epsilon);
  }

  bool operator!=(const point_t& other) const { return !(*this == other); }

  // Add a basic operator< for testing purposes
  // Note: This provides an arbitrary but consistent ordering.
  bool operator<(const point_t& other) const
  {
    constexpr T epsilon = std::is_floating_point_v<T>
        ? std::numeric_limits<T>::epsilon() * 100
        : T {0};

    if (std::abs(x - other.x) > epsilon) {
      return x < other.x;
    }
    if (std::abs(y - other.y) > epsilon) {
      return y < other.y;
    }
    return z < other.z;
  }

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

// Static member definitions
template<typename T>
auto point_t<T>::min_value() -> point_t<T>
{
  if constexpr (std::numeric_limits<T>::is_specialized) {
    T min_val = std::numeric_limits<T>::is_integer
        ? std::numeric_limits<T>::min()
        : std::numeric_limits<T>::lowest();
    return point_t<T>(min_val, min_val, min_val);
  } else {
    return point_t<T>();
  }
}

template<typename T>
auto point_t<T>::max_value() -> point_t<T>
{
  if constexpr (std::numeric_limits<T>::is_specialized) {
    return point_t<T>(std::numeric_limits<T>::max(),
                      std::numeric_limits<T>::max(),
                      std::numeric_limits<T>::max());
  } else {
    return point_t<T>();
  }
}

// Specializations for static members
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
 * @brief Stream output operator for point_t
 * @param output_stream Output stream
 * @param pt Point to output
 * @return Reference to output stream
 */
template<typename T>
auto operator<<(std::ostream& output_stream, const point_t<T>& pt)
    -> std::ostream&
{
  output_stream << "(" << pt.x << ", " << pt.y << ", " << pt.z << ")";
  return output_stream;
}

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

  point_cloud_t()
      : intensity(T {})
  {
  }

  point_cloud_t(const point_cloud_t& other)
      : points(other.points)
      , normals(other.normals)
      , colors(other.colors)
      , intensity(other.intensity)
  {
  }

  point_cloud_t(point_cloud_t&& other) noexcept
      : points(std::move(other.points))
      , normals(std::move(other.normals))
      , colors(std::move(other.colors))
      , intensity(std::move(other.intensity))
  {
    other.intensity = T {};
  }

  point_cloud_t& operator=(const point_cloud_t& other)
  {
    if (this != &other) {
      points = other.points;
      normals = other.normals;
      colors = other.colors;
      intensity = other.intensity;
    }
    return *this;
  }

  point_cloud_t& operator=(point_cloud_t&& other) noexcept
  {
    if (this != &other) {
      points = std::move(other.points);
      normals = std::move(other.normals);
      colors = std::move(other.colors);
      intensity = std::move(other.intensity);
      other.intensity = T {};
    }
    return *this;
  }

  /**
   * @brief Get number of points in cloud
   * @return Number of points
   */
  [[nodiscard]] auto size() const -> std::size_t { return points.size(); }

  /**
   * @brief Check if cloud is empty
   * @return true if empty, false otherwise
   */
  [[nodiscard]] auto empty() const -> bool { return points.empty(); }

  /**
   * @brief Clear all data from cloud
   */
  void clear()
  {
    points.clear();
    normals.clear();
    colors.clear();
    intensity = T {};
  }

  /**
   * @brief Reserve memory for points
   * @param required_size Number of points to reserve space for
   */
  void reserve(const std::size_t& required_size)
  {
    points.reserve(required_size);
    normals.reserve(required_size);
    colors.reserve(required_size);
  }

  /**
   * @brief Add two point clouds
   * @param other Cloud to add
   * @return New combined cloud
   */
  [[nodiscard]] auto operator+(const point_cloud_t& other) const
      -> point_cloud_t
  {
    point_cloud_t result = *this;
    result += other;
    return result;
  }

  /**
   * @brief Add point to cloud
   * @param point Point to add
   * @return New cloud with added point
   */
  [[nodiscard]] auto operator+(const point_t<T>& point) const -> point_cloud_t
  {
    point_cloud_t result = *this;
    result += point;
    return result;
  }

  /**
   * @brief Add point to cloud (move version)
   * @param point Point to add
   * @return New cloud with added point
   */
  [[nodiscard]] auto operator+(point_t<T>&& point) const -> point_cloud_t
  {
    point_cloud_t result = *this;
    result += std::move(point);
    return result;
  }

  /**
   * @brief Add two point clouds (move version)
   * @param other Cloud to add
   * @return New combined cloud
   */
  [[nodiscard]] auto operator+(point_cloud_t&& other) const -> point_cloud_t
  {
    point_cloud_t result = *this;
    result += std::move(other);
    return result;
  }

  /**
   * @brief Add point to cloud
   * @param point Point to add
   * @return Reference to this cloud
   */
  point_cloud_t& operator+=(const point_t<T>& point)
  {
    points.push_back(point);
    return *this;
  }

  /**
   * @brief Add point to cloud (move version)
   * @param point Point to add
   * @return Reference to this cloud
   */
  point_cloud_t& operator+=(point_t<T>&& point)
  {
    points.push_back(std::move(point));
    return *this;
  }

  /**
   * @brief Add another cloud to this one
   * @param other Cloud to add
   * @return Reference to this cloud
   */
  point_cloud_t& operator+=(const point_cloud_t& other)
  {
    if (this != &other) {
      points.insert(points.end(), other.points.begin(), other.points.end());
      if (normals.size() == points.size() - other.points.size()
          && !other.normals.empty())
      {
        normals.insert(
            normals.end(), other.normals.begin(), other.normals.end());
      } else if (!normals.empty() || !other.normals.empty()) {
        std::cerr
            << "Warning: Normals mismatch in point_cloud_t += operation.\n";
      }
      if (colors.size() == points.size() - other.points.size()
          && !other.colors.empty())
      {
        colors.insert(colors.end(), other.colors.begin(), other.colors.end());
      } else if (!colors.empty() || !other.colors.empty()) {
        std::cerr
            << "Warning: Colors mismatch in point_cloud_t += operation.\n";
      }

      intensity += other.intensity;
    }
    return *this;
  }

  /**
   * @brief Add another cloud to this one (move version)
   * @param other Cloud to add
   * @return Reference to this cloud
   */
  point_cloud_t& operator+=(point_cloud_t&& other)
  {
    if (this != &other) {
      points.insert(points.end(),
                    std::make_move_iterator(other.points.begin()),
                    std::make_move_iterator(other.points.end()));
      if (normals.size() == points.size() - other.points.size()
          && !other.normals.empty())
      {
        normals.insert(normals.end(),
                       std::make_move_iterator(other.normals.begin()),
                       std::make_move_iterator(other.normals.end()));
      } else if (!normals.empty() || !other.normals.empty()) {
        std::cerr << "Warning: Normals mismatch in point_cloud_t += move "
                     "operation.\n";
      }
      if (colors.size() == points.size() - other.colors.size()
          && !other.colors.empty())
      {
        colors.insert(colors.end(),
                      std::make_move_iterator(other.colors.begin()),
                      std::make_move_iterator(other.colors.end()));
      } else if (!colors.empty() || !other.colors.empty()) {
        std::cerr
            << "Warning: Colors mismatch in point_cloud_t += move operation.\n";
      }

      intensity += other.intensity;
      other.clear();
    }
    return *this;
  }
};

}  // namespace toolbox::types