#pragma once

#include <cstddef>
#include <memory>

#include <cpp-toolbox/types/point.hpp>

namespace toolbox::pcl
{

/**
 * @brief 描述子签名的基类 / Base class for descriptor signatures
 * 
 * 该类定义了所有描述子签名类型的通用接口，主要提供距离计算功能。
 * This class defines the common interface for all descriptor signature types,
 * mainly providing distance calculation functionality.
 * 
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 * @tparam Derived 派生的签名类型 / Derived signature type
 * 
 * @code
 * // 自定义描述子签名示例 / Custom descriptor signature example
 * struct my_signature_t : base_signature_t<float, my_signature_t> {
 *     std::array<float, 128> histogram;
 *     
 *     float distance_impl(const my_signature_t& other) const {
 *         float dist = 0;
 *         for (size_t i = 0; i < 128; ++i) {
 *             dist += std::abs(histogram[i] - other.histogram[i]);
 *         }
 *         return dist;
 *     }
 * };
 * @endcode
 */
template<typename DataType, class Derived>
struct base_signature_t
{
  using data_type = DataType;
  using derived_type = Derived;

  /**
   * @brief 计算与另一个签名的距离 / Calculate distance to another signature
   * @param other 另一个签名 / Another signature
   * @return 距离值 / Distance value
   */
  DataType distance(const Derived& other) const
  {
    return static_cast<const Derived*>(this)->distance_impl(other);
  }
};  // struct base_signature_t

/**
 * @brief 描述子提取器的基类（CRTP模式） / Base class for descriptor extractors (CRTP pattern)
 * 
 * 该类定义了所有描述子提取算法的通用接口。描述子是用于描述点云局部特征的向量，
 * 常用于物体识别、配准、分类等任务。
 * This class defines the common interface for all descriptor extraction algorithms.
 * Descriptors are vectors that describe local features of point clouds, commonly
 * used for object recognition, registration, classification, and other tasks.
 * 
 * @tparam Derived 派生的提取器类型 / Derived extractor type
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * 
 * @code
 * // 使用描述子提取器的示例 / Example of using descriptor extractor
 * fpfh_extractor_t<float> extractor;
 * extractor.set_search_radius(0.05f);
 * 
 * // 在关键点处计算描述子 / Compute descriptors at keypoints
 * std::vector<fpfh_signature_t<float>> descriptors;
 * extractor.compute(cloud, keypoint_indices, descriptors);
 * 
 * // 使用描述子进行匹配 / Use descriptors for matching
 * for (size_t i = 0; i < descriptors.size(); ++i) {
 *     float min_dist = std::numeric_limits<float>::max();
 *     size_t best_match = 0;
 *     for (size_t j = 0; j < target_descriptors.size(); ++j) {
 *         float dist = descriptors[i].distance(target_descriptors[j]);
 *         if (dist < min_dist) {
 *             min_dist = dist;
 *             best_match = j;
 *         }
 *     }
 * }
 * @endcode
 */
template<class Derived, typename DataType, typename Signature>
class base_descriptor_extractor_t
{
public:
  base_descriptor_extractor_t() = default;
  base_descriptor_extractor_t(const base_descriptor_extractor_t&) = delete;
  base_descriptor_extractor_t(base_descriptor_extractor_t&&) = default;
  base_descriptor_extractor_t& operator=(const base_descriptor_extractor_t&) =
      delete;
  base_descriptor_extractor_t& operator=(base_descriptor_extractor_t&&) =
      default;
  virtual ~base_descriptor_extractor_t() = default;

  /**
   * @brief 启用或禁用并行计算 / Enable or disable parallel computation
   * @param enable true启用并行，false禁用 / true to enable parallel, false to disable
   */
  void enable_parallel(bool enable)
  {
    static_cast<Derived*>(this)->enable_parallel_impl(enable);
  }

  /**
   * @brief 计算指定关键点的描述子 / Compute descriptors at specified keypoints
   * @param cloud 输入点云 / Input point cloud
   * @param keypoint_indices 关键点索引列表 / List of keypoint indices
   * @param descriptors [out] 输出描述子向量 / Output descriptor vector
   * 
   * @code
   * // 计算所有关键点的描述子 / Compute descriptors for all keypoints
   * std::vector<size_t> keypoint_indices = detect_keypoints(cloud);
   * std::vector<fpfh_signature_t<float>> descriptors;
   * extractor.compute(cloud, keypoint_indices, descriptors);
   * 
   * std::cout << "计算了 / Computed " << descriptors.size() 
   *           << " 个描述子 / descriptors" << std::endl;
   * @endcode
   */
  void compute(const toolbox::types::point_cloud_t<DataType>& cloud,
               const std::vector<std::size_t>& keypoint_indices,
               std::vector<Signature>& descriptors)
  {
    static_cast<const Derived*>(this)->compute_impl(
        cloud, keypoint_indices, descriptors);
  }

  /**
   * @brief 计算指定关键点的描述子（智能指针版本） / Compute descriptors at specified keypoints (smart pointer version)
   * @param cloud 输入点云 / Input point cloud
   * @param keypoint_indices 关键点索引列表 / List of keypoint indices
   * @param descriptors [out] 输出描述子向量的智能指针 / Smart pointer to output descriptor vector
   */
  void compute(const toolbox::types::point_cloud_t<DataType>& cloud,
               const std::vector<std::size_t>& keypoint_indices,
               std::unique_ptr<std::vector<Signature>> descriptors)
  {
    static_cast<const Derived*>(this)->compute_impl(
        cloud, keypoint_indices, descriptors);
  }
};  // base_descriptor_extractor_t

}  // namespace toolbox::pcl