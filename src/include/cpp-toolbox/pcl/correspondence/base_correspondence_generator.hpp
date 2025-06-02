#pragma once

#include <memory>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::pcl
{

/**
 * @brief 对应关系结构体 / Correspondence structure
 * 
 * 存储源点云和目标点云中匹配点对的信息
 * Stores information about matched point pairs between source and target clouds
 */
struct correspondence_t
{
  std::size_t src_idx;  ///< 源点索引 / Source point index
  std::size_t dst_idx;  ///< 目标点索引 / Destination point index
  float distance;       ///< 描述子间的距离 / Distance between descriptors
};

/**
 * @brief 对应点生成器的基类（CRTP模式） / Base class for correspondence generators (CRTP pattern)
 * 
 * 该类定义了所有对应点生成算法的通用接口，使用CRTP模式实现静态多态。
 * This class defines the common interface for all correspondence generation algorithms,
 * using CRTP pattern for static polymorphism.
 * 
 * @tparam Derived 派生类类型 / Derived class type
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * 
 * @code
 * // 派生类示例 / Derived class example
 * template<typename DataType, typename Signature>
 * class my_correspondence_generator_t 
 *     : public base_correspondence_generator_t<
 *         my_correspondence_generator_t<DataType, Signature>,
 *         DataType, Signature>
 * {
 * public:
 *     void compute_impl(std::vector<correspondence_t>& correspondences) {
 *         // 实现具体的对应点生成算法 / Implement specific correspondence generation algorithm
 *     }
 * };
 * @endcode
 */
template<typename Derived, typename DataType, typename Signature>
class CPP_TOOLBOX_EXPORT base_correspondence_generator_t
{
public:
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;
  using signatures_ptr = std::shared_ptr<std::vector<Signature>>;
  using keypoint_indices_ptr = std::shared_ptr<std::vector<std::size_t>>;

  base_correspondence_generator_t() = default;
  virtual ~base_correspondence_generator_t() = default;

  // 删除拷贝和移动操作 / Delete copy and move operations
  base_correspondence_generator_t(const base_correspondence_generator_t&) = delete;
  base_correspondence_generator_t& operator=(const base_correspondence_generator_t&) = delete;
  base_correspondence_generator_t(base_correspondence_generator_t&&) = default;
  base_correspondence_generator_t& operator=(base_correspondence_generator_t&&) = default;

  /**
   * @brief 设置源数据 / Set source data
   * @param src_cloud 源点云 / Source point cloud
   * @param src_descriptors 源描述子 / Source descriptors
   * @param src_keypoint_indices 源关键点索引 / Source keypoint indices
   */
  void set_source(const point_cloud_ptr& src_cloud,
                  const signatures_ptr& src_descriptors,
                  const keypoint_indices_ptr& src_keypoint_indices)
  {
    m_src_cloud = src_cloud;
    m_src_descriptors = src_descriptors;
    m_src_keypoint_indices = src_keypoint_indices;
  }

  /**
   * @brief 设置目标数据 / Set destination data
   * @param dst_cloud 目标点云 / Destination point cloud
   * @param dst_descriptors 目标描述子 / Destination descriptors
   * @param dst_keypoint_indices 目标关键点索引 / Destination keypoint indices
   */
  void set_destination(const point_cloud_ptr& dst_cloud,
                       const signatures_ptr& dst_descriptors,
                       const keypoint_indices_ptr& dst_keypoint_indices)
  {
    m_dst_cloud = dst_cloud;
    m_dst_descriptors = dst_descriptors;
    m_dst_keypoint_indices = dst_keypoint_indices;
  }

  /**
   * @brief 设置比率测试阈值 / Set ratio test threshold
   * @param ratio 比率阈值（通常0.7-0.8） / Ratio threshold (typically 0.7-0.8)
   */
  void set_ratio(float ratio) { m_ratio = ratio; }

  /**
   * @brief 获取比率测试阈值 / Get ratio test threshold
   * @return 比率阈值 / Ratio threshold
   */
  float get_ratio() const { return m_ratio; }

  /**
   * @brief 设置是否启用双向验证 / Set whether to enable mutual verification
   * @param mutual_verification true启用，false禁用 / true to enable, false to disable
   */
  void set_mutual_verification(bool mutual_verification)
  {
    m_mutual_verification = mutual_verification;
  }

  /**
   * @brief 获取是否启用双向验证 / Get whether mutual verification is enabled
   * @return 是否启用双向验证 / Whether mutual verification is enabled
   */
  bool get_mutual_verification() const { return m_mutual_verification; }

  /**
   * @brief 设置距离阈值 / Set distance threshold
   * @param distance_threshold 最大允许的描述子距离 / Maximum allowed descriptor distance
   */
  void set_distance_threshold(float distance_threshold)
  {
    m_distance_threshold = distance_threshold;
  }

  /**
   * @brief 获取距离阈值 / Get distance threshold
   * @return 距离阈值 / Distance threshold
   */
  float get_distance_threshold() const { return m_distance_threshold; }

  /**
   * @brief 计算对应关系 / Compute correspondences
   * @param correspondences [out] 输出的对应关系 / Output correspondences
   */
  void compute(std::vector<correspondence_t>& correspondences)
  {
    // 清空输出和统计信息 / Clear output and statistics
    correspondences.clear();
    reset_statistics();

    // 验证输入 / Validate input
    if (!validate_input()) {
      return;
    }

    // 调用派生类的实现 / Call derived class implementation
    static_cast<Derived*>(this)->compute_impl(correspondences);
  }

  /**
   * @brief 获取最后一次计算的统计信息 / Get statistics from last computation
   * @return 统计信息字符串 / Statistics string
   */
  std::string get_statistics() const
  {
    std::stringstream ss;
    ss << "对应点生成统计 / Correspondence Generation Statistics:\n";
    ss << "  候选对应点总数 / Total candidates: " << m_total_candidates << "\n";
    ss << "  通过比率测试 / Passed ratio test: " << m_ratio_test_passed << "\n";
    if (m_mutual_verification) {
      ss << "  通过双向验证 / Passed mutual verification: " << m_mutual_test_passed << "\n";
    }
    ss << "  通过距离阈值 / Passed distance threshold: " << m_distance_test_passed << "\n";
    
    if (m_src_descriptors) {
      float match_rate = static_cast<float>(m_distance_test_passed) / m_src_descriptors->size();
      ss << "  匹配率 / Match rate: " << (match_rate * 100) << "%\n";
    }
    
    return ss.str();
  }

protected:
  /**
   * @brief 验证输入数据的有效性 / Validate input data
   * @return true表示有效，false表示无效 / true if valid, false if invalid
   */
  bool validate_input() const
  {
    // 检查源数据 / Check source data
    if (!m_src_cloud || !m_src_descriptors || !m_src_keypoint_indices) {
      std::cerr << "错误：源数据不完整 / Error: Source data incomplete" << std::endl;
      return false;
    }

    // 检查目标数据 / Check target data
    if (!m_dst_cloud || !m_dst_descriptors || !m_dst_keypoint_indices) {
      std::cerr << "错误：目标数据不完整 / Error: Target data incomplete" << std::endl;
      return false;
    }

    // 检查描述子和关键点数量是否匹配 / Check if descriptors and keypoints match
    if (m_src_descriptors->size() != m_src_keypoint_indices->size()) {
      std::cerr << "错误：源描述子数量与关键点数量不匹配 / Error: Source descriptor count doesn't match keypoint count" << std::endl;
      return false;
    }

    if (m_dst_descriptors->size() != m_dst_keypoint_indices->size()) {
      std::cerr << "错误：目标描述子数量与关键点数量不匹配 / Error: Target descriptor count doesn't match keypoint count" << std::endl;
      return false;
    }

    // 检查参数有效性 / Check parameter validity
    if (m_ratio <= 0.0f || m_ratio >= 1.0f) {
      std::cerr << "警告：比率测试阈值应在(0,1)之间 / Warning: Ratio test threshold should be in (0,1)" << std::endl;
    }

    return true;
  }

  /**
   * @brief 应用比率测试 / Apply ratio test
   * @param candidates 候选对应关系 / Candidate correspondences
   * @return 是否通过测试 / Whether the test is passed
   */
  bool apply_ratio_test(const std::vector<correspondence_t>& candidates) const
  {
    // Lowe's ratio test: 检查最佳匹配是否显著优于次佳匹配 / Check if best match is significantly better than second best
    if (candidates.size() < 2) {
      return true;  // 如果只有一个匹配，默认通过 / If only one match, pass by default
    }
    
    return (candidates[0].distance < m_ratio * candidates[1].distance);
  }

  /**
   * @brief 应用距离阈值过滤 / Apply distance threshold filtering
   * @param correspondences [in/out] 对应关系 / Correspondences
   */
  void apply_distance_threshold(std::vector<correspondence_t>& correspondences) const
  {
    // 移除距离超过阈值的对应关系 / Remove correspondences exceeding distance threshold
    correspondences.erase(
        std::remove_if(correspondences.begin(), correspondences.end(),
                       [this](const correspondence_t& corr) {
                         return corr.distance > m_distance_threshold;
                       }),
        correspondences.end());
  }

  /**
   * @brief 计算几何一致性分数（简单验证） / Compute geometric consistency score (simple verification)
   * @param correspondences 对应关系 / Correspondences
   * @return 一致性分数 / Consistency score
   */
  float compute_geometric_consistency(const std::vector<correspondence_t>& correspondences) const
  {
    if (correspondences.size() < 3) {
      return 0.0f;
    }
    
    // 简单的几何一致性检查：比较点对之间的距离比率 / Simple geometric consistency: compare distance ratios between point pairs
    float consistency_score = 0.0f;
    std::size_t valid_pairs = 0;
    
    for (std::size_t i = 0; i < correspondences.size() - 1; ++i) {
      for (std::size_t j = i + 1; j < correspondences.size(); ++j) {
        // 计算源点对之间的距离 / Compute distance between source point pair
        const auto& src_p1 = m_src_cloud->points[correspondences[i].src_idx];
        const auto& src_p2 = m_src_cloud->points[correspondences[j].src_idx];
        DataType src_dist = std::sqrt(
            (src_p1.x - src_p2.x) * (src_p1.x - src_p2.x) +
            (src_p1.y - src_p2.y) * (src_p1.y - src_p2.y) +
            (src_p1.z - src_p2.z) * (src_p1.z - src_p2.z));
        
        // 计算目标点对之间的距离 / Compute distance between target point pair
        const auto& dst_p1 = m_dst_cloud->points[correspondences[i].dst_idx];
        const auto& dst_p2 = m_dst_cloud->points[correspondences[j].dst_idx];
        DataType dst_dist = std::sqrt(
            (dst_p1.x - dst_p2.x) * (dst_p1.x - dst_p2.x) +
            (dst_p1.y - dst_p2.y) * (dst_p1.y - dst_p2.y) +
            (dst_p1.z - dst_p2.z) * (dst_p1.z - dst_p2.z));
        
        // 避免除零 / Avoid division by zero
        if (src_dist > 0.001 && dst_dist > 0.001) {
          DataType ratio = src_dist / dst_dist;
          // 如果比率接近1，说明几何关系保持良好 / If ratio is close to 1, geometry is well preserved
          if (ratio > 0.8 && ratio < 1.2) {
            consistency_score += 1.0f;
          }
          valid_pairs++;
        }
      }
    }
    
    return valid_pairs > 0 ? consistency_score / valid_pairs : 0.0f;
  }

  /**
   * @brief 重置统计信息 / Reset statistics
   */
  void reset_statistics()
  {
    m_total_candidates = 0;
    m_ratio_test_passed = 0;
    m_mutual_test_passed = 0;
    m_distance_test_passed = 0;
  }

  // 数据成员 / Data members
  point_cloud_ptr m_src_cloud;
  point_cloud_ptr m_dst_cloud;
  signatures_ptr m_src_descriptors;
  signatures_ptr m_dst_descriptors;
  keypoint_indices_ptr m_src_keypoint_indices;
  keypoint_indices_ptr m_dst_keypoint_indices;

  float m_ratio = 0.8F;  ///< 比率测试阈值 / Ratio test threshold
  float m_distance_threshold = std::numeric_limits<float>::max();  ///< 距离阈值 / Distance threshold
  bool m_mutual_verification = true;  ///< 是否启用双向验证 / Whether to enable mutual verification

  // 统计信息 / Statistics
  mutable std::size_t m_total_candidates = 0;
  mutable std::size_t m_ratio_test_passed = 0;
  mutable std::size_t m_mutual_test_passed = 0;
  mutable std::size_t m_distance_test_passed = 0;
};

}  // namespace toolbox::pcl