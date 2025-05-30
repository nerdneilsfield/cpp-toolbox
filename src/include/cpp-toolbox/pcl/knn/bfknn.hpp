#pragma once

#include <cpp-toolbox/pcl/knn/base_knn.hpp>

namespace toolbox::pcl
{

template<typename DataType>
class CPP_TOOLBOX_EXPORT bfknn_t : public base_knn_t<bfknn_t<DataType>, DataType>
{
public:
  using data_type = DataType;
  using base_type = base_knn_t<bfknn_t<DataType>, DataType>;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;

  bfknn_t() = default;
  ~bfknn_t() = default;

  bfknn_t(const bfknn_t&) = delete;
  bfknn_t& operator=(const bfknn_t&) = delete;
  bfknn_t(bfknn_t&&) = delete;
  bfknn_t& operator=(bfknn_t&&) = delete;

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

private:
  data_type compute_distance(const point_t<data_type>& p1,
                             const point_t<data_type>& p2,
                             metric_type_t metric) const;

  point_cloud_ptr m_cloud;
  metric_type_t m_metric = metric_type_t::euclidean;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/knn/impl/bfknn_impl.hpp>