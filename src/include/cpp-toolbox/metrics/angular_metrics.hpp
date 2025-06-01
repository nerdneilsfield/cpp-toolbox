#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>

#include <cpp-toolbox/metrics/base_metric.hpp>

namespace toolbox::metrics
{

template<typename T>
class CosineMetric : public base_metric_t<CosineMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dot_product {};
    T norm_a {};
    T norm_b {};
    
    for (std::size_t i = 0; i < size; ++i) {
      dot_product += a[i] * b[i];
      norm_a += a[i] * a[i];
      norm_b += b[i] * b[i];
    }
    
    norm_a = std::sqrt(norm_a);
    norm_b = std::sqrt(norm_b);
    
    if (norm_a < std::numeric_limits<T>::epsilon() || 
        norm_b < std::numeric_limits<T>::epsilon()) {
      return T(1);  // Maximum distance for zero vectors
    }
    
    // Cosine similarity is in [-1, 1], convert to distance in [0, 2]
    T cosine_similarity = dot_product / (norm_a * norm_b);
    
    // Clamp to handle numerical errors
    cosine_similarity = std::max(T(-1), std::min(T(1), cosine_similarity));
    
    // Convert to distance: 1 - cosine_similarity gives [0, 2]
    return T(1) - cosine_similarity;
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

template<typename T>
class AngularMetric : public base_metric_t<AngularMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dot_product {};
    T norm_a {};
    T norm_b {};
    
    for (std::size_t i = 0; i < size; ++i) {
      dot_product += a[i] * b[i];
      norm_a += a[i] * a[i];
      norm_b += b[i] * b[i];
    }
    
    norm_a = std::sqrt(norm_a);
    norm_b = std::sqrt(norm_b);
    
    if (norm_a < std::numeric_limits<T>::epsilon() || 
        norm_b < std::numeric_limits<T>::epsilon()) {
      return T(M_PI);  // Maximum angular distance
    }
    
    // Compute cosine of angle
    T cosine = dot_product / (norm_a * norm_b);
    
    // Clamp to handle numerical errors
    cosine = std::max(T(-1), std::min(T(1), cosine));
    
    // Return angle in radians [0, pi]
    return std::acos(cosine);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

// Normalized angular distance (scaled to [0, 1])
template<typename T>
class NormalizedAngularMetric : public base_metric_t<NormalizedAngularMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dot_product {};
    T norm_a {};
    T norm_b {};
    
    for (std::size_t i = 0; i < size; ++i) {
      dot_product += a[i] * b[i];
      norm_a += a[i] * a[i];
      norm_b += b[i] * b[i];
    }
    
    norm_a = std::sqrt(norm_a);
    norm_b = std::sqrt(norm_b);
    
    if (norm_a < std::numeric_limits<T>::epsilon() || 
        norm_b < std::numeric_limits<T>::epsilon()) {
      return T(1);  // Maximum normalized distance
    }
    
    // Compute cosine of angle
    T cosine = dot_product / (norm_a * norm_b);
    
    // Clamp to handle numerical errors
    cosine = std::max(T(-1), std::min(T(1), cosine));
    
    // Return normalized angle [0, 1]
    return std::acos(cosine) / T(M_PI);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

// Correlation distance (based on Pearson correlation)
template<typename T>
class CorrelationMetric : public base_metric_t<CorrelationMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    if (size == 0) {
      return T(0);
    }
    
    // Compute means
    T mean_a = std::accumulate(a, a + size, T(0)) / size;
    T mean_b = std::accumulate(b, b + size, T(0)) / size;
    
    // Compute correlation
    T cov {};
    T var_a {};
    T var_b {};
    
    for (std::size_t i = 0; i < size; ++i) {
      T diff_a = a[i] - mean_a;
      T diff_b = b[i] - mean_b;
      cov += diff_a * diff_b;
      var_a += diff_a * diff_a;
      var_b += diff_b * diff_b;
    }
    
    if (var_a < std::numeric_limits<T>::epsilon() || 
        var_b < std::numeric_limits<T>::epsilon()) {
      return T(1);  // Maximum distance for constant vectors
    }
    
    T correlation = cov / std::sqrt(var_a * var_b);
    
    // Clamp to handle numerical errors
    correlation = std::max(T(-1), std::min(T(1), correlation));
    
    // Convert correlation to distance: (1 - correlation) / 2 gives [0, 1]
    return (T(1) - correlation) / T(2);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

// Inner product distance (for normalized vectors)
template<typename T>
class InnerProductMetric : public base_metric_t<InnerProductMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T inner_product {};
    
    for (std::size_t i = 0; i < size; ++i) {
      inner_product += a[i] * b[i];
    }
    
    // For normalized vectors, inner product is in [-1, 1]
    // Convert to distance
    return T(1) - inner_product;
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

}  // namespace toolbox::metrics