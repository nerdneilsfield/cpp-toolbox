#pragma once

#include <cmath>
#include <iterator>  // For std::make_move_iterator
#include <limits>
#include <ostream>
#include <stdexcept>
#include <type_traits>  // For std::is_floating_point_v, std::is_integral_v
#include <vector>

#include <cpp-toolbox/types/point.hpp>  // Include the header with declarations

namespace toolbox::types
{

// --- point_t Implementations ---

template<typename T>
point_t<T>::point_t(T x_coord, T y_coord, T z_coord)
    : x(x_coord)
    , y(y_coord)
    , z(z_coord)
{
}

template<typename T>
point_t<T>::point_t()
    : x(T {})
    , y(T {})
    , z(T {})
{
}

template<typename T>
point_t<T>& point_t<T>::operator+=(const point_t& other)
{
  x += other.x;
  y += other.y;
  z += other.z;
  return *this;
}

template<typename T>
point_t<T>& point_t<T>::operator-=(const point_t& other)
{
  x -= other.x;
  y -= other.y;
  z -= other.z;
  return *this;
}

template<typename T>
point_t<T>& point_t<T>::operator*=(const T& scalar)
{
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}

template<typename T>
point_t<T>& point_t<T>::operator/=(const T& scalar)
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

template<typename T>
[[nodiscard]] auto point_t<T>::dot(const point_t& other) const -> T
{
  return (x * other.x) + (y * other.y) + (z * other.z);
}

template<typename T>
[[nodiscard]] auto point_t<T>::cross(const point_t& other) const -> point_t
{
  return point_t((y * other.z) - (z * other.y),
                 (z * other.x) - (x * other.z),
                 (x * other.y) - (y * other.x));
}

template<typename T>
[[nodiscard]] auto point_t<T>::norm() const -> double
{
  const double dx = static_cast<double>(x);
  const double dy = static_cast<double>(y);
  const double dz = static_cast<double>(z);
  return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

template<typename T>
[[nodiscard]] auto point_t<T>::normalize() const -> point_t<double>
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

template<typename T>
[[nodiscard]] auto point_t<T>::p_norm(const double& p_value) const -> double
{
  const double dx = static_cast<double>(x);
  const double dy = static_cast<double>(y);
  const double dz = static_cast<double>(z);
  return std::pow(std::pow(std::abs(dx), p_value)
                      + std::pow(std::abs(dy), p_value)
                      + std::pow(std::abs(dz), p_value),
                  1.0 / p_value);
}

template<typename T>
[[nodiscard]] auto point_t<T>::p_normalize(const double& p_value) const
    -> point_t<double>
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

template<typename T>
[[nodiscard]] auto point_t<T>::distance(const point_t& other) const -> double
{
  const point_t diff(other.x - x, other.y - y, other.z - z);
  return diff.norm();
}

template<typename T>
[[nodiscard]] auto point_t<T>::distance_p(const point_t& other,
                                          const double& p_value) const -> double
{
  const point_t diff(other.x - x, other.y - y, other.z - z);
  return diff.p_norm(p_value);
}

template<typename T>
bool point_t<T>::operator==(const point_t& other) const
{
  if constexpr (std::is_floating_point_v<T>) {
    constexpr T epsilon = std::numeric_limits<T>::epsilon() * 100;
    return (std::abs(x - other.x) <= epsilon)
        && (std::abs(y - other.y) <= epsilon)
        && (std::abs(z - other.z) <= epsilon);
  } else {
    // For integral types, direct comparison is sufficient
    return (x == other.x) && (y == other.y) && (z == other.z);
  }
}

template<typename T>
bool point_t<T>::operator!=(const point_t& other) const
{
  return !(*this == other);
}

template<typename T>
bool point_t<T>::operator<(const point_t& other) const
{
  if constexpr (std::is_floating_point_v<T>) {
    constexpr T epsilon = std::numeric_limits<T>::epsilon() * 100;
    if (std::abs(x - other.x) > epsilon) {
      return x < other.x;
    }
    if (std::abs(y - other.y) > epsilon) {
      return y < other.y;
    }
    // Compare z directly if x and y are close enough
    // Use a small adjustment for floating point comparison robustness
    return z < other.z - epsilon;
  } else {
    // For integral types, direct comparison is fine
    if (x != other.x) {
      return x < other.x;
    }
    if (y != other.y) {
      return y < other.y;
    }
    return z < other.z;
  }
}

// Static member general template definitions
template<typename T>
auto point_t<T>::min_value() -> point_t<T>
{
  if constexpr (std::numeric_limits<T>::is_specialized) {
    T min_val = std::numeric_limits<T>::is_integer
        ? std::numeric_limits<T>::min()
        : std::numeric_limits<T>::lowest();
    return point_t<T>(min_val, min_val, min_val);
  } else {
    // Provide a default for types without numeric_limits (though unlikely for
    // points)
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
    // Provide a default for types without numeric_limits
    return point_t<T>();
  }
}

// --- point_cloud_t Implementations ---

template<typename T>
point_cloud_t<T>::point_cloud_t()
    : intensity(T {})
{
}

template<typename T>
point_cloud_t<T>::point_cloud_t(const point_cloud_t& other)
    : base_file_data_t(other)  // Assuming base class needs copy construction
    , points(other.points)
    , normals(other.normals)
    , colors(other.colors)
    , intensity(other.intensity)
{
}

template<typename T>
point_cloud_t<T>::point_cloud_t(point_cloud_t&& other) noexcept
    : base_file_data_t(
          std::move(other))  // Assuming base class needs move construction
    , points(std::move(other.points))
    , normals(std::move(other.normals))
    , colors(std::move(other.colors))
    , intensity(std::move(other.intensity))
{
  other.intensity = T {};  // Reset moved-from object
}

template<typename T>
point_cloud_t<T>& point_cloud_t<T>::operator=(const point_cloud_t& other)
{
  if (this != &other) {
    // Assuming base class needs copy assignment
    base_file_data_t::operator=(other);
    points = other.points;
    normals = other.normals;
    colors = other.colors;
    intensity = other.intensity;
  }
  return *this;
}

template<typename T>
point_cloud_t<T>& point_cloud_t<T>::operator=(point_cloud_t&& other) noexcept
{
  if (this != &other) {
    // Assuming base class needs move assignment
    base_file_data_t::operator=(std::move(other));
    points = std::move(other.points);
    normals = std::move(other.normals);
    colors = std::move(other.colors);
    intensity = std::move(other.intensity);
    other.intensity = T {};  // Reset moved-from object
  }
  return *this;
}

template<typename T>
[[nodiscard]] auto point_cloud_t<T>::size() const -> std::size_t
{
  return points.size();
}

template<typename T>
[[nodiscard]] auto point_cloud_t<T>::empty() const -> bool
{
  return points.empty();
}

template<typename T>
void point_cloud_t<T>::clear()
{
  points.clear();
  normals.clear();
  colors.clear();
  intensity = T {};
}

template<typename T>
void point_cloud_t<T>::reserve(const std::size_t& required_size)
{
  points.reserve(required_size);
  // Only reserve if normals/colors might be used - adjust if needed
  if (!normals.empty() || required_size > 0)  // Simple heuristic
    normals.reserve(required_size);
  if (!colors.empty() || required_size > 0)  // Simple heuristic
    colors.reserve(required_size);
}

template<typename T>
[[nodiscard]] auto point_cloud_t<T>::operator+(const point_cloud_t& other) const
    -> point_cloud_t
{
  point_cloud_t result = *this;
  result += other;  // Use the += operator
  return result;
}

template<typename T>
[[nodiscard]] auto point_cloud_t<T>::operator+(const point_t<T>& point) const
    -> point_cloud_t
{
  point_cloud_t result = *this;
  result += point;  // Use the += operator
  return result;
}

template<typename T>
[[nodiscard]] auto point_cloud_t<T>::operator+(point_t<T>&& point) const
    -> point_cloud_t
{
  point_cloud_t result = *this;
  result += std::move(point);  // Use the move += operator
  return result;
}

template<typename T>
[[nodiscard]] auto point_cloud_t<T>::operator+(point_cloud_t&& other) const
    -> point_cloud_t
{
  point_cloud_t result = *this;
  result += std::move(other);  // Use the move += operator
  return result;
}

template<typename T>
point_cloud_t<T>& point_cloud_t<T>::operator+=(const point_t<T>& point)
{
  points.push_back(point);
  // Handle potential mismatch if normals/colors exist
  if (!normals.empty())
    normals.emplace_back();  // Add default normal if needed
  if (!colors.empty())
    colors.emplace_back();  // Add default color if needed
  return *this;
}

template<typename T>
point_cloud_t<T>& point_cloud_t<T>::operator+=(point_t<T>&& point)
{
  points.push_back(std::move(point));
  // Handle potential mismatch if normals/colors exist
  if (!normals.empty())
    normals.emplace_back();  // Add default normal if needed
  if (!colors.empty())
    colors.emplace_back();  // Add default color if needed
  return *this;
}

template<typename T>
point_cloud_t<T>& point_cloud_t<T>::operator+=(const point_cloud_t& other)
{
  if (this != &other) {
    const std::size_t original_size = points.size();
    points.insert(points.end(), other.points.begin(), other.points.end());

    const bool this_has_normals = !normals.empty();
    const bool other_has_normals = !other.normals.empty();
    const bool this_has_colors = !colors.empty();
    const bool other_has_colors = !other.colors.empty();

    if (this_has_normals && other_has_normals) {
      normals.insert(normals.end(), other.normals.begin(), other.normals.end());
    } else if (other_has_normals) {  // Only other has normals
      normals.resize(original_size);  // Ensure original part has defaults
      normals.insert(normals.end(), other.normals.begin(), other.normals.end());
    } else if (this_has_normals) {  // Only this has normals
      normals.resize(points.size());  // Resize to new total size with defaults
    }
    // If neither has normals, do nothing.

    if (this_has_colors && other_has_colors) {
      colors.insert(colors.end(), other.colors.begin(), other.colors.end());
    } else if (other_has_colors) {  // Only other has colors
      colors.resize(original_size);  // Ensure original part has defaults
      colors.insert(colors.end(), other.colors.begin(), other.colors.end());
    } else if (this_has_colors) {  // Only this has colors
      colors.resize(points.size());  // Resize to new total size with defaults
    }
    // If neither has colors, do nothing.

    // Note: Intensity addition might not always be meaningful, depends on
    // context.
    intensity += other.intensity;
  }
  return *this;
}

template<typename T>
point_cloud_t<T>& point_cloud_t<T>::operator+=(point_cloud_t&& other)
{
  if (this != &other) {
    const std::size_t original_size = points.size();
    // Use move iterators for potentially better performance
    points.insert(points.end(),
                  std::make_move_iterator(other.points.begin()),
                  std::make_move_iterator(other.points.end()));

    const bool this_has_normals = !normals.empty();
    const bool other_has_normals = !other.normals.empty();
    const bool this_has_colors = !colors.empty();
    const bool other_has_colors = !other.colors.empty();

    if (this_has_normals && other_has_normals) {
      normals.insert(normals.end(),
                     std::make_move_iterator(other.normals.begin()),
                     std::make_move_iterator(other.normals.end()));
    } else if (other_has_normals) {  // Only other has normals
      normals.resize(original_size);  // Ensure original part has defaults
      normals.insert(normals.end(),
                     std::make_move_iterator(other.normals.begin()),
                     std::make_move_iterator(other.normals.end()));
    } else if (this_has_normals) {  // Only this has normals
      normals.resize(points.size());  // Resize to new total size with defaults
    }
    // If neither has normals, do nothing.

    if (this_has_colors && other_has_colors) {
      colors.insert(colors.end(),
                    std::make_move_iterator(other.colors.begin()),
                    std::make_move_iterator(other.colors.end()));
    } else if (other_has_colors) {  // Only other has colors
      colors.resize(original_size);  // Ensure original part has defaults
      colors.insert(colors.end(),
                    std::make_move_iterator(other.colors.begin()),
                    std::make_move_iterator(other.colors.end()));
    } else if (this_has_colors) {  // Only this has colors
      colors.resize(points.size());  // Resize to new total size with defaults
    }
    // If neither has colors, do nothing.

    intensity += other.intensity;  // Add intensity
    other.clear();  // Clear the moved-from object
  }
  return *this;
}

// --- operator<< Implementation ---

template<typename T>
auto operator<<(std::ostream& output_stream, const point_t<T>& pt)
    -> std::ostream&
{
  // Consider using iomanip for better formatting control if needed
  output_stream << "(" << pt.x << ", " << pt.y << ", " << pt.z << ")";
  return output_stream;
}

}  // namespace toolbox::types
