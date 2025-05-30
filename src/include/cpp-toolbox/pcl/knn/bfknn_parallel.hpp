#pragma once

#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>

namespace toolbox::pcl
{

template<typename DataType>
class CPP_TOOLBOX_EXPORT bfknn_parallel_t : public base_knn_t<bfknn_parallel_t<DataType>, DataType>
{
public:
  using data_type = DataType;
  using base_type = base_knn_t<bfknn_parallel_t<DataType>, DataType>;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;

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

  void enable_parallel(bool enable) { m_parallel_enabled = enable; }
  [[nodiscard]] bool is_parallel_enabled() const noexcept { return m_parallel_enabled; }

private:
  data_type compute_distance(const point_t<data_type>& p1,
                             const point_t<data_type>& p2,
                             metric_type_t metric) const;

  point_cloud_ptr m_cloud;
  metric_type_t m_metric = metric_type_t::euclidean;
  bool m_parallel_enabled = true;
  static constexpr std::size_t kParallelThreshold = 1024;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/knn/impl/bfknn_parallel_impl.hpp>