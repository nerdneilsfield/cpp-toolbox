#pragma once

#include <vector>

#include <Eigen/Dense>

namespace toolbox::pcl
{

/**
 * @brief 配准结果结构体 / Registration result structure
 */
template<typename DataType>
struct registration_result_t
{
  using transformation_t = Eigen::Matrix<DataType, 4, 4>;

  transformation_t transformation;  ///< 变换矩阵 / Transformation matrix
  DataType fitness_score;  ///< 配准质量评分 / Registration fitness score
  std::vector<std::size_t> inliers;  ///< 内点索引 / Inlier indices
  std::size_t num_iterations = 0;  ///< 实际迭代次数 / Actual iteration count
  bool converged = false;  ///< 是否收敛 / Whether converged
};

/**
 * @brief 配准迭代状态 / Registration iteration state
 */
template<typename DataType>
struct iteration_state_t
{
  using transformation_t = Eigen::Matrix<DataType, 4, 4>;

  std::size_t iteration = 0;  ///< 当前迭代次数 / Current iteration number
  transformation_t transformation;  ///< 当前变换 / Current transformation
  DataType error = 0;  ///< 当前误差 / Current error
  DataType error_change = 0;  ///< 误差变化量 / Error change
  std::size_t num_correspondences =
      0;  ///< 对应点数量 / Number of correspondences
};

/**
 * @brief 细配准结果 / Fine registration result
 */
template<typename DataType>
struct fine_registration_result_t
{
  using transformation_t = Eigen::Matrix<DataType, 4, 4>;

  transformation_t transformation =
      transformation_t::Identity();  ///< 最终变换 / Final transformation
  transformation_t initial_transformation =
      transformation_t::Identity();  ///< 初始变换 / Initial transformation
  DataType final_error =
      std::numeric_limits<DataType>::max();  ///< 最终误差 / Final error
  std::size_t iterations_performed =
      0;  ///< 执行的迭代次数 / Iterations performed
  bool converged = false;  ///< 是否收敛 / Whether converged
  std::string termination_reason =
      "not started";  ///< 终止原因 / Termination reason
  std::vector<iteration_state_t<DataType>>
      history;  ///< 迭代历史（可选） / Iteration history (optional)
};

}  // namespace toolbox::pcl