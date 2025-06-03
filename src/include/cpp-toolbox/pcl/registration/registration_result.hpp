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

}  // namespace toolbox::pcl