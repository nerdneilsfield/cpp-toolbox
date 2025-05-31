#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT loam_feature_extractor_t
    : public base_keypoint_extractor_t<loam_feature_extractor_t<DataType, KNN>,
                                       DataType,
                                       KNN>
{
public:
  using base_type = base_keypoint_extractor_t<loam_feature_extractor_t<DataType, KNN>,
                                              DataType,
                                              KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using indices_vector = typename base_type::indices_vector;

  // Feature type labels
  enum class feature_label : uint8_t {
    none = 0,    // Neither edge nor planar
    edge = 1,    // Edge/corner point
    planar = 2   // Planar point
  };
  
  // Result structure containing both point cloud and labels
  struct loam_result {
    point_cloud cloud;
    std::vector<uint8_t> labels;  // feature_label for each point
  };

  loam_feature_extractor_t() = default;

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
  
  // Extended interface for LOAM
  loam_result extract_labeled_cloud();

  // LOAM-specific parameters
  void set_edge_threshold(data_type threshold) { m_edge_threshold = threshold; }
  void set_planar_threshold(data_type threshold) { m_planar_threshold = threshold; }
  void set_curvature_threshold(data_type threshold) { m_curvature_threshold = threshold; }
  void set_num_scan_neighbors(std::size_t num) { m_num_scan_neighbors = num; }

  [[nodiscard]] data_type get_edge_threshold() const { return m_edge_threshold; }
  [[nodiscard]] data_type get_planar_threshold() const { return m_planar_threshold; }
  [[nodiscard]] data_type get_curvature_threshold() const { return m_curvature_threshold; }
  [[nodiscard]] std::size_t get_num_scan_neighbors() const { return m_num_scan_neighbors; }

  // Utility methods for extracting specific feature types
  static point_cloud extract_edge_points(const loam_result& result);
  static point_cloud extract_planar_points(const loam_result& result);
  static point_cloud extract_non_feature_points(const loam_result& result);
  
  // Get indices only
  static indices_vector extract_edge_indices(const std::vector<uint8_t>& labels);
  static indices_vector extract_planar_indices(const std::vector<uint8_t>& labels);

private:
  struct CurvatureInfo
  {
    data_type curvature;
    bool is_valid;
  };

  // Core computation methods
  std::vector<CurvatureInfo> compute_curvatures();
  CurvatureInfo compute_point_curvature(std::size_t point_idx);
  void compute_curvatures_range(std::vector<CurvatureInfo>& curvatures, 
                               std::size_t start_idx, 
                               std::size_t end_idx);
  std::vector<uint8_t> classify_features(const std::vector<CurvatureInfo>& curvatures);

  // Member variables
  bool m_enable_parallel = false;
  data_type m_edge_threshold = static_cast<data_type>(0.2);      // High curvature threshold
  data_type m_planar_threshold = static_cast<data_type>(0.1);    // Low curvature threshold  
  data_type m_curvature_threshold = static_cast<data_type>(0.001); // Minimum curvature
  std::size_t m_num_scan_neighbors = 10;  // Number of neighbors for curvature computation
  
  point_cloud_ptr m_cloud;
  knn_type* m_knn = nullptr;

  // Parallel processing threshold
  static constexpr std::size_t k_parallel_threshold = 1000;
};  // class loam_feature_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/loam_feature_extractor_impl.hpp>