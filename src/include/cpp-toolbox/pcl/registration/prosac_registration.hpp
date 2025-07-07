#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <vector>

#include <Eigen/Dense>
#include <Eigen/SVD>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/registration/base_coarse_registration.hpp>

namespace toolbox::pcl
{

/**
 * @brief PROSAC (渐进式采样一致性) 粗配准算法 / PROSAC (Progressive Sample
 * Consensus) coarse registration algorithm
 *
 * PROSAC是RANSAC的改进版本，通过利用对应关系的质量排序来实现更快的收敛。
 * 与随机均匀采样不同，PROSAC从逐渐增大的高质量对应关系集合中进行渐进式采样。
 * PROSAC is an improved version of RANSAC that achieves faster convergence by
 * leveraging quality ordering of correspondences. Unlike random uniform
 * sampling, PROSAC progressively samples from increasingly larger sets of
 * top-ranked correspondences.
 *
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or
 * double)
 *
 * @code
 * // 使用示例 / Usage example
 * // 1. 首先使用排序器对对应关系进行排序 / First sort correspondences using a
 * sorter auto sorter = std::make_shared<descriptor_distance_sorter_t<float>>();
 * sorter->set_correspondences(correspondences);
 * std::vector<float> quality_scores;
 * auto sorted_indices = sorter->compute_sorted_indices(quality_scores);
 *
 * // 2. 使用PROSAC进行配准 / Use PROSAC for registration
 * prosac_registration_t<float> prosac;
 * prosac.set_source(source_cloud);
 * prosac.set_target(target_cloud);
 * prosac.set_sorted_correspondences(correspondences, sorted_indices);
 * prosac.set_max_iterations(5000);
 * prosac.set_inlier_threshold(0.05f);
 * prosac.set_confidence(0.99f);
 *
 * registration_result_t<float> result;
 * if (prosac.align(result)) {
 *     std::cout << "Registration successful with " << result.inliers.size()
 *               << " inliers" << std::endl;
 * }
 * @endcode
 */
template<typename DataType>
class CPP_TOOLBOX_EXPORT prosac_registration_t
    : public base_coarse_registration_t<prosac_registration_t<DataType>,
                                        DataType>
{
  // Allow base class to access protected members
  friend class base_coarse_registration_t<prosac_registration_t<DataType>,
                                          DataType>;

public:
  using base_type =
      base_coarse_registration_t<prosac_registration_t<DataType>, DataType>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::correspondences_ptr;
  using typename base_type::result_type;
  using transformation_t = Eigen::Matrix<DataType, 4, 4>;
  using vector3_t = Eigen::Matrix<DataType, 3, 1>;
  using matrix3_t = Eigen::Matrix<DataType, 3, 3>;

  prosac_registration_t() = default;
  ~prosac_registration_t() = default;

  /**
   * @brief 设置置信度水平 / Set confidence level
   * @param confidence 置信度（0到1之间） / Confidence level (between 0 and 1)
   */
  void set_confidence(DataType confidence)
  {
    m_confidence = std::clamp(confidence, static_cast<DataType>(0.0),
                              static_cast<DataType>(1.0));
  }

  /**
   * @brief 获取置信度水平 / Get confidence level
   * @return 置信度 / Confidence level
   */
  [[nodiscard]] DataType get_confidence() const { return m_confidence; }

  /**
   * @brief 设置最小样本大小 / Set minimum sample size
   * @param size 样本大小（至少3） / Sample size (at least 3)
   */
  void set_sample_size(std::size_t size)
  {
    m_sample_size = std::max(size, std::size_t(3));
  }

  /**
   * @brief 获取样本大小 / Get sample size
   * @return 样本大小 / Sample size
   */
  [[nodiscard]] std::size_t get_sample_size() const { return m_sample_size; }

  /**
   * @brief 设置是否使用所有内点精炼结果 / Set whether to refine result using
   * all inliers
   * @param refine true启用精炼，false禁用 / true to enable refinement, false to
   * disable
   */
  void set_refine_result(bool refine) { m_refine_result = refine; }

  /**
   * @brief 获取是否精炼结果 / Get whether to refine result
   * @return 是否精炼 / Whether to refine
   */
  [[nodiscard]] bool get_refine_result() const { return m_refine_result; }

  /**
   * @brief 设置早停阈值（内点比例） / Set early stopping threshold (inlier
   * ratio)
   * @param ratio 内点比例阈值（0到1之间） / Inlier ratio threshold (between 0
   * and 1)
   */
  void set_early_stop_ratio(DataType ratio)
  {
    m_early_stop_ratio = std::clamp(ratio, static_cast<DataType>(0.0),
                                    static_cast<DataType>(1.0));
  }

  /**
   * @brief 获取早停阈值 / Get early stopping threshold
   * @return 早停阈值 / Early stopping threshold
   */
  [[nodiscard]] DataType get_early_stop_ratio() const
  {
    return m_early_stop_ratio;
  }

  /**
   * @brief 设置初始内点率估计 / Set initial inlier ratio estimate
   * @param ratio 初始内点率（0到1之间） / Initial inlier ratio (between 0 and 1)
   */
  void set_initial_inlier_ratio(DataType ratio)
  {
    m_initial_inlier_ratio =
        std::clamp(ratio, static_cast<DataType>(0.01), static_cast<DataType>(0.9));
  }

  /**
   * @brief 获取初始内点率 / Get initial inlier ratio
   * @return 初始内点率 / Initial inlier ratio
   */
  [[nodiscard]] DataType get_initial_inlier_ratio() const
  {
    return m_initial_inlier_ratio;
  }

  /**
   * @brief 设置非随机性阈值 / Set non-randomness threshold
   * @param threshold 阈值（通常0.05） / Threshold (typically 0.05)
   */
  void set_non_randomness_threshold(DataType threshold)
  {
    m_non_randomness_threshold = threshold;
  }

  /**
   * @brief 获取非随机性阈值 / Get non-randomness threshold
   * @return 非随机性阈值 / Non-randomness threshold
   */
  [[nodiscard]] DataType get_non_randomness_threshold() const
  {
    return m_non_randomness_threshold;
  }

  /**
   * @brief 设置已排序的对应关系 / Set sorted correspondences
   * @param correspondences 对应关系 / Correspondences
   * @param sorted_indices 按质量降序排序的索引（可选） / Indices sorted by
   * quality in descending order (optional)
   * @param quality_scores 质量分数（可选，用于调试） / Quality scores (optional,
   * for debugging)
   *
   * @note 如果不提供sorted_indices，则假设correspondences已经按质量降序排序
   * / If sorted_indices is not provided, assumes correspondences are already
   * sorted by quality in descending order
   */
  void set_sorted_correspondences(
      const correspondences_ptr& correspondences,
      const std::vector<std::size_t>& sorted_indices = {},
      const std::vector<DataType>& quality_scores = {})
  {
    this->set_correspondences(correspondences);

    if (!sorted_indices.empty()) {
      m_sorted_indices = sorted_indices;
    } else {
      // 如果未提供排序索引，假设对应关系已按顺序排序 / If no sorted indices
      // provided, assume correspondences are already sorted
      m_sorted_indices.resize(correspondences->size());
      std::iota(m_sorted_indices.begin(), m_sorted_indices.end(), 0);
    }

    m_quality_scores = quality_scores;
  }

  /**
   * @brief 获取排序后的索引 / Get sorted indices
   * @return 排序后的索引 / Sorted indices
   */
  [[nodiscard]] const std::vector<std::size_t>& get_sorted_indices() const
  {
    return m_sorted_indices;
  }

  // Methods needed by base class (must be accessible via friend)
protected:
  /**
   * @brief 派生类实现的配准算法 / Registration algorithm implementation by
   * derived class
   * @param result [out] 配准结果 / Registration result
   * @return 是否成功 / Whether successful
   */
  bool align_impl(result_type& result);

  /**
   * @brief 获取算法名称实现 / Get algorithm name implementation
   * @return 算法名称 / Algorithm name
   */
  [[nodiscard]] std::string get_algorithm_name_impl() const { return "PROSAC"; }

  /**
   * @brief 额外的输入验证 / Additional input validation
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool validate_input_impl() const;

  /**
   * @brief 设置对应关系的钩子函数 / Hook function for setting correspondences
   */
  void set_correspondences_impl(const correspondences_ptr& /*correspondences*/)
  {
    // PROSAC需要排序的对应关系 / PROSAC requires sorted correspondences
    // 清除之前的排序信息 / Clear previous sorting information
    m_sorted_indices.clear();
    m_quality_scores.clear();
    m_T_n.clear();
  }

private:
  /**
   * @brief 预计算采样调度T_n / Precompute sampling schedule T_n
   * @param n_correspondences 对应关系总数 / Total number of correspondences
   */
  void precompute_sampling_schedule(std::size_t n_correspondences);

  /**
   * @brief 渐进式采样 / Progressive sampling
   * @param sample [out] 采样结果 / Sampled correspondences
   * @param n 当前采样池大小 / Current sampling pool size
   * @param t 当前迭代次数 / Current iteration number
   * @param generator 随机数生成器 / Random number generator
   */
  void progressive_sample(std::vector<correspondence_t>& sample, std::size_t n,
                          std::size_t t, std::mt19937& generator) const;

  /**
   * @brief 使用SVD计算刚体变换 / Compute rigid transformation using SVD
   * @param sample 采样的对应关系 / Sampled correspondences
   * @return 变换矩阵 / Transformation matrix
   */
  [[nodiscard]] transformation_t estimate_transformation(
      const std::vector<correspondence_t>& sample) const;

  /**
   * @brief 计算内点 / Count inliers
   * @param transform 变换矩阵 / Transformation matrix
   * @param inliers [out] 内点索引 / Inlier indices
   * @return 内点数量 / Number of inliers
   */
  [[nodiscard]] std::size_t count_inliers(const transformation_t& transform,
                                           std::vector<std::size_t>& inliers) const;

  /**
   * @brief 检查非随机性准则 / Check non-randomness criterion
   * @param inlier_count 内点数量 / Number of inliers
   * @param n 当前采样池大小 / Current sampling pool size
   * @return 是否满足非随机性 / Whether non-randomness is satisfied
   */
  [[nodiscard]] bool check_non_randomness(std::size_t inlier_count,
                                           std::size_t n) const;

  /**
   * @brief 检查最大性准则 / Check maximality criterion
   * @param inlier_count 内点数量 / Number of inliers
   * @param n 当前采样池大小 / Current sampling pool size
   * @param t 当前迭代次数 / Current iteration number
   * @return 是否满足最大性 / Whether maximality is satisfied
   */
  [[nodiscard]] bool check_maximality(std::size_t inlier_count, std::size_t n,
                                      std::size_t t) const;

  /**
   * @brief 使用所有内点精炼变换 / Refine transformation using all inliers
   * @param inlier_indices 内点索引 / Inlier indices
   * @return 精炼后的变换矩阵 / Refined transformation matrix
   */
  [[nodiscard]] transformation_t
  refine_transformation(const std::vector<std::size_t>& inlier_indices) const;

  /**
   * @brief 计算二项式系数 / Compute binomial coefficient
   * @param n n值 / n value
   * @param k k值 / k value
   * @return C(n,k) / Binomial coefficient
   */
  [[nodiscard]] std::size_t compute_binomial_coefficient(std::size_t n,
                                                          std::size_t k) const;

  /**
   * @brief 计算beta函数 / Compute beta function
   * @param i 内点数 / Number of inliers
   * @param m 样本大小 / Sample size
   * @param n 对应关系数 / Number of correspondences
   * @return beta值 / Beta value
   */
  [[nodiscard]] DataType compute_beta(std::size_t i, std::size_t m,
                                      std::size_t n) const;

  /**
   * @brief 检查样本是否有效（非共线） / Check if sample is valid (non-collinear)
   * @param sample 样本 / Sample
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool is_sample_valid(
      const std::vector<correspondence_t>& sample) const;

  /**
   * @brief 计算质量评分 / Compute fitness score
   * @param transform 变换矩阵 / Transformation matrix
   * @param inliers 内点索引 / Inlier indices
   * @return 质量评分 / Fitness score
   */
  [[nodiscard]] DataType compute_fitness_score(
      const transformation_t& transform,
      const std::vector<std::size_t>& inliers) const;

  // PROSAC特定参数 / PROSAC-specific parameters
  DataType m_confidence = static_cast<DataType>(0.99);  ///< 置信度水平 /
                                                         ///< Confidence level
  std::size_t m_sample_size = 3;  ///< 最小样本大小 / Minimum sample size
  bool m_refine_result = true;    ///< 是否精炼结果 / Whether to refine result
  DataType m_early_stop_ratio = static_cast<DataType>(
      0.9);  ///< 早停内点比例阈值 / Early stop inlier ratio threshold
  DataType m_initial_inlier_ratio = static_cast<DataType>(
      0.1);  ///< 初始内点率估计 / Initial inlier ratio estimate
  DataType m_non_randomness_threshold = static_cast<DataType>(
      0.05);  ///< 非随机性阈值 / Non-randomness threshold

  // 排序信息 / Sorting information
  std::vector<std::size_t> m_sorted_indices;  ///< 排序后的索引 / Sorted indices
  std::vector<DataType> m_quality_scores;     ///< 质量分数 / Quality scores

  // 预计算的采样调度 / Precomputed sampling schedule
  std::vector<std::size_t> m_T_n;  ///< T_n值 / T_n values

  // 统计信息 / Statistics
  mutable std::size_t m_total_samples = 0;      ///< 总采样次数 / Total samples
  mutable std::size_t m_best_inlier_count = 0;  ///< 最佳内点数 / Best inlier
                                                 ///< count
};

}  // namespace toolbox::pcl

// 包含实现文件 / Include implementation file
#include <cpp-toolbox/pcl/registration/impl/prosac_registration_impl.hpp>