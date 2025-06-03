#pragma once

#include <algorithm>
#include <chrono>
#include <random>
#include <vector>

#include <Eigen/Dense>
#include <Eigen/SVD>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/registration/base_coarse_registration.hpp>

namespace toolbox::pcl
{

/**
 * @brief RANSAC粗配准算法 / RANSAC coarse registration algorithm
 *
 * 基于对应关系的RANSAC配准算法，通过随机采样一致性方法找到最佳的刚体变换。
 * Correspondence-based RANSAC registration algorithm that finds the best rigid
 * transformation through random sample consensus.
 *
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or double)
 *
 * @code
 * // 使用示例 / Usage example
 * ransac_registration_t<float> ransac;
 * ransac.set_source(source_cloud);
 * ransac.set_target(target_cloud);
 * ransac.set_correspondences(correspondences);
 * ransac.set_max_iterations(1000);
 * ransac.set_inlier_threshold(0.05f);
 * ransac.set_confidence(0.99f);
 *
 * registration_result_t<float> result;
 * if (ransac.align(result)) {
 *     std::cout << "Registration successful with " << result.inliers.size()
 *               << " inliers" << std::endl;
 * }
 * @endcode
 */
template<typename DataType>
class CPP_TOOLBOX_EXPORT ransac_registration_t
    : public base_coarse_registration_t<ransac_registration_t<DataType>, DataType>
{
  // Allow base class to access protected members
  friend class base_coarse_registration_t<ransac_registration_t<DataType>, DataType>;
public:
  using base_type = base_coarse_registration_t<ransac_registration_t<DataType>, DataType>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::correspondences_ptr;
  using typename base_type::result_type;
  using transformation_t = Eigen::Matrix<DataType, 4, 4>;
  using vector3_t = Eigen::Matrix<DataType, 3, 1>;
  using matrix3_t = Eigen::Matrix<DataType, 3, 3>;

  ransac_registration_t() = default;
  ~ransac_registration_t() = default;

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
  void set_sample_size(std::size_t size) { m_sample_size = std::max(size, std::size_t(3)); }

  /**
   * @brief 获取样本大小 / Get sample size
   * @return 样本大小 / Sample size
   */
  [[nodiscard]] std::size_t get_sample_size() const { return m_sample_size; }

  /**
   * @brief 设置是否使用所有内点精炼结果 / Set whether to refine result using all
   * inliers
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
   * @brief 设置早停阈值（内点比例） / Set early stopping threshold (inlier ratio)
   * @param ratio 内点比例阈值（0到1之间） / Inlier ratio threshold (between 0 and 1)
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
  [[nodiscard]] DataType get_early_stop_ratio() const { return m_early_stop_ratio; }

  // Methods needed by base class (must be accessible via friend)
protected:
  /**
   * @brief 派生类实现的配准算法 / Registration algorithm implementation by derived
   * class
   * @param result [out] 配准结果 / Registration result
   * @return 是否成功 / Whether successful
   */
  bool align_impl(result_type& result);

  /**
   * @brief 获取算法名称实现 / Get algorithm name implementation
   * @return 算法名称 / Algorithm name
   */
  [[nodiscard]] std::string get_algorithm_name_impl() const { return "RANSAC"; }

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
    // RANSAC需要对应关系，这里可以添加额外的处理
    // RANSAC requires correspondences, additional processing can be added here
  }

private:
  /**
   * @brief 计算需要的迭代次数 / Calculate required number of iterations
   * @param outlier_ratio 外点比例 / Outlier ratio
   * @return 迭代次数 / Number of iterations
   */
  [[nodiscard]] std::size_t calculate_iterations(DataType outlier_ratio) const;

  /**
   * @brief 随机采样对应关系 / Randomly sample correspondences
   * @param sample [out] 采样结果 / Sampled correspondences
   * @param generator 随机数生成器 / Random number generator
   */
  void sample_correspondences(std::vector<correspondence_t>& sample,
                              std::mt19937& generator) const;

  /**
   * @brief 使用SVD计算刚体变换 / Compute rigid transformation using SVD
   * @param sample 采样的对应关系 / Sampled correspondences
   * @return 变换矩阵 / Transformation matrix
   */
  [[nodiscard]] transformation_t
  estimate_rigid_transform_svd(const std::vector<correspondence_t>& sample) const;

  /**
   * @brief 计算内点 / Count inliers
   * @param transform 变换矩阵 / Transformation matrix
   * @param inliers [out] 内点索引 / Inlier indices
   * @return 内点数量 / Number of inliers
   */
  [[nodiscard]] std::size_t count_inliers(const transformation_t& transform,
                                           std::vector<std::size_t>& inliers) const;

  /**
   * @brief 使用所有内点精炼变换 / Refine transformation using all inliers
   * @param inlier_indices 内点索引 / Inlier indices
   * @return 精炼后的变换矩阵 / Refined transformation matrix
   */
  [[nodiscard]] transformation_t
  refine_transformation(const std::vector<std::size_t>& inlier_indices) const;

  /**
   * @brief 计算质量评分 / Compute fitness score
   * @param transform 变换矩阵 / Transformation matrix
   * @param inliers 内点索引 / Inlier indices
   * @return 质量评分 / Fitness score
   */
  [[nodiscard]] DataType compute_fitness_score(const transformation_t& transform,
                                                const std::vector<std::size_t>& inliers) const;

  // RANSAC特定参数 / RANSAC-specific parameters
  DataType m_confidence = static_cast<DataType>(0.99);  ///< 置信度水平 / Confidence level
  std::size_t m_sample_size = 3;  ///< 最小样本大小 / Minimum sample size
  bool m_refine_result = true;  ///< 是否精炼结果 / Whether to refine result
  DataType m_early_stop_ratio =
      static_cast<DataType>(0.9);  ///< 早停内点比例阈值 / Early stop inlier ratio threshold
};

}  // namespace toolbox::pcl

// 包含实现文件 / Include implementation file
#include <cpp-toolbox/pcl/registration/impl/ransac_registration_impl.hpp>