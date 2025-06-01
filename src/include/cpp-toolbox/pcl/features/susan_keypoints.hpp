#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/metrics/vector_metrics.hpp>

namespace toolbox::pcl
{

/**
 * @brief SUSAN (Smallest Univalue Segment Assimilating Nucleus) 3D关键点提取器 / SUSAN (Smallest Univalue Segment Assimilating Nucleus) 3D keypoint extractor
 * 
 * @tparam DataType 数据类型（float或double） / Data type (float or double)
 * @tparam KNN 最近邻搜索算法类型，默认使用 kdtree_generic_t / K-nearest neighbor search algorithm type, defaults to kdtree_generic_t
 * 
 * @details SUSAN算法通过计算局部区域的相似性来检测角点和边缘，对噪声具有良好的鲁棒性 / SUSAN algorithm detects corners and edges by computing local area similarity, with good robustness to noise
 */
template<typename DataType, 
         typename KNN = kdtree_generic_t<point_t<DataType>, toolbox::metrics::L2Metric<DataType>>>
class CPP_TOOLBOX_EXPORT susan_keypoint_extractor_t
    : public base_keypoint_extractor_t<susan_keypoint_extractor_t<DataType, KNN>,
                                       DataType,
                                       KNN>
{
public:
  using base_type = base_keypoint_extractor_t<susan_keypoint_extractor_t<DataType, KNN>,
                                              DataType,
                                              KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using indices_vector = typename base_type::indices_vector;

  susan_keypoint_extractor_t() = default;

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

  // SUSAN-specific parameters
  void set_geometric_threshold(data_type threshold) { m_geometric_threshold = threshold; }
  void set_angular_threshold(data_type threshold) { m_angular_threshold = threshold; }
  void set_susan_threshold(data_type threshold) { m_susan_threshold = threshold; }
  void set_non_maxima_radius(data_type radius) { m_non_maxima_radius = radius; }
  void set_use_normal_similarity(bool use) { m_use_normal_similarity = use; }

  [[nodiscard]] data_type get_geometric_threshold() const { return m_geometric_threshold; }
  [[nodiscard]] data_type get_angular_threshold() const { return m_angular_threshold; }
  [[nodiscard]] data_type get_susan_threshold() const { return m_susan_threshold; }
  [[nodiscard]] data_type get_non_maxima_radius() const { return m_non_maxima_radius; }
  [[nodiscard]] bool get_use_normal_similarity() const { return m_use_normal_similarity; }

private:
  struct SUSANInfo
  {
    data_type susan_value;  // USAN area
    bool is_valid;
  };

  struct NormalInfo
  {
    data_type nx, ny, nz;
    bool is_valid;
  };

  // Core computation methods
  SUSANInfo compute_susan_response(std::size_t point_idx, const std::vector<NormalInfo>& normals);
  std::vector<SUSANInfo> compute_all_susan_responses(const std::vector<NormalInfo>& normals);
  std::vector<NormalInfo> compute_normals();
  indices_vector apply_non_maxima_suppression(const std::vector<SUSANInfo>& susan_responses);
  
  void compute_susan_range(std::vector<SUSANInfo>& susan_responses,
                          const std::vector<NormalInfo>& normals,
                          std::size_t start_idx, 
                          std::size_t end_idx);

  // Member variables
  bool m_enable_parallel = false;
  data_type m_search_radius = static_cast<data_type>(1.0);
  data_type m_geometric_threshold = static_cast<data_type>(0.1);   // For geometric distance
  data_type m_angular_threshold = static_cast<data_type>(0.984);   // cos(10 degrees)
  data_type m_susan_threshold = static_cast<data_type>(0.5);       // SUSAN area threshold
  data_type m_non_maxima_radius = static_cast<data_type>(0.5);
  bool m_use_normal_similarity = true;  // Use normal-based similarity by default
  
  point_cloud_ptr m_cloud;
  knn_type* m_knn = nullptr;

  // Parallel processing threshold
  static constexpr std::size_t k_parallel_threshold = 1000;
};  // class susan_keypoint_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/susan_keypoints_impl.hpp>