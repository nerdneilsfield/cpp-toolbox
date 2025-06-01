#pragma once

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

#include <cpp-toolbox/metrics/base_metric.hpp>

namespace toolbox::metrics
{

template<typename T>
class ChiSquaredMetric : public base_metric_t<ChiSquaredMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T sum {};
    for (std::size_t i = 0; i < size; ++i) {
      T ai = a[i];
      T bi = b[i];
      T denominator = ai + bi;
      
      if (denominator > std::numeric_limits<T>::epsilon()) {
        T diff = ai - bi;
        sum += (diff * diff) / denominator;
      }
    }
    return sum * T(0.5);  // Standard chi-squared distance includes 1/2 factor
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

template<typename T>
class HistogramIntersectionMetric : public base_metric_t<HistogramIntersectionMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T intersection {};
    T sum_a {};
    T sum_b {};
    
    for (std::size_t i = 0; i < size; ++i) {
      if (a[i] < 0 || b[i] < 0) {
        throw std::invalid_argument("Histogram values must be non-negative");
      }
      intersection += std::min(a[i], b[i]);
      sum_a += a[i];
      sum_b += b[i];
    }
    
    T max_sum = std::max(sum_a, sum_b);
    if (max_sum < std::numeric_limits<T>::epsilon()) {
      return T(0);  // Both histograms are empty
    }
    
    // Return distance (1 - similarity)
    return T(1) - (intersection / max_sum);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

template<typename T>
class BhattacharyyaMetric : public base_metric_t<BhattacharyyaMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    // Normalize histograms
    T sum_a = std::accumulate(a, a + size, T(0));
    T sum_b = std::accumulate(b, b + size, T(0));
    
    if (sum_a < std::numeric_limits<T>::epsilon() || 
        sum_b < std::numeric_limits<T>::epsilon()) {
      return T(1);  // Maximum distance if either histogram is empty
    }
    
    // Compute Bhattacharyya coefficient
    T bc {};
    for (std::size_t i = 0; i < size; ++i) {
      T normalized_a = a[i] / sum_a;
      T normalized_b = b[i] / sum_b;
      bc += std::sqrt(normalized_a * normalized_b);
    }
    
    // Bhattacharyya distance
    if (bc >= T(1)) {
      return T(0);  // Identical histograms
    }
    if (bc <= T(0)) {
      return std::numeric_limits<T>::infinity();  // No overlap
    }
    
    return -std::log(bc);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

// Hellinger distance (related to Bhattacharyya)
template<typename T>
class HellingerMetric : public base_metric_t<HellingerMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    // Normalize histograms
    T sum_a = std::accumulate(a, a + size, T(0));
    T sum_b = std::accumulate(b, b + size, T(0));
    
    if (sum_a < std::numeric_limits<T>::epsilon() || 
        sum_b < std::numeric_limits<T>::epsilon()) {
      return T(1);  // Maximum distance if either histogram is empty
    }
    
    // Compute sum of squared differences of square roots
    T sum {};
    for (std::size_t i = 0; i < size; ++i) {
      T sqrt_a = std::sqrt(a[i] / sum_a);
      T sqrt_b = std::sqrt(b[i] / sum_b);
      T diff = sqrt_a - sqrt_b;
      sum += diff * diff;
    }
    
    // Hellinger distance
    return std::sqrt(sum / T(2));
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

// Earth Mover's Distance (EMD) / Wasserstein-1 distance for 1D histograms
template<typename T>
class EMDMetric : public base_metric_t<EMDMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    // Normalize to probability distributions
    T sum_a = std::accumulate(a, a + size, T(0));
    T sum_b = std::accumulate(b, b + size, T(0));
    
    if (sum_a < std::numeric_limits<T>::epsilon() || 
        sum_b < std::numeric_limits<T>::epsilon()) {
      return T(size);  // Maximum distance
    }
    
    // Compute cumulative distributions and their L1 distance
    T emd {};
    T cumsum_a {};
    T cumsum_b {};
    
    for (std::size_t i = 0; i < size; ++i) {
      cumsum_a += a[i] / sum_a;
      cumsum_b += b[i] / sum_b;
      emd += std::abs(cumsum_a - cumsum_b);
    }
    
    return emd;
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

// Kullback-Leibler (KL) divergence - note: not symmetric!
template<typename T>
class KLDivergenceMetric : public base_metric_t<KLDivergenceMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    // Normalize to probability distributions
    T sum_a = std::accumulate(a, a + size, T(0));
    T sum_b = std::accumulate(b, b + size, T(0));
    
    if (sum_a < std::numeric_limits<T>::epsilon()) {
      return std::numeric_limits<T>::infinity();
    }
    
    T kl {};
    const T epsilon = std::numeric_limits<T>::epsilon();
    
    for (std::size_t i = 0; i < size; ++i) {
      T p = a[i] / sum_a;
      T q = b[i] / sum_b;
      
      if (p > epsilon) {
        if (q < epsilon) {
          return std::numeric_limits<T>::infinity();
        }
        kl += p * std::log(p / q);
      }
    }
    
    return kl;
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

// Jensen-Shannon divergence (symmetric version of KL)
template<typename T>
class JensenShannonMetric : public base_metric_t<JensenShannonMetric<T>, T>
{
public:
  using element_type = T;

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    // Normalize to probability distributions
    T sum_a = std::accumulate(a, a + size, T(0));
    T sum_b = std::accumulate(b, b + size, T(0));
    
    if (sum_a < std::numeric_limits<T>::epsilon() || 
        sum_b < std::numeric_limits<T>::epsilon()) {
      return T(1);
    }
    
    T js {};
    const T epsilon = std::numeric_limits<T>::epsilon();
    
    for (std::size_t i = 0; i < size; ++i) {
      T p = a[i] / sum_a;
      T q = b[i] / sum_b;
      T m = (p + q) / T(2);
      
      if (p > epsilon && m > epsilon) {
        js += p * std::log(p / m);
      }
      if (q > epsilon && m > epsilon) {
        js += q * std::log(q / m);
      }
    }
    
    return std::sqrt(js / T(2));  // Square root for metric property
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }
};

}  // namespace toolbox::metrics