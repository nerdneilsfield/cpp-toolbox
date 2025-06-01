#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>

namespace toolbox::pcl
{

/**
 * @brief SIFT 3D (Scale-Invariant Feature Transform) 关键点提取器 / SIFT 3D (Scale-Invariant Feature Transform) keypoint extractor
 * 
 * @tparam DataType 数据类型（float或double） / Data type (float or double)
 * @tparam KNN 最近邻搜索算法类型，默认使用 kdtree_generic_t / K-nearest neighbor search algorithm type, defaults to kdtree_generic_t
 * 
 * @details SIFT 3D是经典SIFT算法在3D点云中的扩展，通过多尺度空间分析检测尺度不变的关键点 / SIFT 3D extends the classic SIFT algorithm to 3D point clouds, detecting scale-invariant keypoints through multi-scale space analysis
 */
template<typename DataType, 
         typename KNN = kdtree_generic_t<point_t<DataType>, toolbox::metrics::L2Metric<DataType>>>
class CPP_TOOLBOX_EXPORT sift3d_keypoint_extractor_t
    : public base_keypoint_extractor_t<sift3d_keypoint_extractor_t<DataType, KNN>,
                                       DataType,
                                       KNN>
{
public:
  using base_type = base_keypoint_extractor_t<sift3d_keypoint_extractor_t<DataType, KNN>,
                                              DataType,
                                              KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using indices_vector = typename base_type::indices_vector;

  sift3d_keypoint_extractor_t() = default;

  // Implementation methods for CRTP
  std::size_t set_input_impl(const point_cloud& cloud);
  std::size_t set_input_impl(const point_cloud_ptr& cloud);
  std::size_t set_knn_impl(const knn_type& knn);
  std::size_t set_search_radius_impl(data_type radius);
  void enable_parallel_impl(bool enable);

  indices_vector extract_impl();
  void extract_impl(indices_vector& keypoint_indices);
  point_cloud extract_keypoints_impl();
  void extract_keypoints_impl(point_cloud_ptr output);

  // SIFT3D-specific parameters
  void set_num_scales(std::size_t num_scales) { m_num_scales = num_scales; }
  void set_base_scale(data_type scale) { m_base_scale = scale; }
  void set_scale_factor(data_type factor) { m_scale_factor = factor; }
  void set_contrast_threshold(data_type threshold) { m_contrast_threshold = threshold; }
  void set_edge_threshold(data_type threshold) { m_edge_threshold = threshold; }
  void set_num_neighbors(std::size_t num_neighbors) { m_num_neighbors = num_neighbors; }

  [[nodiscard]] std::size_t get_num_scales() const { return m_num_scales; }
  [[nodiscard]] data_type get_base_scale() const { return m_base_scale; }
  [[nodiscard]] data_type get_scale_factor() const { return m_scale_factor; }
  [[nodiscard]] data_type get_contrast_threshold() const { return m_contrast_threshold; }
  [[nodiscard]] data_type get_edge_threshold() const { return m_edge_threshold; }
  [[nodiscard]] std::size_t get_num_neighbors() const { return m_num_neighbors; }

private:
  struct ScaleSpacePoint
  {
    std::size_t point_idx;
    std::size_t scale_idx;
    data_type response;
    bool is_extremum;
  };

  // Core computation methods
  std::vector<std::vector<data_type>> build_scale_space();
  std::vector<ScaleSpacePoint> find_scale_space_extrema(const std::vector<std::vector<data_type>>& scale_space);
  indices_vector refine_keypoints(const std::vector<ScaleSpacePoint>& extrema);
  indices_vector remove_edge_responses(const indices_vector& keypoint_indices);
  void compute_scale_space_range(std::vector<std::vector<data_type>>& scale_space,
                                std::size_t start_idx,
                                std::size_t end_idx);

  // Member variables
  bool m_enable_parallel = false;
  std::size_t m_num_scales = 5;
  data_type m_base_scale = static_cast<data_type>(0.05);
  data_type m_scale_factor = static_cast<data_type>(1.414); // √2
  data_type m_contrast_threshold = static_cast<data_type>(0.03);
  data_type m_edge_threshold = static_cast<data_type>(10.0);
  std::size_t m_num_neighbors = 20;
  
  point_cloud_ptr m_cloud;
  knn_type* m_knn = nullptr;

  // Parallel processing threshold
  static constexpr std::size_t k_parallel_threshold = 1000;
};  // class sift3d_keypoint_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/sift3d_keypoints_impl.hpp>