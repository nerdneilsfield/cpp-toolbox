#pragma once

#include <cpp-toolbox/pcl/descriptors/base_descriptor_extractor.hpp>
#include <cpp-toolbox/pcl/knn/base_knn.hpp>
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
};  // struct correspondence_t

/**
 * @brief 对应点生成器 / Correspondence generator
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
 * using CorrespondenceGen = correspondence_generator_t<float, fpfh_signature_t<float>, 
 *                                                     bfknn_generic_t<fpfh_signature_t<float>, 
 *                                                                     FPFHMetric<float>>>;
 * 
 * CorrespondenceGen corr_gen;
 * 
 * // 设置KNN算法 / Set KNN algorithm
 * auto knn = std::make_shared<bfknn_generic_t<fpfh_signature_t<float>, FPFHMetric<float>>>();
 * corr_gen.set_knn(knn);
 * 
 * // 设置源和目标数据 / Set source and target data
 * corr_gen.set_source(src_cloud, src_descriptors, src_keypoints);
 * corr_gen.set_destination(dst_cloud, dst_descriptors, dst_keypoints);
 * 
 * // 设置参数 / Set parameters
 * corr_gen.set_ratio(0.8f);  // Lowe's ratio test
 * corr_gen.set_mutual_verification(true);  // 双向验证 / Mutual verification
 * corr_gen.set_distance_threshold(0.25f);  // 最大描述子距离 / Max descriptor distance
 * 
 * // 计算对应关系 / Compute correspondences
 * std::vector<correspondence_t> correspondences;
 * corr_gen.compute(correspondences);
 * @endcode
 */
template<typename DataType, typename Signature, typename KNN>
class CPP_TOOLBOX_EXPORT correspondence_generator_t
{
public:
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;
  using signatures_ptr = std::shared_ptr<std::vector<Signature>>;
  using keypoint_indices_ptr = std::shared_ptr<std::vector<std::size_t>>;

  correspondence_generator_t() = default;
  correspondence_generator_t(const correspondence_generator_t&) = delete;
  correspondence_generator_t(correspondence_generator_t&&) = default;
  correspondence_generator_t& operator=(const correspondence_generator_t&) =
      delete;
  correspondence_generator_t& operator=(correspondence_generator_t&&) = default;
  ~correspondence_generator_t() = default;

  /**
   * @brief 设置KNN搜索算法 / Set KNN search algorithm
   * @param knn KNN算法的共享指针 / Shared pointer to KNN algorithm
   */
  void set_knn(std::shared_ptr<KNN> knn) { m_knn = knn; }

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
   * @brief 设置是否启用双向验证 / Set whether to enable mutual verification
   * @param mutual_verification true启用，false禁用 / true to enable, false to disable
   */
  void set_mutual_verification(bool mutual_verification)
  {
    m_mutual_verification = mutual_verification;
  }

  /**
   * @brief 设置距离阈值 / Set distance threshold
   * @param distance_threshold 最大允许的描述子距离 / Maximum allowed descriptor distance
   */
  void set_distance_threshold(float distance_threshold)
  {
    m_distance_threshold = distance_threshold;
  }

  /**
   * @brief 计算对应关系 / Compute correspondences
   * @param correspondences [out] 输出的对应关系 / Output correspondences
   */
  void compute(std::vector<correspondence_t>& correspondences);

  /**
   * @brief 获取最后一次计算的统计信息 / Get statistics from last computation
   * @return 统计信息字符串 / Statistics string
   */
  std::string get_statistics() const;

private:
  /**
   * @brief 验证输入数据的有效性 / Validate input data
   * @return true表示有效，false表示无效 / true if valid, false if invalid
   */
  bool validate_input() const;

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
   * @brief 应用比率测试 / Apply ratio test
   * @param candidates 候选对应关系 / Candidate correspondences
   * @return 是否通过测试 / Whether the test is passed
   */
  bool apply_ratio_test(const std::vector<correspondence_t>& candidates) const;

  /**
   * @brief 执行双向验证 / Perform mutual verification
   * @param forward_corrs 正向对应关系 / Forward correspondences
   * @param verified_corrs [out] 验证后的对应关系 / Verified correspondences
   */
  void perform_mutual_verification(const std::vector<correspondence_t>& forward_corrs,
                                  std::vector<correspondence_t>& verified_corrs);

  /**
   * @brief 应用距离阈值过滤 / Apply distance threshold filtering
   * @param correspondences [in/out] 对应关系 / Correspondences
   */
  void apply_distance_threshold(std::vector<correspondence_t>& correspondences) const;

  /**
   * @brief 计算几何一致性分数（简单验证） / Compute geometric consistency score (simple verification)
   * @param correspondences 对应关系 / Correspondences
   * @return 一致性分数 / Consistency score
   */
  float compute_geometric_consistency(const std::vector<correspondence_t>& correspondences) const;

  std::shared_ptr<KNN> m_knn;
  float m_ratio = 0.8F;  ///< 比率测试阈值 / Ratio test threshold
  float m_distance_threshold = std::numeric_limits<float>::max();  ///< 距离阈值 / Distance threshold
  bool m_mutual_verification = true;  ///< 是否启用双向验证 / Whether to enable mutual verification

  point_cloud_ptr m_src_cloud;
  point_cloud_ptr m_dst_cloud;
  signatures_ptr m_src_descriptors;
  signatures_ptr m_dst_descriptors;
  keypoint_indices_ptr m_src_keypoint_indices;
  keypoint_indices_ptr m_dst_keypoint_indices;

  // 统计信息 / Statistics
  std::size_t m_total_candidates = 0;
  std::size_t m_ratio_test_passed = 0;
  std::size_t m_mutual_test_passed = 0;
  std::size_t m_distance_test_passed = 0;
};  // class correspondence_generator_t

}  // namespace toolbox::pcl

#include <cpp-toolbox/pcl/correspondence/impl/correspondence_generator_impl.hpp>