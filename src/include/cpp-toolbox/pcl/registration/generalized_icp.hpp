#pragma once

#include <cpp-toolbox/pcl/registration/base_fine_registration.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace toolbox::pcl
{

/**
 * @brief Generalized ICP (Plane-to-Plane) 算法实现 / Generalized ICP algorithm implementation
 * 
 * 该算法考虑两个点云的局部平面结构，通过协方差矩阵建模局部不确定性。
 * This algorithm considers local planar structure of both point clouds,
 * modeling local uncertainty through covariance matrices.
 * 
 * @tparam DataType 数据类型 / Data type
 * @tparam KNNSearcher KNN搜索器类型 / KNN searcher type
 * 
 * @code
 * // 使用示例 / Usage example
 * generalized_icp_t<float> gicp;
 * 
 * // 设置点云 / Set point clouds
 * gicp.set_source(source_cloud);
 * gicp.set_target(target_cloud);
 * 
 * // 设置参数 / Set parameters
 * gicp.set_k_correspondences(20);  // 用于协方差估计的邻居数
 * gicp.set_max_iterations(50);
 * 
 * // 执行配准 / Perform registration
 * fine_registration_result_t<float> result;
 * gicp.align(initial_guess, result);
 * @endcode
 */
template<typename DataType, typename KNNSearcher = kdtree_t<DataType>>
class generalized_icp_t : public base_fine_registration_t<generalized_icp_t<DataType, KNNSearcher>, DataType>
{
public:
  using base_type = base_fine_registration_t<generalized_icp_t<DataType, KNNSearcher>, DataType>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::transformation_t;
  using typename base_type::result_type;
  
  using point_type = toolbox::types::point_t<DataType>;
  using knn_searcher_type = KNNSearcher;
  using Matrix3 = Eigen::Matrix<DataType, 3, 3>;
  using Vector3 = Eigen::Matrix<DataType, 3, 1>;
  using Vector6 = Eigen::Matrix<DataType, 6, 1>;
  using Matrix6 = Eigen::Matrix<DataType, 6, 6>;
  
  /**
   * @brief 构造函数 / Constructor
   * @param enable_parallel 是否启用并行优化（默认开启） / Enable parallel optimization (default enabled)
   */
  explicit generalized_icp_t(bool enable_parallel = false)
      : m_enable_parallel(enable_parallel),
        m_source_knn_searcher(std::make_unique<knn_searcher_type>()),
        m_target_knn_searcher(std::make_unique<knn_searcher_type>())
  {
  }
  
  /**
   * @brief 设置是否启用并行优化 / Set whether to enable parallel optimization
   */
  void set_enable_parallel(bool enable) { m_enable_parallel = enable; }
  
  /**
   * @brief 获取是否启用并行优化 / Get whether parallel optimization is enabled
   */
  bool get_enable_parallel() const { return m_enable_parallel; }
  
  /**
   * @brief 获取对应关系类型 / Get correspondence type
   */
  correspondence_type_e get_correspondence_type_impl() const {
    return correspondence_type_e::PLANE_TO_PLANE;
  }
  
  /**
   * @brief 设置用于协方差估计的近邻数 / Set number of neighbors for covariance estimation
   */
  void set_k_correspondences(std::size_t k) { m_k_correspondences = k; }
  
  /**
   * @brief 获取用于协方差估计的近邻数 / Get number of neighbors for covariance estimation
   */
  std::size_t get_k_correspondences() const { return m_k_correspondences; }
  
  /**
   * @brief 设置协方差正则化参数 / Set covariance regularization parameter
   * @param epsilon 正则化系数 / Regularization coefficient
   */
  void set_covariance_epsilon(DataType epsilon) {
    m_covariance_epsilon = std::max(static_cast<DataType>(0), epsilon);
  }
  
  /**
   * @brief 获取协方差正则化参数 / Get covariance regularization parameter
   */
  DataType get_covariance_epsilon() const { return m_covariance_epsilon; }
  
  /**
   * @brief 设置异常值剔除比例 / Set outlier rejection ratio
   */
  void set_outlier_rejection_ratio(DataType ratio) {
    m_outlier_rejection_ratio = std::clamp(ratio, static_cast<DataType>(0), static_cast<DataType>(1));
  }
  
  /**
   * @brief 获取异常值剔除比例 / Get outlier rejection ratio
   */
  DataType get_outlier_rejection_ratio() const { return m_outlier_rejection_ratio; }
  
  /**
   * @brief 设置优化器最大内部迭代次数 / Set optimizer maximum inner iterations
   */
  void set_optimizer_max_iterations(std::size_t max_iter) {
    m_optimizer_max_iterations = max_iter;
  }
  
  /**
   * @brief 获取优化器最大内部迭代次数 / Get optimizer maximum inner iterations
   */
  std::size_t get_optimizer_max_iterations() const { return m_optimizer_max_iterations; }
  
protected:
  friend class base_fine_registration_t<generalized_icp_t<DataType, KNNSearcher>, DataType>;
  
  /**
   * @brief 预处理（构建KD树和计算协方差） / Preprocessing (build KD-trees and compute covariances)
   */
  void preprocess_impl();
  
  /**
   * @brief 执行配准 / Perform registration
   */
  bool align_impl(const transformation_t& initial_guess, result_type& result);
  
  /**
   * @brief 计算点云的协方差矩阵 / Compute covariance matrices for point cloud
   */
  void compute_covariances(const point_cloud& cloud,
                          knn_searcher_type& searcher,
                          std::vector<Matrix3>& covariances);
  
  /**
   * @brief 查找对应关系 / Find correspondences
   */
  void find_correspondences(const point_cloud& transformed_source,
                           std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                           std::vector<DataType>& distances);
  
  /**
   * @brief 计算变换矩阵（使用L-BFGS优化） / Compute transformation matrix (using L-BFGS optimization)
   */
  transformation_t compute_transformation(const point_cloud& source,
                                        const point_cloud& target,
                                        const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                                        const transformation_t& current_transform);
  
  /**
   * @brief 计算Mahalanobis距离和梯度 / Compute Mahalanobis distance and gradient
   */
  DataType compute_mahalanobis_distance(const Vector3& p_src,
                                       const Vector3& p_tgt,
                                       const Matrix3& C_src,
                                       const Matrix3& C_tgt,
                                       const transformation_t& transform,
                                       Vector6* gradient = nullptr) const;
  
  /**
   * @brief 计算配准误差 / Compute registration error
   */
  DataType compute_error(const point_cloud& source,
                        const point_cloud& target,
                        const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                        const transformation_t& transform) const;
  
  /**
   * @brief 剔除异常值 / Reject outliers
   */
  void reject_outliers(std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                      std::vector<DataType>& distances);
  
  /**
   * @brief 优化目标函数 / Optimization objective function
   */
  DataType objective_function(const Vector6& x,
                             const point_cloud& source,
                             const point_cloud& target,
                             const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                             const transformation_t& base_transform,
                             Vector6* gradient = nullptr) const;
  
  /**
   * @brief L-BFGS优化器 / L-BFGS optimizer
   */
  Vector6 lbfgs_optimize(const Vector6& x0,
                        const point_cloud& source,
                        const point_cloud& target,
                        const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                        const transformation_t& base_transform);
  
  /**
   * @brief 将变换矩阵转换为优化向量 / Convert transformation matrix to optimization vector
   */
  Vector6 transformation_to_vector(const transformation_t& transform) const;
  
  /**
   * @brief 将优化向量转换为变换矩阵 / Convert optimization vector to transformation matrix
   */
  transformation_t vector_to_transformation(const Vector6& vec) const;
  
private:
  bool m_enable_parallel;                                    ///< 是否启用并行优化 / Enable parallel optimization
  std::unique_ptr<knn_searcher_type> m_source_knn_searcher; ///< 源点云KNN搜索器 / Source KNN searcher
  std::unique_ptr<knn_searcher_type> m_target_knn_searcher; ///< 目标点云KNN搜索器 / Target KNN searcher
  
  std::size_t m_k_correspondences = 20;                     ///< 协方差估计的近邻数 / Neighbors for covariance
  DataType m_covariance_epsilon = 0.001;                   ///< 协方差正则化参数 / Covariance regularization
  DataType m_outlier_rejection_ratio = 0.1;                ///< 异常值剔除比例 / Outlier rejection ratio
  std::size_t m_optimizer_max_iterations = 10;             ///< 优化器最大迭代次数 / Optimizer max iterations
  
  std::vector<Matrix3> m_source_covariances;               ///< 源点云协方差矩阵 / Source covariances
  std::vector<Matrix3> m_target_covariances;               ///< 目标点云协方差矩阵 / Target covariances
};


}  // namespace toolbox::pcl

// 包含实现文件 / Include implementation file
#include <cpp-toolbox/pcl/registration/impl/generalized_icp_impl.hpp>