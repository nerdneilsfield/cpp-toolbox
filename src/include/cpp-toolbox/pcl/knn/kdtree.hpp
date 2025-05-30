#pragma once

#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <nanoflann.hpp>

namespace toolbox::pcl
{

template<typename DataType>
class CPP_TOOLBOX_EXPORT kdtree_t : public base_knn_t<kdtree_t<DataType>, DataType>
{
public:
  using data_type = DataType;
  using base_type = base_knn_t<kdtree_t<DataType>, DataType>;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;

  kdtree_t() = default;
  ~kdtree_t() = default;

  kdtree_t(const kdtree_t&) = delete;
  kdtree_t& operator=(const kdtree_t&) = delete;
  kdtree_t(kdtree_t&&) = delete;
  kdtree_t& operator=(kdtree_t&&) = delete;

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

  void set_max_leaf_size(std::size_t max_leaf_size) { m_max_leaf_size = max_leaf_size; }
  [[nodiscard]] std::size_t get_max_leaf_size() const noexcept { return m_max_leaf_size; }

private:
  // Dataset adaptor for nanoflann
  struct point_cloud_adaptor_t
  {
    const point_cloud_ptr& cloud;

    point_cloud_adaptor_t(const point_cloud_ptr& cloud_) : cloud(cloud_) {}

    inline std::size_t kdtree_get_point_count() const { return cloud->points.size(); }

    inline data_type kdtree_get_pt(const std::size_t idx, const std::size_t dim) const
    {
      if (dim == 0) return cloud->points[idx].x;
      else if (dim == 1) return cloud->points[idx].y;
      else return cloud->points[idx].z;
    }

    template<class BBOX>
    bool kdtree_get_bbox(BBOX& /*bb*/) const { return false; }
  };

  // KD-tree type definitions
  using kd_tree_t = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<data_type, point_cloud_adaptor_t>,
      point_cloud_adaptor_t,
      3,  // dimensions
      std::size_t
  >;

  void build_tree();

  point_cloud_ptr m_cloud;
  std::unique_ptr<point_cloud_adaptor_t> m_adaptor;
  std::unique_ptr<kd_tree_t> m_kdtree;
  metric_type_t m_metric = metric_type_t::euclidean;
  std::size_t m_max_leaf_size = 10;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/knn/impl/kdtree_impl.hpp>