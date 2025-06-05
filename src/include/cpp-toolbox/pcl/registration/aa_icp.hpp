#pragma once

#include <cpp-toolbox/pcl/registration/point_to_point_icp.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <deque>
#include <Eigen/QR>

namespace toolbox::pcl
{

/**
 * @brief AA-ICP (Anderson Acceleration ICP) 算法实现 / AA-ICP algorithm implementation
 * 
 * 该算法使用Anderson加速技术加速ICP的定点迭代收敛。
 * This algorithm uses Anderson acceleration to speed up the fixed-point iteration convergence of ICP.
 * 
 * @tparam DataType 数据类型 / Data type
 * @tparam KNNSearcher KNN搜索器类型 / KNN searcher type
 * @tparam BaseICP 基础ICP算法类型（默认为Point-to-Point） / Base ICP algorithm type
 * 
 * @code
 * // 使用示例 / Usage example
 * aa_icp_t<float> aa_icp;
 * 
 * // 设置Anderson加速参数 / Set Anderson acceleration parameters
 * aa_icp.set_anderson_m(5);  // 历史窗口大小
 * aa_icp.set_beta(0.5);      // 阻尼因子
 * 
 * // 设置点云 / Set point clouds
 * aa_icp.set_source(source_cloud);
 * aa_icp.set_target(target_cloud);
 * 
 * // 执行配准 / Perform registration
 * fine_registration_result_t<float> result;
 * aa_icp.align(initial_guess, result);
 * @endcode
 */
template<typename DataType, 
         typename KNNSearcher = kdtree_t<DataType>,
         typename BaseICP = point_to_point_icp_t<DataType, KNNSearcher>>
class aa_icp_t : public base_fine_registration_t<aa_icp_t<DataType, KNNSearcher, BaseICP>, DataType>
{
public:
  using base_type = base_fine_registration_t<aa_icp_t<DataType, KNNSearcher, BaseICP>, DataType>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::transformation_t;
  using typename base_type::result_type;
  
  using base_icp_type = BaseICP;
  using VectorX = Eigen::Matrix<DataType, Eigen::Dynamic, 1>;
  using MatrixX = Eigen::Matrix<DataType, Eigen::Dynamic, Eigen::Dynamic>;
  
  /**
   * @brief 构造函数 / Constructor
   * @param enable_parallel 是否启用并行优化（默认开启） / Enable parallel optimization (default enabled)
   */
  explicit aa_icp_t(bool enable_parallel = false)
      : m_base_icp(enable_parallel)
  {
    // 传递基类的参数设置函数到基础ICP
    m_base_icp.set_outlier_rejection_ratio(0.1);
  }
  
  /**
   * @brief 设置Anderson加速的历史窗口大小 / Set Anderson acceleration history window size
   * @param m 历史窗口大小（默认5） / History window size (default 5)
   */
  void set_anderson_m(std::size_t m) { m_anderson_m = m; }
  
  /**
   * @brief 获取Anderson加速的历史窗口大小 / Get Anderson acceleration history window size
   */
  std::size_t get_anderson_m() const { return m_anderson_m; }
  
  /**
   * @brief 设置阻尼因子 / Set damping factor
   * @param beta 阻尼因子（0-1），用于稳定性 / Damping factor (0-1) for stability
   */
  void set_beta(DataType beta) {
    m_beta = std::clamp(beta, static_cast<DataType>(0), static_cast<DataType>(1));
  }
  
  /**
   * @brief 获取阻尼因子 / Get damping factor
   */
  DataType get_beta() const { return m_beta; }
  
  /**
   * @brief 设置正则化参数 / Set regularization parameter
   * @param lambda 正则化系数 / Regularization coefficient
   */
  void set_regularization(DataType lambda) {
    m_regularization = std::max(static_cast<DataType>(0), lambda);
  }
  
  /**
   * @brief 获取正则化参数 / Get regularization parameter
   */
  DataType get_regularization() const { return m_regularization; }
  
  /**
   * @brief 设置是否启用安全保护 / Set whether to enable safeguarding
   * @param enable 是否启用 / Whether to enable
   */
  void set_enable_safeguarding(bool enable) { m_enable_safeguarding = enable; }
  
  /**
   * @brief 获取是否启用安全保护 / Get whether safeguarding is enabled
   */
  bool get_enable_safeguarding() const { return m_enable_safeguarding; }
  
  /**
   * @brief 获取对应关系类型（从基础ICP继承） / Get correspondence type (inherited from base ICP)
   */
  correspondence_type_e get_correspondence_type_impl() const {
    return m_base_icp.get_correspondence_type_impl();
  }
  
  /**
   * @brief 设置是否启用并行优化 / Set whether to enable parallel optimization
   */
  void set_enable_parallel(bool enable) { 
    m_base_icp.set_enable_parallel(enable);
  }
  
  /**
   * @brief 获取是否启用并行优化 / Get whether parallel optimization is enabled
   */
  bool get_enable_parallel() const { 
    return m_base_icp.get_enable_parallel();
  }
  
protected:
  friend class base_fine_registration_t<aa_icp_t<DataType, KNNSearcher, BaseICP>, DataType>;
  
  /**
   * @brief 额外的输入验证 / Additional input validation
   */
  bool validate_input_impl() const {
    // 基础验证交给base_icp自己处理
    return true;
  }
  
  /**
   * @brief 预处理 / Preprocessing
   */
  void preprocess_impl() {
    // 设置基础ICP的点云
    m_base_icp.set_source(this->m_source_cloud);
    m_base_icp.set_target(this->m_target_cloud);
    
    // 传递最大对应距离设置
    m_base_icp.set_max_correspondence_distance(this->m_max_correspondence_distance);
    
    // 基础ICP会在自己的align中调用preprocess
  }
  
  /**
   * @brief 执行配准 / Perform registration
   */
  bool align_impl(const transformation_t& initial_guess, result_type& result);
  
  /**
   * @brief 将变换矩阵转换为向量表示 / Convert transformation matrix to vector representation
   */
  VectorX transformation_to_vector(const transformation_t& transform) const;
  
  /**
   * @brief 将向量表示转换为变换矩阵 / Convert vector representation to transformation matrix
   */
  transformation_t vector_to_transformation(const VectorX& vec) const;
  
  /**
   * @brief 执行一步基础ICP迭代 / Perform one step of base ICP iteration
   */
  transformation_t perform_base_icp_step(const transformation_t& current_transform,
                                       DataType& error,
                                       std::size_t& num_correspondences);
  
  /**
   * @brief Anderson加速更新 / Anderson acceleration update
   */
  VectorX anderson_acceleration_update(const std::deque<VectorX>& g_history,
                                      const std::deque<VectorX>& x_history);
  
  /**
   * @brief 检查数值稳定性 / Check numerical stability
   */
  bool is_numerically_stable(const VectorX& vec) const;
  
private:
  base_icp_type m_base_icp;                    ///< 基础ICP算法 / Base ICP algorithm
  
  std::size_t m_anderson_m = 5;                ///< Anderson历史窗口大小 / Anderson history window size
  DataType m_beta = 0.5;                       ///< 阻尼因子 / Damping factor
  DataType m_regularization = 1e-6;            ///< 正则化参数 / Regularization parameter
  bool m_enable_safeguarding = true;           ///< 是否启用安全保护 / Enable safeguarding
  
  // Anderson加速的历史记录
  std::deque<VectorX> m_g_history;             ///< 残差历史 / Residual history
  std::deque<VectorX> m_x_history;             ///< 迭代历史 / Iteration history
  
  bool m_initialized = false;                    ///< 是否已初始化 / Whether initialized
};


}  // namespace toolbox::pcl

// 包含实现文件 / Include implementation file
#include <cpp-toolbox/pcl/registration/impl/aa_icp_impl.hpp>