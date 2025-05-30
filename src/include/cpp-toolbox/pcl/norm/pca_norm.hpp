#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/norm/base_norm.hpp>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT pca_norm_extractor_t
    : public base_norm_extractor_t<pca_norm_extractor_t<DataType, KNN>,
                                   DataType,
                                   KNN>
{
public:
  using base_type =
      base_norm_extractor_t<pca_norm_extractor_t<DataType, KNN>, DataType, KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;

  std::size_t set_input_impl(const point_cloud& cloud);

  std::size_t set_input_impl(const point_cloud_ptr& cloud);

  std::size_t set_knn_impl(const knn_type& knn);

  std::size_t set_num_neighbors_impl(std::size_t num_neighbors);

  point_cloud extract_impl();

  void extract_impl(point_cloud_ptr output);

  void enable_parallel(bool enable) { m_enable_parallel = enable; }

private:
  void compute_normals_range(point_cloud_ptr output, std::size_t start_idx, std::size_t end_idx);
  point_t<data_type> compute_pca_normal(const std::vector<std::size_t>& indices);

  bool m_enable_parallel = false;
  std::size_t m_num_neighbors = 0;
  point_cloud_ptr m_cloud;
  knn_type* m_knn = nullptr;
};  // class pca_norm_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/norm/impl/pca_norm_impl.hpp>