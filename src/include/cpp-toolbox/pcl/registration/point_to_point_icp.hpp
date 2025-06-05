#pragma once

#include <cpp-toolbox/pcl/registration/base_fine_registration.hpp>
#include <cpp-toolbox/pcl/knn/kdtree.hpp>
#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <Eigen/Core>
#include <Eigen/SVD>
#include <Eigen/Geometry>

namespace toolbox::pcl
{

/**
 * @brief Point-to-Point ICP 算法实现 / Point-to-Point ICP algorithm implementation
 * 
 * 该算法最小化点对之间的欧氏距离平方和。
 * This algorithm minimizes the sum of squared Euclidean distances between point pairs.
 * 
 * @tparam DataType 数据类型 / Data type
 * @tparam KNNSearcher KNN搜索器类型 / KNN searcher type
 * 
 * @code
 * // 使用默认KD树 / Using default KD-tree
 * point_to_point_icp_t<float> icp;
 * 
 * // 使用自定义KNN搜索器 / Using custom KNN searcher
 * point_to_point_icp_t<float, bfknn_parallel_t<float>> icp_parallel;
 * 
 * // 设置点云 / Set point clouds
 * icp.set_source(source_cloud);
 * icp.set_target(target_cloud);
 * 
 * // 执行配准 / Perform registration
 * fine_registration_result_t<float> result;
 * icp.align(initial_guess, result);
 * @endcode
 */
template<typename DataType, typename KNNSearcher = kdtree_t<DataType>>
class point_to_point_icp_t : public base_fine_registration_t<point_to_point_icp_t<DataType, KNNSearcher>, DataType>
{
public:
  using base_type = base_fine_registration_t<point_to_point_icp_t<DataType, KNNSearcher>, DataType>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::transformation_t;
  using typename base_type::result_type;
  
  using point_type = toolbox::types::point_t<DataType>;
  using knn_searcher_type = KNNSearcher;
  
  /**
   * @brief 构造函数 / Constructor
   * @param enable_parallel 是否启用并行优化（默认开启） / Enable parallel optimization (default enabled)
   */
  explicit point_to_point_icp_t(bool enable_parallel = false)
      : m_enable_parallel(enable_parallel),
        m_knn_searcher(std::make_unique<knn_searcher_type>())
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
    return correspondence_type_e::POINT_TO_POINT;
  }
  
  /**
   * @brief 设置异常值剔除比例 / Set outlier rejection ratio
   * @param ratio 剔除比例（0-1），0表示不剔除 / Rejection ratio (0-1), 0 means no rejection
   */
  void set_outlier_rejection_ratio(DataType ratio) {
    m_outlier_rejection_ratio = std::clamp(ratio, static_cast<DataType>(0), static_cast<DataType>(1));
  }
  
  /**
   * @brief 获取异常值剔除比例 / Get outlier rejection ratio
   */
  DataType get_outlier_rejection_ratio() const { return m_outlier_rejection_ratio; }
  
protected:
  friend class base_fine_registration_t<point_to_point_icp_t<DataType, KNNSearcher>, DataType>;
  
  /**
   * @brief 预处理（构建KD树） / Preprocessing (build KD-tree)
   */
  void preprocess_impl() {
    if (this->m_target_cloud) {
      LOG_INFO_S << "构建目标点云KD树 / Building target cloud KD-tree";
      m_knn_searcher->set_input(*this->m_target_cloud);
    }
  }
  
  /**
   * @brief 执行配准 / Perform registration
   */
  bool align_impl(const transformation_t& initial_guess, result_type& result);
  
  /**
   * @brief 查找对应关系 / Find correspondences
   */
  void find_correspondences(const point_cloud& transformed_source,
                           std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                           std::vector<DataType>& distances);
  
  /**
   * @brief 计算变换矩阵（使用SVD） / Compute transformation matrix (using SVD)
   */
  transformation_t compute_transformation(const point_cloud& source,
                                        const point_cloud& target,
                                        const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                                        const std::vector<DataType>& distances);
  
  /**
   * @brief 计算配准误差 / Compute registration error
   */
  DataType compute_error(const std::vector<DataType>& distances) const;
  
  /**
   * @brief 剔除异常值 / Reject outliers
   */
  void reject_outliers(std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
                      std::vector<DataType>& distances);
  
private:
  bool m_enable_parallel;                              ///< 是否启用并行优化 / Enable parallel optimization
  std::unique_ptr<knn_searcher_type> m_knn_searcher;  ///< KNN搜索器 / KNN searcher
  DataType m_outlier_rejection_ratio = 0.1;           ///< 异常值剔除比例 / Outlier rejection ratio
};


}  // namespace toolbox::pcl

// 包含实现文件 / Include implementation file
#include <cpp-toolbox/pcl/registration/impl/point_to_point_icp_impl.hpp>