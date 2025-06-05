#pragma once

#include <cpp-toolbox/pcl/registration/ndt.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>

#include <algorithm>
#include <numeric>
#include <cmath>
#include <mutex>

namespace toolbox::pcl
{

template<typename DataType>
void ndt_t<DataType>::preprocess_impl()
{
  if (this->m_target_cloud && (this->m_target_updated || m_voxel_grid_updated)) {
    LOG_INFO_S << "构建目标点云的NDT体素网格 / Building NDT voxel grid for target cloud";
    build_voxel_grid();
    m_voxel_grid_updated = false;
  }
  
  // 计算高斯相关常数
  m_gauss_d1 = -std::log(2 * M_PI) / 2.0;
  m_gauss_d2 = -std::log(m_outlier_ratio * std::sqrt(2 * M_PI)) - 0.5;
}

template<typename DataType>
bool ndt_t<DataType>::align_impl(
    const transformation_t& initial_guess, result_type& result)
{
  // 初始化
  transformation_t current_transform = initial_guess;
  transformation_t previous_transform = current_transform;
  
  DataType previous_error = std::numeric_limits<DataType>::max();
  bool converged = false;
  
  // More-Thuente线搜索参数
  DataType step_size = m_step_size;
  
  // 迭代优化
  for (std::size_t iter = 0; iter < this->m_max_iterations; ++iter) {
    // 计算目标函数值、梯度和Hessian
    Vector6 gradient;
    Matrix6 hessian;
    DataType current_score = compute_objective(current_transform, &gradient, &hessian);
    
    // 转换为最小化问题（NDT最大化似然，所以取负）
    DataType current_error = -current_score;
    DataType error_change = std::abs(current_error - previous_error);
    
    // 记录迭代状态
    this->record_iteration(result, iter, current_transform, current_error, 
                          error_change, m_voxel_grid.size());
    
    // 检查收敛
    std::string termination_reason;
    if (this->has_converged(iter, current_transform, previous_transform,
                           current_error, previous_error, termination_reason)) {
      converged = true;
      result.termination_reason = termination_reason;
      break;
    }
    
    // 检查梯度范数
    if (gradient.norm() < 1e-6) {
      converged = true;
      result.termination_reason = "gradient converged";
      break;
    }
    
    // 求解牛顿方向：H * delta = -g
    Vector6 delta = hessian.ldlt().solve(-gradient);
    
    // 线搜索找到合适的步长
    DataType optimal_step = line_search(current_transform, delta, step_size);
    
    if (optimal_step < 1e-10) {
      converged = true;
      result.termination_reason = "line search failed";
      break;
    }
    
    // 更新变换
    Vector6 update = optimal_step * delta;
    transformation_t delta_transform = vector_to_transformation(update);
    
    previous_transform = current_transform;
    current_transform = delta_transform * current_transform;
    previous_error = current_error;
    
    // 自适应调整步长
    if (optimal_step > 0.8 * step_size) {
      step_size = std::min(1.0, step_size * 1.5);
    } else if (optimal_step < 0.1 * step_size) {
      step_size = std::max(0.001, step_size * 0.5);
    }
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

template<typename DataType>
void ndt_t<DataType>::build_voxel_grid()
{
  m_voxel_grid.clear();
  
  if (!this->m_target_cloud || this->m_target_cloud->empty()) {
    return;
  }
  
  // 第一遍：将点分配到体素
  std::unordered_map<std::size_t, std::vector<std::size_t>> voxel_indices;
  
  for (std::size_t i = 0; i < this->m_target_cloud->size(); ++i) {
    const auto& point = this->m_target_cloud->points[i];
    Vector3 p(point.x, point.y, point.z);
    
    auto voxel_idx = compute_voxel_index(p);
    std::size_t key = get_voxel_key(voxel_idx);
    
    voxel_indices[key].push_back(i);
  }
  
  // 第二遍：计算每个体素的统计信息（暂时禁用并行，避免编译错误）
  for (const auto& [key, indices] : voxel_indices) {
    if (indices.size() < 5) {
      continue;  // 需要至少5个点来计算有效的协方差
    }
    
    voxel_cell_t& cell = m_voxel_grid[key];
    cell.num_points = indices.size();
    
    // 计算均值
    for (std::size_t idx : indices) {
      const auto& p = this->m_target_cloud->points[idx];
      cell.mean += Vector3(p.x, p.y, p.z);
    }
    cell.mean /= static_cast<DataType>(cell.num_points);
    
    // 计算协方差
    for (std::size_t idx : indices) {
      const auto& p = this->m_target_cloud->points[idx];
      Vector3 diff = Vector3(p.x, p.y, p.z) - cell.mean;
      cell.covariance += diff * diff.transpose();
    }
    cell.covariance /= static_cast<DataType>(cell.num_points - 1);
    
    // 正则化协方差矩阵，避免奇异
    Matrix3 reg = Matrix3::Identity() * 0.01 * m_resolution * m_resolution;
    cell.covariance += reg;
    
    // 计算协方差的逆
    cell.covariance_inv = cell.covariance.inverse();
    cell.valid = true;
  }
  
  LOG_INFO_S << "构建了 " << m_voxel_grid.size() << " 个有效体素 / "
                "Built " << m_voxel_grid.size() << " valid voxels";
}

template<typename DataType>
std::array<int, 3> ndt_t<DataType>::compute_voxel_index(const Vector3& point) const
{
  std::array<int, 3> index;
  index[0] = static_cast<int>(std::floor(point[0] / m_resolution));
  index[1] = static_cast<int>(std::floor(point[1] / m_resolution));
  index[2] = static_cast<int>(std::floor(point[2] / m_resolution));
  return index;
}

template<typename DataType>
std::size_t ndt_t<DataType>::get_voxel_key(const std::array<int, 3>& index) const
{
  // 使用hash组合三个索引
  std::size_t h1 = std::hash<int>()(index[0]);
  std::size_t h2 = std::hash<int>()(index[1]);
  std::size_t h3 = std::hash<int>()(index[2]);
  
  // 组合hash值
  return h1 ^ (h2 << 1) ^ (h3 << 2);
}

template<typename DataType>
DataType ndt_t<DataType>::compute_objective(
    const transformation_t& transform,
    Vector6* gradient,
    Matrix6* hessian) const
{
  if (gradient) gradient->setZero();
  if (hessian) hessian->setZero();
  
  DataType total_score = 0;
  
  // 串行版本（暂时禁用并行，避免编译错误）
  for (std::size_t i = 0; i < this->m_source_cloud->size(); ++i) {
    const auto& p = this->m_source_cloud->points[i];
    Vector3 point(p.x, p.y, p.z);
    
    total_score += compute_point_contribution(point, transform, gradient, hessian);
  }
  
  return total_score / static_cast<DataType>(this->m_source_cloud->size());
}

template<typename DataType>
DataType ndt_t<DataType>::compute_point_contribution(
    const Vector3& point,
    const transformation_t& transform,
    Vector6* gradient,
    Matrix6* hessian) const
{
  // 变换点
  Eigen::Vector4<DataType> p_h(point[0], point[1], point[2], 1.0);
  Eigen::Vector4<DataType> p_transformed_h = transform * p_h;
  Vector3 p_transformed = p_transformed_h.template head<3>();
  
  // 找到对应的体素
  auto voxel_idx = compute_voxel_index(p_transformed);
  std::size_t key = get_voxel_key(voxel_idx);
  
  auto it = m_voxel_grid.find(key);
  if (it == m_voxel_grid.end() || !it->second.valid) {
    // 没有找到有效体素，返回异常值分数
    return m_gauss_d2;
  }
  
  const voxel_cell_t& cell = it->second;
  
  // 计算到体素中心的差值
  Vector3 x_diff = p_transformed - cell.mean;
  
  // 计算指数部分
  DataType exp_arg = -0.5 * x_diff.transpose() * cell.covariance_inv * x_diff;
  
  // 防止数值下溢
  if (exp_arg < -20) {
    return m_gauss_d2;
  }
  
  DataType exp_val = std::exp(exp_arg);
  DataType score = m_gauss_d1 + exp_arg;
  
  // 如果需要计算梯度和Hessian
  if (gradient || hessian) {
    // 计算雅可比矩阵 J = d(T*p)/d(params)
    Eigen::Matrix<DataType, 3, 6> jacobian;
    compute_jacobian(p_transformed, jacobian);
    
    // 一阶导数项
    Vector3 d_exp_dx = -cell.covariance_inv * x_diff;
    
    if (gradient) {
      *gradient += jacobian.transpose() * d_exp_dx * exp_val;
    }
    
    if (hessian) {
      // 二阶导数项
      Matrix3 d2_exp_dx2 = -cell.covariance_inv;
      
      // 外积项
      Matrix3 outer = d_exp_dx * d_exp_dx.transpose();
      
      // 完整的Hessian
      Matrix3 point_hessian = (d2_exp_dx2 + outer) * exp_val;
      
      *hessian += jacobian.transpose() * point_hessian * jacobian;
    }
  }
  
  return score;
}

template<typename DataType>
DataType ndt_t<DataType>::line_search(
    const transformation_t& current_transform,
    const Vector6& direction,
    DataType initial_step_size)
{
  // More-Thuente线搜索
  DataType alpha = initial_step_size;
  DataType alpha_min = 0;
  DataType alpha_max = 1.0;
  
  // 计算初始函数值
  DataType f0 = compute_objective(current_transform);
  
  // 计算初始梯度在搜索方向上的投影
  Vector6 g0;
  compute_objective(current_transform, &g0);
  DataType dg0 = g0.dot(direction);
  
  if (dg0 >= 0) {
    // 不是下降方向
    return 0;
  }
  
  // Wolfe条件参数
  const DataType c1 = 1e-4;
  const DataType c2 = 0.9;
  
  for (std::size_t iter = 0; iter < m_line_search_max_iterations; ++iter) {
    // 计算新的变换
    Vector6 update = alpha * direction;
    transformation_t delta = vector_to_transformation(update);
    transformation_t new_transform = delta * current_transform;
    
    // 计算新的函数值
    DataType f_new = compute_objective(new_transform);
    
    // 检查Armijo条件
    if (f_new <= f0 + c1 * alpha * dg0) {
      // 计算新的梯度
      Vector6 g_new;
      compute_objective(new_transform, &g_new);
      DataType dg_new = g_new.dot(direction);
      
      // 检查曲率条件
      if (std::abs(dg_new) <= c2 * std::abs(dg0)) {
        return alpha;
      }
      
      if (dg_new >= 0) {
        alpha_max = alpha;
      } else {
        alpha_min = alpha;
      }
    } else {
      alpha_max = alpha;
    }
    
    // 更新步长
    if (alpha_max - alpha_min < 1e-10) {
      break;
    }
    
    alpha = 0.5 * (alpha_min + alpha_max);
  }
  
  return alpha;
}

template<typename DataType>
typename ndt_t<DataType>::transformation_t
ndt_t<DataType>::vector_to_transformation(const Vector6& vec) const
{
  transformation_t transform = transformation_t::Identity();
  
  // 平移部分
  transform(0, 3) = vec[0];
  transform(1, 3) = vec[1];
  transform(2, 3) = vec[2];
  
  // 旋转部分（使用ZYX欧拉角）
  DataType roll = vec[3];
  DataType pitch = vec[4];
  DataType yaw = vec[5];
  
  DataType sr = std::sin(roll);
  DataType cr = std::cos(roll);
  DataType sp = std::sin(pitch);
  DataType cp = std::cos(pitch);
  DataType sy = std::sin(yaw);
  DataType cy = std::cos(yaw);
  
  transform(0, 0) = cy * cp;
  transform(0, 1) = cy * sp * sr - sy * cr;
  transform(0, 2) = cy * sp * cr + sy * sr;
  
  transform(1, 0) = sy * cp;
  transform(1, 1) = sy * sp * sr + cy * cr;
  transform(1, 2) = sy * sp * cr - cy * sr;
  
  transform(2, 0) = -sp;
  transform(2, 1) = cp * sr;
  transform(2, 2) = cp * cr;
  
  return transform;
}

template<typename DataType>
DataType ndt_t<DataType>::gaussian_d1(
    DataType x_bar, DataType x, const Matrix3& c_inv) const
{
  DataType d = x_bar - x;
  DataType exp_arg = -0.5 * d * d * c_inv(0, 0);
  return std::exp(exp_arg);
}

template<typename DataType>
void ndt_t<DataType>::compute_jacobian(
    const Vector3& point,
    Eigen::Matrix<DataType, 3, 6>& jacobian) const
{
  // 雅可比矩阵: J = [I, -[p]×]
  // 其中 [p]× 是点p的反对称矩阵
  
  jacobian.setZero();
  
  // 对平移的导数是单位矩阵
  jacobian(0, 0) = 1.0;
  jacobian(1, 1) = 1.0;
  jacobian(2, 2) = 1.0;
  
  // 对旋转的导数是 -[p]×
  jacobian(0, 3) = 0;
  jacobian(0, 4) = -point[2];
  jacobian(0, 5) = point[1];
  
  jacobian(1, 3) = point[2];
  jacobian(1, 4) = 0;
  jacobian(1, 5) = -point[0];
  
  jacobian(2, 3) = -point[1];
  jacobian(2, 4) = point[0];
  jacobian(2, 5) = 0;
}

}  // namespace toolbox::pcl