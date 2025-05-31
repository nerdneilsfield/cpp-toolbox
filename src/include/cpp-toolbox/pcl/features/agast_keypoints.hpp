#pragma once

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/features/base_feature_extractor.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/pcl/knn/bfknn_parallel.hpp>
#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

/**
 * @brief AGAST (Adaptive and Generic Accelerated Segment Test) 3D关键点提取器 / AGAST (Adaptive and Generic Accelerated Segment Test) 3D keypoint extractor
 * 
 * @tparam DataType 数据类型（float或double） / Data type (float or double)
 * @tparam KNN 最近邻搜索算法类型 / K-nearest neighbor search algorithm type
 * 
 * @details AGAST是FAST算法的改进版本，在3D点云中通过自适应决策树快速检测角点 / AGAST is an improved version of FAST, detecting corners quickly in 3D point clouds through adaptive decision trees
 */
template<typename DataType, typename KNN>
class CPP_TOOLBOX_EXPORT agast_keypoint_extractor_t
    : public base_keypoint_extractor_t<agast_keypoint_extractor_t<DataType, KNN>,
                                       DataType,
                                       KNN>
{
public:
  using base_type = base_keypoint_extractor_t<agast_keypoint_extractor_t<DataType, KNN>,
                                              DataType,
                                              KNN>;
  using data_type = typename base_type::data_type;
  using knn_type = typename base_type::knn_type;
  using point_cloud = typename base_type::point_cloud;
  using point_cloud_ptr = typename base_type::point_cloud_ptr;
  using indices_vector = typename base_type::indices_vector;

  agast_keypoint_extractor_t() = default;

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

  // AGAST-specific parameters
  void set_threshold(data_type threshold) { m_threshold = threshold; }
  void set_pattern_radius(data_type radius) { m_pattern_radius = radius; }
  void set_non_maxima_radius(data_type radius) { m_non_maxima_radius = radius; }
  void set_num_test_points(std::size_t num) { m_num_test_points = num; initialize_test_pattern(); }
  void set_min_arc_length(std::size_t length) { m_min_arc_length = length; }

  [[nodiscard]] data_type get_threshold() const { return m_threshold; }
  [[nodiscard]] data_type get_pattern_radius() const { return m_pattern_radius; }
  [[nodiscard]] data_type get_non_maxima_radius() const { return m_non_maxima_radius; }
  [[nodiscard]] std::size_t get_num_test_points() const { return m_num_test_points; }
  [[nodiscard]] std::size_t get_min_arc_length() const { return m_min_arc_length; }

private:
  struct TestPoint
  {
    data_type x, y, z;  // Relative position on sphere
  };

  struct AGASTInfo
  {
    data_type score;
    bool is_keypoint;
  };

  // Core computation methods
  void initialize_test_pattern();
  AGASTInfo compute_agast_response(std::size_t point_idx);
  std::vector<AGASTInfo> compute_all_agast_responses();
  indices_vector apply_non_maxima_suppression(const std::vector<AGASTInfo>& agast_responses);
  
  void compute_agast_range(std::vector<AGASTInfo>& agast_responses,
                          std::size_t start_idx, 
                          std::size_t end_idx);
  
  data_type compute_test_value(const point_t<data_type>& center, const TestPoint& test_point);
  bool is_consecutive_arc(const std::vector<bool>& brighter, const std::vector<bool>& darker);

  // Member variables
  bool m_enable_parallel = false;
  data_type m_threshold = static_cast<data_type>(0.1);
  data_type m_pattern_radius = static_cast<data_type>(0.5);
  data_type m_non_maxima_radius = static_cast<data_type>(0.5);
  std::size_t m_num_test_points = 16;  // Number of test points on sphere
  std::size_t m_min_arc_length = 9;    // Minimum consecutive similar points
  
  std::vector<TestPoint> m_test_pattern;  // 3D test pattern on sphere
  
  point_cloud_ptr m_cloud;
  knn_type* m_knn = nullptr;

  // Parallel processing threshold
  static constexpr std::size_t k_parallel_threshold = 1000;
};  // class agast_keypoint_extractor_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/features/impl/agast_keypoints_impl.hpp>