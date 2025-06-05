#pragma once

#include <cpp-toolbox/pcl/registration/point_to_plane_icp.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>

#include <algorithm>
#include <numeric>
#include <mutex>

namespace toolbox::pcl
{

namespace detail_p2pl {
// 辅助函数：创建反对称矩阵 / Helper function: create skew-symmetric matrix
template<typename DataType>
inline Eigen::Matrix<DataType, 3, 3> skew_symmetric(const Eigen::Matrix<DataType, 3, 1>& v)
{
  Eigen::Matrix<DataType, 3, 3> S;
  S <<     0, -v[2],  v[1],
        v[2],     0, -v[0],
       -v[1],  v[0],     0;
  return S;
}
} // namespace detail_p2pl

template<typename DataType, typename KNNSearcher>
bool point_to_plane_icp_t<DataType, KNNSearcher>::align_impl(
    const transformation_t& initial_guess, result_type& result)
{
  // 检查目标点云是否有法线
  if (this->m_target_cloud->normals.empty()) {
    LOG_ERROR_S << "目标点云缺少法线数据，Point-to-Plane ICP需要法线信息 / "
                   "Target cloud missing normals, Point-to-Plane ICP requires normal information";
    result.termination_reason = "missing normals";
    return false;
  }
  
  if (this->m_target_cloud->normals.size() != this->m_target_cloud->points.size()) {
    LOG_ERROR_S << "目标点云的法线数量与点数量不匹配 / "
                   "Number of normals doesn't match number of points in target cloud";
    result.termination_reason = "normal count mismatch";
    return false;
  }
  
  // 初始化
  transformation_t current_transform = initial_guess;
  transformation_t previous_transform = current_transform;
  
  DataType previous_error = std::numeric_limits<DataType>::max();
  bool converged = false;
  
  // 创建源点云的副本用于变换
  point_cloud transformed_source = *this->m_source_cloud;
  
  // 迭代优化
  for (std::size_t iter = 0; iter < this->m_max_iterations; ++iter) {
    // 变换源点云
    for (std::size_t i = 0; i < transformed_source.size(); ++i) {
      const auto& p = this->m_source_cloud->points[i];
      Eigen::Vector4<DataType> pt(p.x, p.y, p.z, 1.0);
      Eigen::Vector4<DataType> transformed_pt = current_transform * pt;
      transformed_source.points[i].x = transformed_pt[0];
      transformed_source.points[i].y = transformed_pt[1];
      transformed_source.points[i].z = transformed_pt[2];
    }
    
    // 查找对应关系
    std::vector<std::pair<std::size_t, std::size_t>> correspondences;
    std::vector<DataType> distances;
    find_correspondences(transformed_source, correspondences, distances);
    
    if (correspondences.empty()) {
      LOG_ERROR_S << "未找到有效的对应关系 / No valid correspondences found";
      result.termination_reason = "no correspondences";
      return false;
    }
    
    // 剔除异常值
    if (m_outlier_rejection_ratio > 0) {
      reject_outliers(correspondences, distances);
    }
    
    // 计算当前误差
    DataType current_error = compute_error(transformed_source, *this->m_target_cloud,
                                          correspondences, transformation_t::Identity());
    DataType error_change = std::abs(current_error - previous_error);
    
    // 记录迭代状态
    this->record_iteration(result, iter, current_transform, current_error, 
                          error_change, correspondences.size());
    
    // 检查收敛
    std::string termination_reason;
    if (this->has_converged(iter, current_transform, previous_transform,
                           current_error, previous_error, termination_reason)) {
      converged = true;
      result.termination_reason = termination_reason;
      break;
    }
    
    // 计算新的变换（相对于已变换的源点云）
    transformation_t delta_transform = compute_transformation(
        transformed_source, *this->m_target_cloud, correspondences);
    
    // 更新变换
    previous_transform = current_transform;
    current_transform = delta_transform * current_transform;
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

template<typename DataType, typename KNNSearcher>
void point_to_plane_icp_t<DataType, KNNSearcher>::find_correspondences(
    const point_cloud& transformed_source,
    std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    std::vector<DataType>& distances)
{
  correspondences.clear();
  distances.clear();
  correspondences.reserve(transformed_source.size());
  distances.reserve(transformed_source.size());
  
  // 串行查找对应关系（暂时禁用并行，避免编译错误）
  std::vector<std::size_t> indices;
  std::vector<DataType> dists;
  
  for (std::size_t i = 0; i < transformed_source.size(); ++i) {
    const auto& point = transformed_source.points[i];
    m_knn_searcher->kneighbors(point, 1, indices, dists);
    
    if (!indices.empty() && dists[0] <= this->m_max_correspondence_distance * this->m_max_correspondence_distance) {
      correspondences.emplace_back(i, indices[0]);
      distances.push_back(std::sqrt(dists[0]));
    }
  }
}

template<typename DataType, typename KNNSearcher>
typename point_to_plane_icp_t<DataType, KNNSearcher>::transformation_t
point_to_plane_icp_t<DataType, KNNSearcher>::compute_transformation(
    const point_cloud& source,
    const point_cloud& target,
    const std::vector<std::pair<std::size_t, std::size_t>>& correspondences)
{
  using Vector3 = Eigen::Matrix<DataType, 3, 1>;
  using Vector6 = Eigen::Matrix<DataType, 6, 1>;
  using Matrix6 = Eigen::Matrix<DataType, 6, 6>;
  using Matrix3x6 = Eigen::Matrix<DataType, 3, 6>;
  
  // 构建线性系统 A * x = b
  Matrix6 A = Matrix6::Zero();
  Vector6 b = Vector6::Zero();
  
  // 添加正则化项
  A.diagonal() = Vector6::Constant(m_regularization);
  
  for (const auto& [src_idx, tgt_idx] : correspondences) {
    const auto& src_pt = source.points[src_idx];
    const auto& tgt_pt = target.points[tgt_idx];
    
    // 获取目标点的法线
    if (tgt_idx >= this->m_target_cloud->normals.size()) {
      LOG_ERROR_S << "目标点云缺少法线数据 / Target cloud missing normal data";
      continue;
    }
    const auto& normal_pt = this->m_target_cloud->normals[tgt_idx];
    
    Vector3 p_src(src_pt.x, src_pt.y, src_pt.z);
    Vector3 p_tgt(tgt_pt.x, tgt_pt.y, tgt_pt.z);
    Vector3 n(normal_pt.x, normal_pt.y, normal_pt.z);
    
    // 确保法线归一化
    n.normalize();
    
    // 计算残差：点到平面的距离
    DataType residual = n.dot(p_src - p_tgt);
    
    // 构建Jacobian矩阵的一行
    // J = [n^T, (p_src x n)^T]
    Matrix3x6 J = Matrix3x6::Zero();
    J.template block<3, 3>(0, 0) = Eigen::Matrix<DataType, 3, 3>::Identity();
    J.template block<3, 3>(0, 3) = -detail_p2pl::skew_symmetric(p_src);
    
    Eigen::Matrix<DataType, 1, 6> J_row = n.transpose() * J;
    
    // 累加到线性系统
    A += J_row.transpose() * J_row;
    b -= J_row.transpose() * residual;
  }
  
  // 求解线性系统
  Vector6 x = A.ldlt().solve(b);
  
  // 将解转换为变换矩阵
  return vector_to_transformation(x);
}

template<typename DataType, typename KNNSearcher>
DataType point_to_plane_icp_t<DataType, KNNSearcher>::compute_error(
    const point_cloud& source,
    const point_cloud& target,
    const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    const transformation_t& transform) const
{
  if (correspondences.empty()) {
    return std::numeric_limits<DataType>::max();
  }
  
  DataType sum_error = 0;
  
  for (const auto& [src_idx, tgt_idx] : correspondences) {
    const auto& src_pt = source.points[src_idx];
    const auto& tgt_pt = target.points[tgt_idx];
    
    // 获取目标点的法线
    if (tgt_idx >= this->m_target_cloud->normals.size()) {
      LOG_ERROR_S << "目标点云缺少法线数据 / Target cloud missing normal data";
      continue;
    }
    const auto& normal_pt = this->m_target_cloud->normals[tgt_idx];
    
    // 变换源点
    Eigen::Vector4<DataType> p_src(src_pt.x, src_pt.y, src_pt.z, 1.0);
    Eigen::Vector4<DataType> p_src_transformed = transform * p_src;
    
    Eigen::Vector3<DataType> p_src_3d(p_src_transformed[0], p_src_transformed[1], p_src_transformed[2]);
    Eigen::Vector3<DataType> p_tgt_3d(tgt_pt.x, tgt_pt.y, tgt_pt.z);
    Eigen::Vector3<DataType> n(normal_pt.x, normal_pt.y, normal_pt.z);
    
    // 点到平面的距离
    DataType distance = std::abs(n.dot(p_src_3d - p_tgt_3d));
    sum_error += distance * distance;
  }
  
  return sum_error / static_cast<DataType>(correspondences.size());
}

template<typename DataType, typename KNNSearcher>
void point_to_plane_icp_t<DataType, KNNSearcher>::reject_outliers(
    std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    std::vector<DataType>& distances)
{
  if (correspondences.empty() || m_outlier_rejection_ratio <= 0) {
    return;
  }
  
  // 计算点到平面的距离用于异常值剔除
  std::vector<DataType> plane_distances;
  plane_distances.reserve(correspondences.size());
  
  for (const auto& [src_idx, tgt_idx] : correspondences) {
    const auto& src_pt = this->m_source_cloud->points[src_idx];
    const auto& tgt_pt = this->m_target_cloud->points[tgt_idx];
    // 假设法线存储为额外的点云数据，这里需要用户提供
    // TODO: 实现更好的法线访问机制
    normal_type normal;
    normal.normal_x = 0;
    normal.normal_y = 0;
    normal.normal_z = 1;
    normal.curvature = 0;
    
    Eigen::Vector3<DataType> p_src(src_pt.x, src_pt.y, src_pt.z);
    Eigen::Vector3<DataType> p_tgt(tgt_pt.x, tgt_pt.y, tgt_pt.z);
    Eigen::Vector3<DataType> n(normal.normal_x, normal.normal_y, normal.normal_z);
    
    DataType distance = std::abs(n.dot(p_src - p_tgt));
    plane_distances.push_back(distance);
  }
  
  // 创建索引数组用于排序
  std::vector<std::size_t> indices(plane_distances.size());
  std::iota(indices.begin(), indices.end(), 0);
  
  // 按距离排序索引
  std::sort(indices.begin(), indices.end(),
            [&plane_distances](std::size_t a, std::size_t b) {
              return plane_distances[a] < plane_distances[b];
            });
  
  // 计算保留的对应关系数量
  std::size_t num_to_keep = static_cast<std::size_t>(
      correspondences.size() * (1.0 - m_outlier_rejection_ratio));
  num_to_keep = std::max(num_to_keep, std::size_t(1));
  
  // 创建新的对应关系和距离
  std::vector<std::pair<std::size_t, std::size_t>> new_correspondences;
  std::vector<DataType> new_distances;
  new_correspondences.reserve(num_to_keep);
  new_distances.reserve(num_to_keep);
  
  for (std::size_t i = 0; i < num_to_keep; ++i) {
    std::size_t idx = indices[i];
    new_correspondences.push_back(correspondences[idx]);
    new_distances.push_back(distances[idx]);
  }
  
  correspondences = std::move(new_correspondences);
  distances = std::move(new_distances);
}

template<typename DataType, typename KNNSearcher>
Eigen::Matrix<DataType, 6, 1> point_to_plane_icp_t<DataType, KNNSearcher>::transformation_to_vector(
    const transformation_t& transform) const
{
  Eigen::Matrix<DataType, 6, 1> vec;
  
  // 提取平移部分
  vec.template head<3>() = transform.template block<3, 1>(0, 3);
  
  // 提取旋转部分（使用旋转矩阵的对数映射）
  Eigen::Matrix<DataType, 3, 3> R = transform.template block<3, 3>(0, 0);
  DataType trace = R.trace();
  
  if (trace > 3.0 - 1e-6) {
    // 接近单位矩阵
    vec.template tail<3>() = Eigen::Matrix<DataType, 3, 1>::Zero();
  } else if (trace < -1.0 + 1e-6) {
    // 接近180度旋转
    Eigen::Matrix<DataType, 3, 1> axis;
    int i = 0;
    if (R(1, 1) > R(0, 0)) i = 1;
    if (R(2, 2) > R(i, i)) i = 2;
    
    axis[i] = std::sqrt((R(i, i) - R((i+1)%3, (i+1)%3) - R((i+2)%3, (i+2)%3) + 1.0) / 2.0);
    axis[(i+1)%3] = R(i, (i+1)%3) / (2.0 * axis[i]);
    axis[(i+2)%3] = R(i, (i+2)%3) / (2.0 * axis[i]);
    
    vec.template tail<3>() = M_PI * axis;
  } else {
    // 一般情况
    DataType theta = std::acos((trace - 1.0) / 2.0);
    DataType factor = theta / (2.0 * std::sin(theta));
    
    vec[3] = factor * (R(2, 1) - R(1, 2));
    vec[4] = factor * (R(0, 2) - R(2, 0));
    vec[5] = factor * (R(1, 0) - R(0, 1));
  }
  
  return vec;
}

template<typename DataType, typename KNNSearcher>
typename point_to_plane_icp_t<DataType, KNNSearcher>::transformation_t
point_to_plane_icp_t<DataType, KNNSearcher>::vector_to_transformation(
    const Eigen::Matrix<DataType, 6, 1>& vec) const
{
  transformation_t transform = transformation_t::Identity();
  
  // 设置平移部分
  transform.template block<3, 1>(0, 3) = vec.template head<3>();
  
  // 设置旋转部分（使用罗德里格斯公式）
  Eigen::Matrix<DataType, 3, 1> omega = vec.template tail<3>();
  DataType theta = omega.norm();
  
  if (theta < 1e-6) {
    // 小角度近似
    transform.template block<3, 3>(0, 0) = Eigen::Matrix<DataType, 3, 3>::Identity() + detail_p2pl::skew_symmetric(omega);
  } else {
    // 罗德里格斯公式
    Eigen::Matrix<DataType, 3, 1> axis = omega / theta;
    Eigen::Matrix<DataType, 3, 3> K = detail_p2pl::skew_symmetric(axis);
    
    transform.template block<3, 3>(0, 0) = 
        Eigen::Matrix<DataType, 3, 3>::Identity() + 
        std::sin(theta) * K + 
        (1 - std::cos(theta)) * K * K;
  }
  
  return transform;
}



}  // namespace toolbox::pcl