#pragma once

#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <cpp-toolbox/metrics/metric_factory.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>

namespace toolbox::pcl
{

// Generic parallel brute-force KNN implementation
template<typename Element, typename Metric = toolbox::metrics::L2Metric<typename Element::value_type>>
class CPP_TOOLBOX_EXPORT bfknn_parallel_generic_t : public base_knn_generic_t<bfknn_parallel_generic_t<Element, Metric>, Element, Metric>
{
public:
  using base_type = base_knn_generic_t<bfknn_parallel_generic_t<Element, Metric>, Element, Metric>;
  using traits_type = typename base_type::traits_type;
  using element_type = typename traits_type::element_type;
  using metric_type = typename traits_type::metric_type;
  using distance_type = typename traits_type::distance_type;
  using container_type = typename base_type::container_type;
  using container_ptr = typename base_type::container_ptr;

  bfknn_parallel_generic_t() = default;
  ~bfknn_parallel_generic_t() = default;

  bfknn_parallel_generic_t(const bfknn_parallel_generic_t&) = delete;
  bfknn_parallel_generic_t& operator=(const bfknn_parallel_generic_t&) = delete;
  bfknn_parallel_generic_t(bfknn_parallel_generic_t&&) = delete;
  bfknn_parallel_generic_t& operator=(bfknn_parallel_generic_t&&) = delete;

  // Set input data implementations
  std::size_t set_input_impl(const container_type& data);
  std::size_t set_input_impl(const container_ptr& data);
  
  // Set metric implementations
  void set_metric_impl(const metric_type& metric);
  void set_metric_impl(std::shared_ptr<toolbox::metrics::IMetric<typename Element::value_type>> metric);

  // KNN search implementations
  bool kneighbors_impl(const element_type& query,
                       std::size_t num_neighbors,
                       std::vector<std::size_t>& indices,
                       std::vector<distance_type>& distances);

  bool radius_neighbors_impl(const element_type& query,
                             distance_type radius,
                             std::vector<std::size_t>& indices,
                             std::vector<distance_type>& distances);

  void enable_parallel(bool enable) { m_parallel_enabled = enable; }
  [[nodiscard]] bool is_parallel_enabled() const noexcept { return m_parallel_enabled; }

private:
  container_ptr m_data;
  metric_type m_compile_time_metric;
  std::shared_ptr<toolbox::metrics::IMetric<typename Element::value_type>> m_runtime_metric;
  bool m_use_runtime_metric = false;
  bool m_parallel_enabled = true;
  static constexpr std::size_t k_parallel_threshold = 1024;
};

// Legacy parallel brute-force KNN for backward compatibility
template<typename DataType>
class CPP_TOOLBOX_EXPORT bfknn_parallel_t : public base_knn_t<bfknn_parallel_t<DataType>, DataType>
{
public:
  using data_type = DataType;
  using base_type = base_knn_t<bfknn_parallel_t<DataType>, DataType>;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;

  // Internal generic implementation
  using generic_impl_type = bfknn_parallel_generic_t<point_t<DataType>, toolbox::metrics::L2Metric<DataType>>;

  bfknn_parallel_t() = default;
  ~bfknn_parallel_t() = default;

  bfknn_parallel_t(const bfknn_parallel_t&) = delete;
  bfknn_parallel_t& operator=(const bfknn_parallel_t&) = delete;
  bfknn_parallel_t(bfknn_parallel_t&&) = delete;
  bfknn_parallel_t& operator=(bfknn_parallel_t&&) = delete;

  std::size_t set_input_impl(const point_cloud& cloud);
  std::size_t set_input_impl(const point_cloud_ptr& cloud);
  std::size_t set_metric_impl(metric_type_t metric);

  bool kneighbors_impl(const point_t<data_type>& query_point,
                       std::size_t num_neighbors,
                       std::vector<std::size_t>& indices,
                       std::vector<data_type>& distances);

  bool radius_neighbors_impl(const point_t<data_type>& query_point,
                             data_type radius,
                             std::vector<std::size_t>& indices,
                             std::vector<data_type>& distances);

  void enable_parallel(bool enable) { m_impl.enable_parallel(enable); }
  [[nodiscard]] bool is_parallel_enabled() const noexcept { return m_impl.is_parallel_enabled(); }

private:
  generic_impl_type m_impl;
  metric_type_t m_metric = metric_type_t::euclidean;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/knn/impl/bfknn_parallel_impl.hpp>