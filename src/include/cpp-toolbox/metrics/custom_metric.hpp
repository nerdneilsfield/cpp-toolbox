#pragma once

#include <functional>
#include <memory>

#include <cpp-toolbox/metrics/base_metric.hpp>

namespace toolbox::metrics
{

// Function-based custom metric
template<typename T>
class CustomMetric : public base_metric_t<CustomMetric<T>, T>
{
public:
  using element_type = T;
  using distance_function = std::function<T(const T*, const T*, std::size_t)>;

  explicit CustomMetric(distance_function dist_func)
      : dist_func_(std::move(dist_func))
  {
    if (!dist_func_) {
      throw std::invalid_argument("Distance function cannot be null");
    }
  }

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    return dist_func_(a, b, size);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }

private:
  distance_function dist_func_;
};

// Lambda-based custom metric with state
template<typename T, typename Lambda>
class LambdaMetric : public base_metric_t<LambdaMetric<T, Lambda>, T>
{
public:
  using element_type = T;

  explicit LambdaMetric(Lambda lambda) : lambda_(std::move(lambda)) {}

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    return lambda_(a, b, size);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }

private:
  Lambda lambda_;
};

// Factory function for creating lambda metrics
template<typename T, typename Lambda>
auto make_lambda_metric(Lambda&& lambda)
{
  return LambdaMetric<T, std::decay_t<Lambda>>(std::forward<Lambda>(lambda));
}

// Weighted metric wrapper
template<typename BaseMetric>
class WeightedMetric : public base_metric_t<WeightedMetric<BaseMetric>, 
                                           typename BaseMetric::element_type>
{
public:
  using base_metric_type = BaseMetric;
  using element_type = typename BaseMetric::element_type;

  WeightedMetric(BaseMetric base_metric, const std::vector<element_type>& weights)
      : base_metric_(std::move(base_metric)), weights_(weights) {}

  constexpr element_type distance_impl(const element_type* a, 
                                      const element_type* b, 
                                      std::size_t size) const
  {
    if (weights_.size() != size) {
      throw std::invalid_argument("Weight vector size mismatch");
    }

    // Create weighted versions of the input vectors
    std::vector<element_type> weighted_a(size);
    std::vector<element_type> weighted_b(size);

    for (std::size_t i = 0; i < size; ++i) {
      element_type w = std::sqrt(weights_[i]);
      weighted_a[i] = a[i] * w;
      weighted_b[i] = b[i] * w;
    }

    return base_metric_.distance(weighted_a.data(), weighted_b.data(), size);
  }

  constexpr element_type squared_distance_impl(const element_type* a, 
                                              const element_type* b, 
                                              std::size_t size) const
  {
    element_type dist = distance_impl(a, b, size);
    return dist * dist;
  }

private:
  BaseMetric base_metric_;
  std::vector<element_type> weights_;
};

// Factory function for creating weighted metrics
template<typename BaseMetric>
auto make_weighted_metric(BaseMetric&& metric, 
                         const std::vector<typename std::decay_t<BaseMetric>::element_type>& weights)
{
  return WeightedMetric<std::decay_t<BaseMetric>>(
      std::forward<BaseMetric>(metric), weights);
}

// Minkowski-like custom metric with parameter
template<typename T>
class ParameterizedMetric : public base_metric_t<ParameterizedMetric<T>, T>
{
public:
  using element_type = T;

  ParameterizedMetric() = default;

  void set_parameter(T param) { parameter_ = param; }
  T get_parameter() const { return parameter_; }

  // Note: This is an abstract class, derived classes must implement distance_impl
  // We provide squared_distance_impl here for convenience
  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = static_cast<const ParameterizedMetric*>(this)->distance_impl(a, b, size);
    return dist * dist;
  }

protected:
  T parameter_ = T(2);  // Default to L2-like behavior
};

// Composite metric that combines multiple metrics
template<typename T>
class CompositeMetric : public base_metric_t<CompositeMetric<T>, T>
{
public:
  using element_type = T;
  using metric_ptr = std::shared_ptr<base_metric_t<void, T>>;

  void add_metric(metric_ptr metric, T weight = T(1))
  {
    metrics_.emplace_back(std::move(metric), weight);
  }

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T total_distance {};
    T total_weight {};

    for (const auto& [metric, weight] : metrics_) {
      total_distance += weight * metric->distance(a, b, size);
      total_weight += weight;
    }

    if (total_weight > T(0)) {
      return total_distance / total_weight;
    }
    return T(0);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    T dist = distance_impl(a, b, size);
    return dist * dist;
  }

private:
  std::vector<std::pair<metric_ptr, T>> metrics_;
};

// Mahalanobis-like metric (simplified version without full covariance matrix)
template<typename T>
class ScaledMetric : public base_metric_t<ScaledMetric<T>, T>
{
public:
  using element_type = T;

  explicit ScaledMetric(const std::vector<T>& scales)
      : scales_(scales) {}

  constexpr T distance_impl(const T* a, const T* b, std::size_t size) const
  {
    if (scales_.size() != size) {
      throw std::invalid_argument("Scale vector size mismatch");
    }

    T sum {};
    for (std::size_t i = 0; i < size; ++i) {
      T diff = (a[i] - b[i]) / scales_[i];
      sum += diff * diff;
    }
    return std::sqrt(sum);
  }

  constexpr T squared_distance_impl(const T* a, const T* b, std::size_t size) const
  {
    if (scales_.size() != size) {
      throw std::invalid_argument("Scale vector size mismatch");
    }

    T sum {};
    for (std::size_t i = 0; i < size; ++i) {
      T diff = (a[i] - b[i]) / scales_[i];
      sum += diff * diff;
    }
    return sum;  // Return squared distance directly (no sqrt)
  }

private:
  std::vector<T> scales_;
};

}  // namespace toolbox::metrics