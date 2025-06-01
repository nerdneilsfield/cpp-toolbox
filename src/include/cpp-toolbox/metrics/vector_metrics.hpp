#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <stdexcept>

#include <cpp-toolbox/metrics/base_metric.hpp>

namespace toolbox::metrics
{

template<typename T>
class L2Metric : public base_metric_t<L2Metric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    return std::sqrt(squared_distance_impl(a, b, size));
  }

  constexpr T squared_distance_impl(const T* a,
                                    const T* b,
                                    std::size_t size) const
  {
    T sum {};
    for (std::size_t i = 0; i < size; ++i) {
      T diff = a[i] - b[i];
      sum += diff * diff;
    }
    return sum;
  }
};

template<typename T>
class L1Metric : public base_metric_t<L1Metric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T sum {};
    for (std::size_t i = 0; i < size; ++i) {
      sum += std::abs(a[i] - b[i]);
    }
    return sum;
  }

  constexpr T squared_distance_impl(const T* a,
                                    const T* b,
                                    std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

template<typename T>
class LinfMetric : public base_metric_t<LinfMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T max_diff {};
    for (std::size_t i = 0; i < size; ++i) {
      T diff = std::abs(a[i] - b[i]);
      if (diff > max_diff) {
        max_diff = diff;
      }
    }
    return max_diff;
  }

  constexpr T squared_distance_impl(const T* a,
                                    const T* b,
                                    std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

template<typename T, int P = 2>
class LpMetric : public base_metric_t<LpMetric<T, P>, T>
{
public:
  using element_type = T;
  static constexpr int p_value = P;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    static_assert(P > 0, "P must be positive");
    
    if constexpr (P == 1) {
      // Special case for L1
      T sum {};
      for (std::size_t i = 0; i < size; ++i) {
        sum += std::abs(a[i] - b[i]);
      }
      return sum;
    }
    else if constexpr (P == 2) {
      // Special case for L2
      return std::sqrt(squared_distance_impl(a, b, size));
    }
    else {
      T sum {};
      for (std::size_t i = 0; i < size; ++i) {
        T diff = std::abs(a[i] - b[i]);
        sum += std::pow(diff, P);
      }
      return std::pow(sum, T(1) / P);
    }
  }

  constexpr T squared_distance_impl(const T* a,
                                    const T* b,
                                    std::size_t size) const
  {
    if constexpr (P == 2) {
      T sum {};
      for (std::size_t i = 0; i < size; ++i) {
        T diff = a[i] - b[i];
        sum += diff * diff;
      }
      return sum;
    }
    else {
      T dist = distance_impl(a, b, size);
      return dist * dist;
    }
  }
};

// Specialization for dynamic p-value
template<typename T>
class GeneralizedLpMetric : public base_metric_t<GeneralizedLpMetric<T>, T>
{
public:
  using element_type = T;

  explicit GeneralizedLpMetric(T p) : p_(p) 
  {
    if (p <= 0) {
      throw std::invalid_argument("P must be positive");
    }
  }

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    if (std::abs(p_ - T(1)) < std::numeric_limits<T>::epsilon()) {
      // L1 norm
      T sum {};
      for (std::size_t i = 0; i < size; ++i) {
        sum += std::abs(a[i] - b[i]);
      }
      return sum;
    }
    else if (std::abs(p_ - T(2)) < std::numeric_limits<T>::epsilon()) {
      // L2 norm
      return std::sqrt(squared_distance_impl(a, b, size));
    }
    else if (std::isinf(p_)) {
      // L-infinity norm
      T max_diff {};
      for (std::size_t i = 0; i < size; ++i) {
        T diff = std::abs(a[i] - b[i]);
        if (diff > max_diff) {
          max_diff = diff;
        }
      }
      return max_diff;
    }
    else {
      // General Lp norm
      T sum {};
      for (std::size_t i = 0; i < size; ++i) {
        T diff = std::abs(a[i] - b[i]);
        sum += std::pow(diff, p_);
      }
      return std::pow(sum, T(1) / p_);
    }
  }

  constexpr T squared_distance_impl(const T* a,
                                    const T* b,
                                    std::size_t size) const
  {
    if (std::abs(p_ - T(2)) < std::numeric_limits<T>::epsilon()) {
      T sum {};
      for (std::size_t i = 0; i < size; ++i) {
        T diff = a[i] - b[i];
        sum += diff * diff;
      }
      return sum;
    }
    else {
      T dist = distance_impl(a, b, size);
      return dist * dist;
    }
  }

  T get_p() const { return p_; }

private:
  T p_;
};

}  // namespace toolbox::metrics