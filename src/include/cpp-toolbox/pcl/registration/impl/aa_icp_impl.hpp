#pragma once

#include <cpp-toolbox/pcl/registration/aa_icp.hpp>

namespace toolbox::pcl
{

template<typename DataType, typename KNNSearcher, typename BaseICP>
bool aa_icp_t<DataType, KNNSearcher, BaseICP>::align_impl(
    const transformation_t& initial_guess, result_type& result)
{
  // 初始化
  transformation_t current_transform = initial_guess;
  transformation_t previous_transform = current_transform;
  
  DataType previous_error = std::numeric_limits<DataType>::max();
  bool converged = false;
  
  // 清空历史记录
  m_g_history.clear();
  m_x_history.clear();
  m_initialized = false;
  
  // 传递基类参数到基础ICP，并设置点云
  m_base_icp.set_source(this->m_source_cloud);
  m_base_icp.set_target(this->m_target_cloud);
  m_base_icp.set_max_iterations(10);  // 设置更多迭代次数
  m_base_icp.set_max_correspondence_distance(this->m_max_correspondence_distance);
  m_base_icp.set_transformation_epsilon(this->m_transformation_epsilon);
  m_base_icp.set_euclidean_fitness_epsilon(this->m_euclidean_fitness_epsilon);
  
  // 迭代优化
  for (std::size_t iter = 0; iter < this->m_max_iterations; ++iter) {
    // 执行一步基础ICP
    DataType current_error;
    std::size_t num_correspondences;
    transformation_t icp_transform = perform_base_icp_step(
        current_transform, current_error, num_correspondences);
    
    if (num_correspondences == 0) {
      LOG_ERROR_S << "未找到有效的对应关系 / No valid correspondences found";
      result.termination_reason = "no correspondences";
      return false;
    }
    
    // 转换为向量表示
    VectorX x_current = transformation_to_vector(current_transform);
    VectorX x_icp = transformation_to_vector(icp_transform);
    
    // 计算定点迭代的残差 g(x) = f(x) - x
    VectorX g_current = x_icp - x_current;
    
    // 更新历史记录
    m_x_history.push_back(x_current);
    m_g_history.push_back(g_current);
    
    // 限制历史记录大小
    if (m_x_history.size() > m_anderson_m + 1) {
      m_x_history.pop_front();
      m_g_history.pop_front();
    }
    
    // 计算下一次迭代的变换
    VectorX x_next;
    
    if (m_x_history.size() <= 1 || iter < 2) {
      // 前几次迭代使用标准ICP更新
      x_next = x_icp;
    } else {
      // 使用Anderson加速
      x_next = anderson_acceleration_update(m_g_history, m_x_history);
      
      // 安全保护：检查Anderson加速的结果
      if (m_enable_safeguarding) {
        if (!is_numerically_stable(x_next)) {
          LOG_WARN_S << "Anderson加速结果不稳定，回退到标准ICP / "
                        "Anderson acceleration unstable, falling back to standard ICP";
          x_next = x_icp;
          
          // 清空历史，重新开始
          m_x_history.clear();
          m_g_history.clear();
          m_x_history.push_back(x_current);
          m_g_history.push_back(g_current);
        } else {
          // 计算新变换对应的误差
          transformation_t test_transform = vector_to_transformation(x_next);
          DataType test_error;
          std::size_t test_corr;
          perform_base_icp_step(test_transform, test_error, test_corr);
          
          // 如果误差增加太多，使用阻尼
          if (test_error > current_error * (1.0 + 0.1)) {
            x_next = m_beta * x_next + (1.0 - m_beta) * x_icp;
          }
        }
      }
    }
    
    // 更新变换
    transformation_t next_transform = vector_to_transformation(x_next);
    
    // 记录迭代状态
    DataType error_change = std::abs(current_error - previous_error);
    this->record_iteration(result, iter, next_transform, current_error, 
                          error_change, num_correspondences);
    
    // 检查收敛
    std::string termination_reason;
    if (this->has_converged(iter, next_transform, current_transform,
                           current_error, previous_error, termination_reason)) {
      converged = true;
      result.termination_reason = termination_reason;
      break;
    }
    
    // 更新状态
    previous_transform = current_transform;
    current_transform = next_transform;
    previous_error = current_error;
  }
  
  // 设置结果
  result.transformation = current_transform;
  result.converged = converged;
  result.iterations_performed = result.history.size();
  result.final_error = previous_error;
  
  if (!converged && result.termination_reason.empty()) {
    result.termination_reason = "maximum iterations reached";
  }
  
  return true;
}

template<typename DataType, typename KNNSearcher, typename BaseICP>
typename aa_icp_t<DataType, KNNSearcher, BaseICP>::VectorX
aa_icp_t<DataType, KNNSearcher, BaseICP>::transformation_to_vector(
    const transformation_t& transform) const
{
  VectorX vec(12);
  
  // 将4x4矩阵的前3行展平为向量
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 4; ++j) {
      vec[i * 4 + j] = transform(i, j);
    }
  }
  
  return vec;
}

template<typename DataType, typename KNNSearcher, typename BaseICP>
typename aa_icp_t<DataType, KNNSearcher, BaseICP>::transformation_t
aa_icp_t<DataType, KNNSearcher, BaseICP>::vector_to_transformation(
    const VectorX& vec) const
{
  transformation_t transform = transformation_t::Identity();
  
  // 从向量恢复4x4矩阵的前3行
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 4; ++j) {
      transform(i, j) = vec[i * 4 + j];
    }
  }
  
  // 正交化旋转矩阵部分（使用SVD）
  Eigen::Matrix<DataType, 3, 3> R = transform.template block<3, 3>(0, 0);
  Eigen::JacobiSVD<Eigen::Matrix<DataType, 3, 3>> svd(R, Eigen::ComputeFullU | Eigen::ComputeFullV);
  R = svd.matrixU() * svd.matrixV().transpose();
  
  // 处理反射
  if (R.determinant() < 0) {
    Eigen::Matrix<DataType, 3, 3> V = svd.matrixV();
    V.col(2) *= -1;
    R = svd.matrixU() * V.transpose();
  }
  
  transform.template block<3, 3>(0, 0) = R;
  
  return transform;
}

template<typename DataType, typename KNNSearcher, typename BaseICP>
typename aa_icp_t<DataType, KNNSearcher, BaseICP>::transformation_t
aa_icp_t<DataType, KNNSearcher, BaseICP>::perform_base_icp_step(
    const transformation_t& current_transform,
    DataType& error,
    std::size_t& num_correspondences)
{
  // 创建临时结果对象
  result_type temp_result;
  
  LOG_DEBUG_S << "AA-ICP: 执行基础ICP步骤，源点云大小=" << this->m_source_cloud->size() 
              << "，目标点云大小=" << this->m_target_cloud->size()
              << "，最大对应距离=" << this->m_max_correspondence_distance;
  
  // 执行一步基础ICP
  m_base_icp.align(current_transform, temp_result);
  
  // 提取结果
  error = temp_result.final_error;
  num_correspondences = temp_result.history.empty() ? 0 : 
                       temp_result.history.back().num_correspondences;
  
  return temp_result.transformation;
}

template<typename DataType, typename KNNSearcher, typename BaseICP>
typename aa_icp_t<DataType, KNNSearcher, BaseICP>::VectorX
aa_icp_t<DataType, KNNSearcher, BaseICP>::anderson_acceleration_update(
    const std::deque<VectorX>& g_history,
    const std::deque<VectorX>& x_history)
{
  const std::size_t m_k = std::min(m_anderson_m, g_history.size() - 1);
  
  if (m_k == 0) {
    // 标准定点迭代
    return x_history.back() + g_history.back();
  }
  
  // 构建矩阵 G_k = [g_k - g_{k-1}, ..., g_k - g_{k-m_k}]
  MatrixX G_k(g_history.back().size(), m_k);
  
  for (std::size_t i = 0; i < m_k; ++i) {
    G_k.col(i) = g_history[g_history.size() - 1] - g_history[g_history.size() - 2 - i];
  }
  
  // 求解最小二乘问题：min ||G_k * gamma + g_k||^2
  // 使用QR分解求解
  Eigen::HouseholderQR<MatrixX> qr(G_k);
  VectorX gamma = qr.solve(-g_history.back());
  
  // 检查解的数值稳定性
  DataType gamma_norm = gamma.norm();
  if (gamma_norm > 10.0) {
    // 如果系数太大，进行截断
    gamma *= 10.0 / gamma_norm;
  }
  
  // 计算加权组合
  VectorX x_next = x_history.back() + g_history.back();
  
  for (std::size_t i = 0; i < m_k; ++i) {
    std::size_t idx = x_history.size() - 2 - i;
    x_next -= gamma[i] * (x_history[idx + 1] - x_history[idx] + 
                          g_history[idx + 1] - g_history[idx]);
  }
  
  return x_next;
}

template<typename DataType, typename KNNSearcher, typename BaseICP>
bool aa_icp_t<DataType, KNNSearcher, BaseICP>::is_numerically_stable(
    const VectorX& vec) const
{
  // 检查是否包含NaN或Inf
  if (!vec.allFinite()) {
    return false;
  }
  
  // 检查向量范数是否合理
  DataType norm = vec.norm();
  if (norm > 1e6 || norm < 1e-6) {
    return false;
  }
  
  // 检查变换矩阵的合理性
  transformation_t transform = vector_to_transformation(vec);
  
  // 旋转部分的行列式应该接近1
  DataType det = transform.template block<3, 3>(0, 0).determinant();
  if (std::abs(det - 1.0) > 0.1) {
    return false;
  }
  
  // 平移部分不应该太大
  DataType translation_norm = transform.template block<3, 1>(0, 3).norm();
  if (translation_norm > 100.0) {
    return false;
  }
  
  return true;
}

}  // namespace toolbox::pcl