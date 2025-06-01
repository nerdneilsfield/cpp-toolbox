#pragma once

#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <cpp-toolbox/metrics/metric_factory.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>
#include <nanoflann.hpp>

namespace toolbox::pcl
{

// Generic KD-tree implementation
template<typename Element, typename Metric = toolbox::metrics::L2Metric<typename Element::value_type>>
class CPP_TOOLBOX_EXPORT kdtree_generic_t : public base_knn_generic_t<kdtree_generic_t<Element, Metric>, Element, Metric>
{
public:
  using base_type = base_knn_generic_t<kdtree_generic_t<Element, Metric>, Element, Metric>;
  using traits_type = typename base_type::traits_type;
  using element_type = typename traits_type::element_type;
  using metric_type = typename traits_type::metric_type;
  using distance_type = typename traits_type::distance_type;
  using container_type = typename base_type::container_type;
  using container_ptr = typename base_type::container_ptr;
  using value_type = typename Element::value_type;

  kdtree_generic_t() = default;
  ~kdtree_generic_t() = default;

  kdtree_generic_t(const kdtree_generic_t&) = delete;
  kdtree_generic_t& operator=(const kdtree_generic_t&) = delete;
  kdtree_generic_t(kdtree_generic_t&&) = delete;
  kdtree_generic_t& operator=(kdtree_generic_t&&) = delete;

  // Set input data implementations
  std::size_t set_input_impl(const container_type& data);
  std::size_t set_input_impl(const container_ptr& data);
  
  // Set metric implementations
  void set_metric_impl(const metric_type& metric);
  void set_metric_impl(std::shared_ptr<toolbox::metrics::IMetric<value_type>> metric);

  // KNN search implementations
  bool kneighbors_impl(const element_type& query,
                       std::size_t num_neighbors,
                       std::vector<std::size_t>& indices,
                       std::vector<distance_type>& distances);

  bool radius_neighbors_impl(const element_type& query,
                             distance_type radius,
                             std::vector<std::size_t>& indices,
                             std::vector<distance_type>& distances);

  void set_max_leaf_size(std::size_t max_leaf_size) { m_max_leaf_size = max_leaf_size; }
  [[nodiscard]] std::size_t get_max_leaf_size() const noexcept { return m_max_leaf_size; }

private:
  // Dataset adaptor for nanoflann - generic version
  struct data_adaptor_t
  {
    const container_ptr& data;
    static constexpr std::size_t dims = 3; // Assuming 3D points for now

    data_adaptor_t(const container_ptr& data_) : data(data_) {}

    inline std::size_t kdtree_get_point_count() const { return data->size(); }

    inline value_type kdtree_get_pt(const std::size_t idx, const std::size_t dim) const
    {
      const auto& element = (*data)[idx];
      if (dim == 0) return element.x;
      else if (dim == 1) return element.y;
      else return element.z;
    }

    template<class BBOX>
    bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
  };

  // Metric adaptor to bridge between our metrics and nanoflann
  template<typename NanoflannMetric>
  struct metric_adaptor_t
  {
    metric_type m_metric;
    
    metric_adaptor_t(const metric_type& metric) : m_metric(metric) {}
    
    template<typename U, typename V>
    inline distance_type operator()(const U* a, const V& b, std::size_t size) const
    {
      // For nanoflann compatibility, we need to convert array to element
      element_type elem_a;
      elem_a.x = a[0];
      elem_a.y = a[1];
      elem_a.z = a[2];
      
      return m_metric(elem_a, b);
    }
  };

  // KD-tree type definitions using nanoflann's L2_Simple
  using kd_tree_t = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<value_type, data_adaptor_t>,
      data_adaptor_t,
      3,  // dimensions
      std::size_t
  >;

  void build_tree();
  bool validate_metric() const;

  container_ptr m_data;
  std::unique_ptr<data_adaptor_t> m_adaptor;
  std::unique_ptr<kd_tree_t> m_kdtree;
  metric_type m_compile_time_metric;
  std::shared_ptr<toolbox::metrics::IMetric<value_type>> m_runtime_metric;
  bool m_use_runtime_metric = false;
  std::size_t m_max_leaf_size = 10;
};

// Type aliases for common use cases
template<typename DataType>
using kdtree_t = kdtree_generic_t<point_t<DataType>, toolbox::metrics::L2Metric<DataType>>;

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/knn/impl/kdtree_impl.hpp>