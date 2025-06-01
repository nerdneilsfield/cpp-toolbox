#pragma once

#include <cpp-toolbox/metrics/metric_factory.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::metrics
{

// Adapter to use IMetric with point types
template<typename T>
class PointMetricAdapter : public IMetric<T>
{
public:
  explicit PointMetricAdapter(std::shared_ptr<IMetric<T>> metric) 
    : m_metric(metric) {}

  T distance(const T* a, const T* b, std::size_t size) const override
  {
    return m_metric->distance(a, b, size);
  }
  
  T squared_distance(const T* a, const T* b, std::size_t size) const override
  {
    return m_metric->squared_distance(a, b, size);
  }
  
  // Adapter method for point types
  T operator()(const toolbox::types::point_t<T>& a, 
               const toolbox::types::point_t<T>& b) const
  {
    T arr_a[3] = {a.x, a.y, a.z};
    T arr_b[3] = {b.x, b.y, b.z};
    return distance(arr_a, arr_b, 3);
  }

private:
  std::shared_ptr<IMetric<T>> m_metric;
};

}  // namespace toolbox::metrics