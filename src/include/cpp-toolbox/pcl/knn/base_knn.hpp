#pragma once

#include <memory>
#include <type_traits>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>
#include <cpp-toolbox/metrics/metric_factory.hpp>

namespace toolbox::pcl
{

using toolbox::types::point_t;

// KNN traits for generic element and metric types
template<typename Element, typename Metric>
struct knn_traits
{
  using element_type = Element;
  using metric_type = Metric;
  using distance_type = typename Metric::result_type;
};

// Legacy metric type enum for backward compatibility
enum class CPP_TOOLBOX_EXPORT metric_type_t : std::uint8_t
{
  euclidean,
  manhattan,
  chebyshev,
  minkowski,
};

// Primary template: generic element and metric support
template<typename Derived, typename Element, typename Metric = toolbox::metrics::L2Metric<typename Element::value_type>>
class CPP_TOOLBOX_EXPORT base_knn_generic_t
{
public:
  using traits_type = knn_traits<Element, Metric>;
  using element_type = typename traits_type::element_type;
  using metric_type = typename traits_type::metric_type;
  using distance_type = typename traits_type::distance_type;
  using container_type = std::vector<element_type>;
  using container_ptr = std::shared_ptr<container_type>;

  // Set input data
  std::size_t set_input(const container_type& data)
  {
    return static_cast<Derived*>(this)->set_input_impl(data);
  }

  std::size_t set_input(const container_ptr& data)
  {
    return static_cast<Derived*>(this)->set_input_impl(data);
  }

  // Set metric (compile-time version)
  void set_metric(const metric_type& metric)
  {
    static_cast<Derived*>(this)->set_metric_impl(metric);
  }

  // Set metric (runtime version)
  template<typename T = typename Element::value_type>
  void set_metric(std::shared_ptr<toolbox::metrics::IMetric<T>> metric)
  {
    static_cast<Derived*>(this)->set_metric_impl(metric);
  }

  // K-nearest neighbors search
  bool kneighbors(const element_type& query,
                  std::size_t num_neighbors,
                  std::vector<std::size_t>& indices,
                  std::vector<distance_type>& distances)
  {
    return static_cast<Derived*>(this)->kneighbors_impl(
        query, num_neighbors, indices, distances);
  }

  // Radius neighbors search
  bool radius_neighbors(const element_type& query,
                        distance_type radius,
                        std::vector<std::size_t>& indices,
                        std::vector<distance_type>& distances)
  {
    return static_cast<Derived*>(this)->radius_neighbors_impl(
        query, radius, indices, distances);
  }

protected:
  base_knn_generic_t() = default;
  ~base_knn_generic_t() = default;

public:
  base_knn_generic_t(const base_knn_generic_t&) = delete;
  base_knn_generic_t& operator=(const base_knn_generic_t&) = delete;
  base_knn_generic_t(base_knn_generic_t&&) = delete;
  base_knn_generic_t& operator=(base_knn_generic_t&&) = delete;
};  // class base_knn_generic_t

// Legacy base_knn_t for backward compatibility
template<typename Derived, typename DataType>
class CPP_TOOLBOX_EXPORT base_knn_t : public base_knn_generic_t<Derived, point_t<DataType>, toolbox::metrics::L2Metric<DataType>>
{
public:
  using base_type = base_knn_generic_t<Derived, point_t<DataType>, toolbox::metrics::L2Metric<DataType>>;
  using data_type = DataType;
  using point_cloud = toolbox::types::point_cloud_t<data_type>;
  using point_cloud_ptr = std::shared_ptr<toolbox::types::point_cloud_t<data_type>>;

  // Legacy interface
  std::size_t set_input(const point_cloud& cloud)
  {
    // Convert point cloud to vector of points
    std::vector<point_t<DataType>> points(cloud.points.begin(), cloud.points.end());
    return base_type::set_input(points);
  }

  std::size_t set_input(const point_cloud_ptr& cloud)
  {
    if (!cloud) return 0;
    return set_input(*cloud);
  }

  std::size_t set_metric(metric_type_t metric)
  {
    return static_cast<Derived*>(this)->set_metric_impl(metric);
  }

  [[nodiscard]] metric_type_t get_metric() const noexcept { return m_metric; }

protected:
  base_knn_t() = default;
  ~base_knn_t() = default;

private:
  metric_type_t m_metric = metric_type_t::euclidean;
};  // class base_knn_t

}  // namespace toolbox::pcl