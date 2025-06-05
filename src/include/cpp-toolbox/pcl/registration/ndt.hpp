#pragma once

#include <cpp-toolbox/pcl/registration/base_fine_registration.hpp>
#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <unordered_map>
#include <array>

namespace toolbox::pcl
{

/**
 * @brief NDT (Normal Distributions Transform) 算法实现 / NDT algorithm implementation
 * 
 * 该算法将点云转换为体素网格的概率分布，通过最大化似然函数进行配准。
 * This algorithm converts point cloud to probability distributions in voxel grid,
 * and performs registration by maximizing the likelihood function.
 * 
 * @tparam DataType 数据类型 / Data type
 * 
 * @code
 * // 使用示例 / Usage example
 * ndt_t<float> ndt;
 * 
 * // 设置体素分辨率 / Set voxel resolution
 * ndt.set_resolution(1.0);
 * 
 * // 设置优化参数 / Set optimization parameters
 * ndt.set_step_size(0.1);
 * ndt.set_outlier_ratio(0.55);
 * 
 * // 设置点云 / Set point clouds
 * ndt.set_source(source_cloud);
 * ndt.set_target(target_cloud);
 * 
 * // 执行配准 / Perform registration
 * fine_registration_result_t<float> result;
 * ndt.align(initial_guess, result);
 * @endcode
 */
template<typename DataType>
class ndt_t : public base_fine_registration_t<ndt_t<DataType>, DataType>
{
public:
  using base_type = base_fine_registration_t<ndt_t<DataType>, DataType>;
  using typename base_type::point_cloud;
  using typename base_type::point_cloud_ptr;
  using typename base_type::transformation_t;
  using typename base_type::result_type;
  
  using point_type = toolbox::types::point_t<DataType>;
  using Vector3 = Eigen::Matrix<DataType, 3, 1>;
  using Matrix3 = Eigen::Matrix<DataType, 3, 3>;
  using Vector6 = Eigen::Matrix<DataType, 6, 1>;
  using Matrix6 = Eigen::Matrix<DataType, 6, 6>;
  
  /**
   * @brief 体素单元结构 / Voxel cell structure
   */
  struct voxel_cell_t {
    Vector3 mean = Vector3::Zero();           ///< 均值 / Mean
    Matrix3 covariance = Matrix3::Zero();     ///< 协方差矩阵 / Covariance matrix
    Matrix3 covariance_inv = Matrix3::Zero(); ///< 协方差矩阵的逆 / Inverse of covariance
    std::size_t num_points = 0;               ///< 点数 / Number of points
    bool valid = false;                       ///< 是否有效 / Is valid
  };
  
  /**
   * @brief 构造函数 / Constructor
   * @param enable_parallel 是否启用并行优化（默认开启） / Enable parallel optimization (default enabled)
   */
  explicit ndt_t(bool enable_parallel = false)
      : m_enable_parallel(enable_parallel)
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
    return correspondence_type_e::POINT_TO_DISTRIBUTION;
  }
  
  /**
   * @brief 设置体素分辨率 / Set voxel resolution
   * @param resolution 体素大小 / Voxel size
   */
  void set_resolution(DataType resolution) {
    m_resolution = std::max(static_cast<DataType>(0.01), resolution);
    m_voxel_grid_updated = true;
  }
  
  /**
   * @brief 获取体素分辨率 / Get voxel resolution
   */
  DataType get_resolution() const { return m_resolution; }
  
  /**
   * @brief 设置步长 / Set step size
   * @param step_size 优化步长 / Optimization step size
   */
  void set_step_size(DataType step_size) {
    m_step_size = std::max(static_cast<DataType>(0.001), step_size);
  }
  
  /**
   * @brief 获取步长 / Get step size
   */
  DataType get_step_size() const { return m_step_size; }
  
  /**
   * @brief 设置异常值比例 / Set outlier ratio
   * @param ratio 异常值比例（0-1） / Outlier ratio (0-1)
   */
  void set_outlier_ratio(DataType ratio) {
    m_outlier_ratio = std::clamp(ratio, static_cast<DataType>(0), static_cast<DataType>(1));
  }
  
  /**
   * @brief 获取异常值比例 / Get outlier ratio
   */
  DataType get_outlier_ratio() const { return m_outlier_ratio; }
  
  /**
   * @brief 设置线搜索最大步长缩放次数 / Set line search maximum step scaling iterations
   */
  void set_line_search_max_iterations(std::size_t max_iter) {
    m_line_search_max_iterations = max_iter;
  }
  
  /**
   * @brief 获取线搜索最大步长缩放次数 / Get line search maximum step scaling iterations
   */
  std::size_t get_line_search_max_iterations() const { return m_line_search_max_iterations; }
  
protected:
  friend class base_fine_registration_t<ndt_t<DataType>, DataType>;
  
  /**
   * @brief 预处理（构建体素网格） / Preprocessing (build voxel grid)
   */
  void preprocess_impl();
  
  /**
   * @brief 执行配准 / Perform registration
   */
  bool align_impl(const transformation_t& initial_guess, result_type& result);
  
  /**
   * @brief 构建体素网格 / Build voxel grid
   */
  void build_voxel_grid();
  
  /**
   * @brief 计算体素索引 / Compute voxel index
   */
  std::array<int, 3> compute_voxel_index(const Vector3& point) const;
  
  /**
   * @brief 获取体素键值 / Get voxel key
   */
  std::size_t get_voxel_key(const std::array<int, 3>& index) const;
  
  /**
   * @brief 计算目标函数和梯度 / Compute objective function and gradient
   */
  DataType compute_objective(const transformation_t& transform,
                            Vector6* gradient = nullptr,
                            Matrix6* hessian = nullptr) const;
  
  /**
   * @brief 计算单个点的贡献 / Compute contribution of single point
   */
  DataType compute_point_contribution(const Vector3& point,
                                     const transformation_t& transform,
                                     Vector6* gradient = nullptr,
                                     Matrix6* hessian = nullptr) const;
  
  /**
   * @brief 执行More-Thuente线搜索 / Perform More-Thuente line search
   */
  DataType line_search(const transformation_t& current_transform,
                      const Vector6& direction,
                      DataType initial_step_size);
  
  /**
   * @brief 将优化向量转换为变换矩阵 / Convert optimization vector to transformation matrix
   */
  transformation_t vector_to_transformation(const Vector6& vec) const;
  
  /**
   * @brief 计算高斯分布的值 / Compute Gaussian distribution value
   */
  DataType gaussian_d1(DataType x_bar, DataType x, 
                      const Matrix3& c_inv) const;
  
  /**
   * @brief 计算雅可比矩阵 / Compute Jacobian matrix
   */
  void compute_jacobian(const Vector3& point, 
                       Eigen::Matrix<DataType, 3, 6>& jacobian) const;
  
private:
  bool m_enable_parallel;                           ///< 是否启用并行优化 / Enable parallel optimization
  DataType m_resolution = 1.0;                      ///< 体素分辨率 / Voxel resolution
  DataType m_step_size = 0.1;                       ///< 步长 / Step size
  DataType m_outlier_ratio = 0.55;                  ///< 异常值比例 / Outlier ratio
  std::size_t m_line_search_max_iterations = 20;    ///< 线搜索最大迭代次数 / Line search max iterations
  
  bool m_voxel_grid_updated = true;                 ///< 体素网格是否需要更新 / Voxel grid needs update
  std::unordered_map<std::size_t, voxel_cell_t> m_voxel_grid; ///< 体素网格 / Voxel grid
  
  // 预计算的常数
  DataType m_gauss_d1 = 0;                          ///< 高斯归一化常数 / Gaussian normalization constant
  DataType m_gauss_d2 = 0;                          ///< 高斯指数系数 / Gaussian exponential coefficient
};


}  // namespace toolbox::pcl

// 包含实现文件 / Include implementation file
#include <cpp-toolbox/pcl/registration/impl/ndt_impl.hpp>