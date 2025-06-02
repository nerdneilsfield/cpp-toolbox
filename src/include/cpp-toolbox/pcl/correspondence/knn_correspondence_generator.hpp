#pragma once

#include <cpp-toolbox/pcl/correspondence/base_correspondence_generator.hpp>
#include <cpp-toolbox/pcl/knn/base_knn.hpp>
#include <unordered_map>
#include <algorithm>

namespace toolbox::pcl
{

/**
 * @brief 基于KNN的对应点生成器 / KNN-based correspondence generator
 * 
 * 使用KNN算法在描述子空间中快速查找对应点，支持多种验证策略
 * Uses KNN algorithm to quickly find correspondences in descriptor space,
 * supporting various verification strategies
 * 
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 * @tparam Signature 描述子签名类型 / Descriptor signature type
 * @tparam KNN KNN搜索算法类型 / KNN search algorithm type
 * 
 * @code
 * // 使用示例 / Usage example
 * using CorrespondenceGen = knn_correspondence_generator_t<float, fpfh_signature_t<float>, 
 *                                                          bfknn_generic_t<fpfh_signature_t<float>, 
 *                                                                         FPFHMetric<float>>>;
 * 
 * CorrespondenceGen corr_gen;
 * 
 * // 设置KNN算法 / Set KNN algorithm
 * auto knn = std::make_shared<bfknn_generic_t<fpfh_signature_t<float>, FPFHMetric<float>>>();
 * corr_gen.set_knn(knn);
 * 
 * // 设置数据和参数 / Set data and parameters
 * corr_gen.set_source(src_cloud, src_descriptors, src_keypoints);
 * corr_gen.set_destination(dst_cloud, dst_descriptors, dst_keypoints);
 * corr_gen.set_ratio(0.8f);
 * corr_gen.set_mutual_verification(true);
 * 
 * // 计算对应关系 / Compute correspondences
 * std::vector<correspondence_t> correspondences;
 * corr_gen.compute(correspondences);
 * @endcode
 */
template<typename DataType, typename Signature, typename KNN>
class knn_correspondence_generator_t 
    : public base_correspondence_generator_t<
        knn_correspondence_generator_t<DataType, Signature, KNN>,
        DataType, Signature>
{
public:
  using base_type = base_correspondence_generator_t<
      knn_correspondence_generator_t<DataType, Signature, KNN>,
      DataType, Signature>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::signatures_ptr;
  using typename base_type::keypoint_indices_ptr;

  /**
   * @brief 设置KNN搜索算法 / Set KNN search algorithm
   * @param knn KNN算法的共享指针 / Shared pointer to KNN algorithm
   */
  void set_knn(std::shared_ptr<KNN> knn) { m_knn = knn; }

  /**
   * @brief 获取KNN搜索算法 / Get KNN search algorithm
   * @return KNN算法的共享指针 / Shared pointer to KNN algorithm
   */
  std::shared_ptr<KNN> get_knn() const { return m_knn; }

  /**
   * @brief 计算对应关系的实现 / Implementation of computing correspondences
   * @param correspondences [out] 输出的对应关系 / Output correspondences
   */
  void compute_impl(std::vector<correspondence_t>& correspondences);

private:
  /**
   * @brief 构建KNN的描述子数据集 / Build descriptor dataset for KNN
   */
  void build_knn_dataset();

  /**
   * @brief 为单个源描述子查找候选匹配 / Find candidate matches for a single source descriptor
   * @param src_idx 源描述子索引 / Source descriptor index
   * @param candidates [out] 候选对应关系 / Candidate correspondences
   */
  void find_candidates_for_descriptor(std::size_t src_idx, 
                                     std::vector<correspondence_t>& candidates) const;

  /**
   * @brief 执行双向验证 / Perform mutual verification
   * @param forward_corrs 正向对应关系 / Forward correspondences
   * @param verified_corrs [out] 验证后的对应关系 / Verified correspondences
   */
  void perform_mutual_verification(const std::vector<correspondence_t>& forward_corrs,
                                  std::vector<correspondence_t>& verified_corrs);

  std::shared_ptr<KNN> m_knn;
};

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/correspondence/impl/knn_correspondence_generator_impl.hpp>