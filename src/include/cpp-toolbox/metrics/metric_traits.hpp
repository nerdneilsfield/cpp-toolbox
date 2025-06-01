#pragma once

#include <type_traits>
#include <limits>

#include <cpp-toolbox/metrics/base_metric.hpp>

namespace toolbox::metrics
{

// Forward declarations
template<typename T> class L1Metric;
template<typename T> class L2Metric;
template<typename T> class LinfMetric;
template<typename T, int P> class LpMetric;
template<typename T> class ChiSquaredMetric;
template<typename T> class HistogramIntersectionMetric;
template<typename T> class BhattacharyyaMetric;
template<typename T> class CosineMetric;
template<typename T> class AngularMetric;

// Base metric traits
template<typename Metric>
struct metric_traits
{
  using metric_type = Metric;
  using element_type = typename Metric::element_type;
  using result_type = typename Metric::result_type;
  
  static constexpr bool is_symmetric = true;
  static constexpr bool is_squared = false;
  static constexpr bool has_squared_form = false;
  static constexpr bool is_normalized = false;
  static constexpr bool requires_positive_values = false;
  
  // Default bounds
  static constexpr result_type min_distance = 0;
  static constexpr result_type max_distance = std::numeric_limits<result_type>::max();
};

// Specializations for specific metrics
template<typename T>
struct metric_traits<L2Metric<T>>
{
  using metric_type = L2Metric<T>;
  using element_type = T;
  using result_type = T;
  
  static constexpr bool is_symmetric = true;
  static constexpr bool is_squared = false;
  static constexpr bool has_squared_form = true;
  static constexpr bool is_normalized = false;
  static constexpr bool requires_positive_values = false;
  
  static constexpr result_type min_distance = 0;
  static constexpr result_type max_distance = std::numeric_limits<result_type>::max();
};

template<typename T>
struct metric_traits<L1Metric<T>>
{
  using metric_type = L1Metric<T>;
  using element_type = T;
  using result_type = T;
  
  static constexpr bool is_symmetric = true;
  static constexpr bool is_squared = false;
  static constexpr bool has_squared_form = false;
  static constexpr bool is_normalized = false;
  static constexpr bool requires_positive_values = false;
  
  static constexpr result_type min_distance = 0;
  static constexpr result_type max_distance = std::numeric_limits<result_type>::max();
};

// Helper templates for C++17 (instead of concepts)
template<typename Metric>
constexpr bool is_symmetric_metric_v = metric_traits<Metric>::is_symmetric;

template<typename Metric>
constexpr bool is_normalized_metric_v = metric_traits<Metric>::is_normalized;

template<typename Metric>
constexpr bool has_squared_form_v = metric_traits<Metric>::has_squared_form;

// Type traits helpers
template<typename T>
struct is_metric : std::false_type {};

template<typename Derived, typename ElementType>
struct is_metric<base_metric_t<Derived, ElementType>> : std::true_type {};

template<typename T>
inline constexpr bool is_metric_v = is_metric<T>::value;

}  // namespace toolbox::metrics