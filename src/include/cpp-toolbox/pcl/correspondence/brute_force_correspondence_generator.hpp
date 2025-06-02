#pragma once

#include <cpp-toolbox/pcl/correspondence/base_correspondence_generator.hpp>
#include <algorithm>
#include <limits>

namespace toolbox::pcl
{

/**
 * @brief 暴力搜索对应点生成器 / Brute-force correspondence generator
 * 
 * 直接计算所有描述子对之间的距离，不使用任何加速结构。
 * 适用于小规模数据或需要精确结果的场景。
 * Directly computes distances between all descriptor pairs without any acceleration structure.
 * Suitable for small-scale data or scenarios requiring exact results.
 * 
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * 
 * @code
 * // 使用示例 / Usage example
 * brute_force_correspondence_generator_t<float, fpfh_signature_t<float>> corr_gen;
 * 
 * // 设置数据和参数 / Set data and parameters
 * corr_gen.set_source(src_cloud, src_descriptors, src_keypoints);
 * corr_gen.set_destination(dst_cloud, dst_descriptors, dst_keypoints);
 * corr_gen.set_ratio(0.8f);
 * corr_gen.set_mutual_verification(true);
 * corr_gen.enable_parallel(true);  // 启用并行计算 / Enable parallel computation
 * 
 * // 计算对应关系 / Compute correspondences
 * std::vector<correspondence_t> correspondences;
 * corr_gen.compute(correspondences);
 * @endcode
 */
template<typename DataType, typename Signature>
class brute_force_correspondence_generator_t 
    : public base_correspondence_generator_t<
        brute_force_correspondence_generator_t<DataType, Signature>,
        DataType, Signature>
{
public:
  using base_type = base_correspondence_generator_t<
      brute_force_correspondence_generator_t<DataType, Signature>,
      DataType, Signature>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::signatures_ptr;
  using typename base_type::keypoint_indices_ptr;

  /**
   * @brief 启用或禁用并行计算 / Enable or disable parallel computation
   * @param enable true启用并行，false禁用 / true to enable parallel, false to disable
   */
  void enable_parallel(bool enable) { m_enable_parallel = enable; }

  /**
   * @brief 获取是否启用并行计算 / Get whether parallel computation is enabled
   * @return 是否启用并行 / Whether parallel is enabled
   */
  bool is_parallel_enabled() const { return m_enable_parallel; }

  /**
   * @brief 计算对应关系的实现 / Implementation of computing correspondences
   * @param correspondences [out] 输出的对应关系 / Output correspondences
   */
  void compute_impl(std::vector<correspondence_t>& correspondences);

private:
  /**
   * @brief 为单个源描述子查找候选匹配（串行版本） / Find candidate matches for a single source descriptor (serial version)
   * @param src_idx 源描述子索引 / Source descriptor index
   * @param candidates [out] 候选对应关系 / Candidate correspondences
   */
  void find_candidates_for_descriptor_serial(std::size_t src_idx, 
                                            std::vector<correspondence_t>& candidates) const;

  /**
   * @brief 计算所有对应关系（并行版本） / Compute all correspondences (parallel version)
   * @param all_candidates [out] 所有候选对应关系 / All candidate correspondences
   */
  void compute_all_candidates_parallel(std::vector<correspondence_t>& all_candidates);

  /**
   * @brief 执行双向验证 / Perform mutual verification
   * @param forward_corrs 正向对应关系 / Forward correspondences
   * @param verified_corrs [out] 验证后的对应关系 / Verified correspondences
   */
  void perform_mutual_verification(const std::vector<correspondence_t>& forward_corrs,
                                  std::vector<correspondence_t>& verified_corrs);

  bool m_enable_parallel = false;  ///< 是否启用并行计算 / Whether to enable parallel computation
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/correspondence/impl/brute_force_correspondence_generator_impl.hpp>