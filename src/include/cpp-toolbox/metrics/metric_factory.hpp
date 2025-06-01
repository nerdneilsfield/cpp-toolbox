#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <vector>

#include <cpp-toolbox/metrics/base_metric.hpp>
#include <cpp-toolbox/metrics/metric_traits.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>
#include <cpp-toolbox/metrics/histogram_metrics.hpp>
#include <cpp-toolbox/metrics/angular_metrics.hpp>
#include <cpp-toolbox/metrics/custom_metric.hpp>

namespace toolbox::metrics
{

// Abstract metric interface for runtime polymorphism
template<typename T>
class IMetric
{
public:
  using element_type = T;
  using result_type = T;

  virtual ~IMetric() = default;
  
  virtual T distance(const T* a, const T* b, std::size_t size) const = 0;
  virtual T squared_distance(const T* a, const T* b, std::size_t size) const = 0;
  
  // Container interface
  template<typename Container>
  T distance(const Container& a, const Container& b) const
  {
    return distance(a.data(), b.data(), a.size());
  }
  
  template<typename Container>
  T squared_distance(const Container& a, const Container& b) const
  {
    return squared_distance(a.data(), b.data(), a.size());
  }
};

// Concrete wrapper for CRTP metrics
template<typename MetricType>
class MetricWrapper : public IMetric<typename MetricType::element_type>
{
public:
  using T = typename MetricType::element_type;
  
  explicit MetricWrapper(MetricType metric) : metric_(std::move(metric)) {}
  
  T distance(const T* a, const T* b, std::size_t size) const override
  {
    return metric_.distance(a, b, size);
  }
  
  T squared_distance(const T* a, const T* b, std::size_t size) const override
  {
    return metric_.squared_distance(a, b, size);
  }

private:
  MetricType metric_;
};

// Metric factory for runtime metric creation
template<typename T>
class MetricFactory
{
public:
  using metric_ptr = std::unique_ptr<IMetric<T>>;
  using creator_func = std::function<metric_ptr()>;

  static MetricFactory& instance()
  {
    static MetricFactory factory;
    return factory;
  }

  // Register a metric type
  template<typename MetricType>
  void register_metric(const std::string& name)
  {
    creators_[name] = []() -> metric_ptr {
      return std::make_unique<MetricWrapper<MetricType>>(MetricType{});
    };
  }

  // Register a metric with parameters
  void register_metric(const std::string& name, creator_func creator)
  {
    creators_[name] = std::move(creator);
  }

  // Create a metric by name
  metric_ptr create(const std::string& name) const
  {
    auto it = creators_.find(name);
    if (it != creators_.end()) {
      return it->second();
    }
    throw std::invalid_argument("Unknown metric: " + name);
  }

  // Get list of registered metrics
  std::vector<std::string> available_metrics() const
  {
    std::vector<std::string> names;
    names.reserve(creators_.size());
    for (const auto& [name, _] : creators_) {
      names.push_back(name);
    }
    return names;
  }

  // Clear all registrations
  void clear()
  {
    creators_.clear();
  }

private:
  MetricFactory()
  {
    // Register default metrics
    register_default_metrics();
  }

  void register_default_metrics()
  {
    // Vector metrics
    register_metric<L1Metric<T>>("l1");
    register_metric<L1Metric<T>>("manhattan");
    register_metric<L2Metric<T>>("l2");
    register_metric<L2Metric<T>>("euclidean");
    register_metric<LinfMetric<T>>("linf");
    register_metric<LinfMetric<T>>("chebyshev");
    
    // Histogram metrics
    register_metric<ChiSquaredMetric<T>>("chi_squared");
    register_metric<HistogramIntersectionMetric<T>>("histogram_intersection");
    register_metric<BhattacharyyaMetric<T>>("bhattacharyya");
    register_metric<HellingerMetric<T>>("hellinger");
    register_metric<EMDMetric<T>>("emd");
    register_metric<EMDMetric<T>>("wasserstein");
    register_metric<KLDivergenceMetric<T>>("kl_divergence");
    register_metric<JensenShannonMetric<T>>("jensen_shannon");
    
    // Angular metrics
    register_metric<CosineMetric<T>>("cosine");
    register_metric<AngularMetric<T>>("angular");
    register_metric<NormalizedAngularMetric<T>>("normalized_angular");
    register_metric<CorrelationMetric<T>>("correlation");
    register_metric<InnerProductMetric<T>>("inner_product");
    
    // Lp metrics with specific p values
    register_metric("l3", []() -> metric_ptr {
      return std::make_unique<MetricWrapper<LpMetric<T, 3>>>(LpMetric<T, 3>{});
    });
    
    register_metric("l4", []() -> metric_ptr {
      return std::make_unique<MetricWrapper<LpMetric<T, 4>>>(LpMetric<T, 4>{});
    });
  }

  std::unordered_map<std::string, creator_func> creators_;
};

// Helper function to create metrics
template<typename T>
std::unique_ptr<IMetric<T>> create_metric(const std::string& name)
{
  return MetricFactory<T>::instance().create(name);
}

// Metric registry for plugin architecture
template<typename T>
class MetricRegistry
{
public:
  struct MetricInfo
  {
    std::string name;
    std::string description;
    bool is_symmetric;
    bool has_squared_form;
    bool requires_positive_values;
  };

  static MetricRegistry& instance()
  {
    static MetricRegistry registry;
    return registry;
  }

  template<typename MetricType>
  void register_metric(const std::string& name, const std::string& description)
  {
    MetricInfo info;
    info.name = name;
    info.description = description;
    info.is_symmetric = metric_traits<MetricType>::is_symmetric;
    info.has_squared_form = metric_traits<MetricType>::has_squared_form;
    info.requires_positive_values = metric_traits<MetricType>::requires_positive_values;
    
    metrics_[name] = info;
    
    // Also register in factory
    MetricFactory<T>::instance().template register_metric<MetricType>(name);
  }

  const MetricInfo* get_info(const std::string& name) const
  {
    auto it = metrics_.find(name);
    return it != metrics_.end() ? &it->second : nullptr;
  }

  std::vector<MetricInfo> list_metrics() const
  {
    std::vector<MetricInfo> result;
    result.reserve(metrics_.size());
    for (const auto& [_, info] : metrics_) {
      result.push_back(info);
    }
    return result;
  }

private:
  std::unordered_map<std::string, MetricInfo> metrics_;
};

// Convenience function for creating metrics with automatic type deduction
template<typename Container>
auto create_metric_for(const Container& example, const std::string& name)
{
  using T = typename Container::value_type;
  return create_metric<T>(name);
}

}  // namespace toolbox::metrics