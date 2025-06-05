#pragma once

#include <cpp-toolbox/pcl/registration/generalized_icp.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>

#include <algorithm>
#include <numeric>
#include <deque>
#include <mutex>

namespace toolbox::pcl
{

namespace detail {
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
} // namespace detail

template<typename DataType, typename KNNSearcher>
void generalized_icp_t<DataType, KNNSearcher>::preprocess_impl()
{
  // 构建源点云和目标点云的KD树
  if (this->m_source_cloud) {
    LOG_INFO_S << "构建源点云KD树 / Building source cloud KD-tree";
    m_source_knn_searcher->set_input(*this->m_source_cloud);
    
    LOG_INFO_S << "计算源点云协方差矩阵 / Computing source cloud covariances";
    compute_covariances(*this->m_source_cloud, *m_source_knn_searcher, m_source_covariances);
  }
  
  if (this->m_target_cloud) {
    LOG_INFO_S << "构建目标点云KD树 / Building target cloud KD-tree";
    m_target_knn_searcher->set_input(*this->m_target_cloud);
    
    LOG_INFO_S << "计算目标点云协方差矩阵 / Computing target cloud covariances";
    compute_covariances(*this->m_target_cloud, *m_target_knn_searcher, m_target_covariances);
  }
}

template<typename DataType, typename KNNSearcher>
bool generalized_icp_t<DataType, KNNSearcher>::align_impl(
    const transformation_t& initial_guess, result_type& result)
{
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
    DataType current_error = compute_error(*this->m_source_cloud, *this->m_target_cloud,
                                          correspondences, current_transform);
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
    
    // 计算新的变换
    transformation_t new_transform = compute_transformation(
        *this->m_source_cloud, *this->m_target_cloud, correspondences, current_transform);
    
    // 更新变换
    previous_transform = current_transform;
    current_transform = new_transform;
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
void generalized_icp_t<DataType, KNNSearcher>::compute_covariances(
    const point_cloud& cloud,
    knn_searcher_type& searcher,
    std::vector<Matrix3>& covariances)
{
  covariances.resize(cloud.size());
  
  // 正则化矩阵
  Matrix3 regularization = Matrix3::Identity();
  regularization(0, 0) = m_covariance_epsilon;
  regularization(1, 1) = 1.0;
  regularization(2, 2) = 1.0;
  
  // 串行计算协方差（暂时禁用并行，避免编译错误）
  for (std::size_t i = 0; i < cloud.size(); ++i) {
    std::vector<std::size_t> indices;
    std::vector<DataType> distances;
    
    // 查找k个最近邻
    searcher.kneighbors(cloud.points[i], m_k_correspondences + 1, indices, distances);
    
    if (indices.size() <= 1) {
      // 如果没有足够的邻居，使用单位矩阵
      covariances[i] = regularization;
      continue;
    }
    
    // 计算邻域点的质心
    Vector3 centroid = Vector3::Zero();
    for (std::size_t j = 1; j < indices.size(); ++j) {  // 跳过自己
      const auto& p = cloud.points[indices[j]];
      centroid += Vector3(p.x, p.y, p.z);
    }
    centroid /= static_cast<DataType>(indices.size() - 1);
    
    // 计算协方差矩阵
    Matrix3 cov = Matrix3::Zero();
    for (std::size_t j = 1; j < indices.size(); ++j) {
      const auto& p = cloud.points[indices[j]];
      Vector3 diff(p.x - centroid[0], p.y - centroid[1], p.z - centroid[2]);
      cov += diff * diff.transpose();
    }
    cov /= static_cast<DataType>(indices.size() - 1);
    
    // 应用正则化
    covariances[i] = cov + regularization;
  }
}

template<typename DataType, typename KNNSearcher>
void generalized_icp_t<DataType, KNNSearcher>::find_correspondences(
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
    m_target_knn_searcher->kneighbors(point, 1, indices, dists);
    
    if (!indices.empty() && dists[0] <= this->m_max_correspondence_distance * this->m_max_correspondence_distance) {
      correspondences.emplace_back(i, indices[0]);
      distances.push_back(std::sqrt(dists[0]));
    }
  }
}

template<typename DataType, typename KNNSearcher>
typename generalized_icp_t<DataType, KNNSearcher>::transformation_t
generalized_icp_t<DataType, KNNSearcher>::compute_transformation(
    const point_cloud& source,
    const point_cloud& target,
    const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    const transformation_t& current_transform)
{
  // 使用L-BFGS优化求解变换
  Vector6 x0 = Vector6::Zero();  // 从零开始优化增量
  Vector6 x_opt = lbfgs_optimize(x0, source, target, correspondences, current_transform);
  
  // 将优化结果转换为变换矩阵
  transformation_t delta_transform = vector_to_transformation(x_opt);
  return delta_transform * current_transform;
}

template<typename DataType, typename KNNSearcher>
DataType generalized_icp_t<DataType, KNNSearcher>::compute_mahalanobis_distance(
    const Vector3& p_src,
    const Vector3& p_tgt,
    const Matrix3& C_src,
    const Matrix3& C_tgt,
    const transformation_t& transform,
    Vector6* gradient) const
{
  // 变换源点
  Eigen::Vector4<DataType> p_src_h(p_src[0], p_src[1], p_src[2], 1.0);
  Eigen::Vector4<DataType> p_src_transformed_h = transform * p_src_h;
  Vector3 p_src_transformed = p_src_transformed_h.template head<3>();
  
  // 变换源点的协方差
  Matrix3 R = transform.template block<3, 3>(0, 0);
  Matrix3 C_src_transformed = R * C_src * R.transpose();
  
  // 计算组合协方差的逆
  Matrix3 C_combined = C_src_transformed + C_tgt;
  Matrix3 C_inv = C_combined.inverse();
  
  // 计算残差
  Vector3 residual = p_src_transformed - p_tgt;
  
  // 计算Mahalanobis距离
  DataType distance = residual.transpose() * C_inv * residual;
  
  // 如果需要计算梯度
  if (gradient) {
    // 计算对平移的梯度
    gradient->template head<3>() = 2.0 * C_inv * residual;
    
    // 计算对旋转的梯度（使用李代数）
    Matrix3 dR_dx = -detail::skew_symmetric(p_src_transformed);
    gradient->template tail<3>() = 2.0 * dR_dx.transpose() * C_inv * residual;
  }
  
  return distance;
}

template<typename DataType, typename KNNSearcher>
DataType generalized_icp_t<DataType, KNNSearcher>::compute_error(
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
    const auto& p_src = source.points[src_idx];
    const auto& p_tgt = target.points[tgt_idx];
    
    Vector3 src_vec(p_src.x, p_src.y, p_src.z);
    Vector3 tgt_vec(p_tgt.x, p_tgt.y, p_tgt.z);
    
    DataType distance = compute_mahalanobis_distance(
        src_vec, tgt_vec, m_source_covariances[src_idx], 
        m_target_covariances[tgt_idx], transform);
    
    sum_error += distance;
  }
  
  return sum_error / static_cast<DataType>(correspondences.size());
}

template<typename DataType, typename KNNSearcher>
void generalized_icp_t<DataType, KNNSearcher>::reject_outliers(
    std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    std::vector<DataType>& distances)
{
  if (correspondences.empty() || m_outlier_rejection_ratio <= 0) {
    return;
  }
  
  // 计算Mahalanobis距离用于异常值剔除
  std::vector<DataType> mahalanobis_distances;
  mahalanobis_distances.reserve(correspondences.size());
  
  transformation_t identity = transformation_t::Identity();
  
  for (const auto& [src_idx, tgt_idx] : correspondences) {
    const auto& p_src = this->m_source_cloud->points[src_idx];
    const auto& p_tgt = this->m_target_cloud->points[tgt_idx];
    
    Vector3 src_vec(p_src.x, p_src.y, p_src.z);
    Vector3 tgt_vec(p_tgt.x, p_tgt.y, p_tgt.z);
    
    DataType distance = compute_mahalanobis_distance(
        src_vec, tgt_vec, m_source_covariances[src_idx], 
        m_target_covariances[tgt_idx], identity);
    
    mahalanobis_distances.push_back(distance);
  }
  
  // 创建索引数组用于排序
  std::vector<std::size_t> indices(mahalanobis_distances.size());
  std::iota(indices.begin(), indices.end(), 0);
  
  // 按距离排序索引
  std::sort(indices.begin(), indices.end(),
            [&mahalanobis_distances](std::size_t a, std::size_t b) {
              return mahalanobis_distances[a] < mahalanobis_distances[b];
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
DataType generalized_icp_t<DataType, KNNSearcher>::objective_function(
    const Vector6& x,
    const point_cloud& source,
    const point_cloud& target,
    const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    const transformation_t& base_transform,
    Vector6* gradient) const
{
  // 将优化向量转换为变换矩阵
  transformation_t delta_transform = vector_to_transformation(x);
  transformation_t transform = delta_transform * base_transform;
  
  DataType total_cost = 0;
  Vector6 total_gradient = Vector6::Zero();
  
  for (const auto& [src_idx, tgt_idx] : correspondences) {
    const auto& p_src = source.points[src_idx];
    const auto& p_tgt = target.points[tgt_idx];
    
    Vector3 src_vec(p_src.x, p_src.y, p_src.z);
    Vector3 tgt_vec(p_tgt.x, p_tgt.y, p_tgt.z);
    
    Vector6 point_gradient;
    DataType distance = compute_mahalanobis_distance(
        src_vec, tgt_vec, m_source_covariances[src_idx], 
        m_target_covariances[tgt_idx], transform,
        gradient ? &point_gradient : nullptr);
    
    total_cost += distance;
    if (gradient) {
      total_gradient += point_gradient;
    }
  }
  
  if (gradient) {
    *gradient = total_gradient / static_cast<DataType>(correspondences.size());
  }
  
  return total_cost / static_cast<DataType>(correspondences.size());
}

template<typename DataType, typename KNNSearcher>
typename generalized_icp_t<DataType, KNNSearcher>::Vector6
generalized_icp_t<DataType, KNNSearcher>::lbfgs_optimize(
    const Vector6& x0,
    const point_cloud& source,
    const point_cloud& target,
    const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    const transformation_t& base_transform)
{
  // 简化的L-BFGS实现
  const std::size_t m = 5;  // 历史记录数量
  const DataType alpha_init = 1.0;
  const DataType c1 = 1e-4;  // Wolfe条件参数
  const DataType c2 = 0.9;
  
  Vector6 x = x0;
  Vector6 g;
  DataType f = objective_function(x, source, target, correspondences, base_transform, &g);
  
  std::deque<Vector6> s_history;
  std::deque<Vector6> y_history;
  std::deque<DataType> rho_history;
  
  for (std::size_t iter = 0; iter < m_optimizer_max_iterations; ++iter) {
    // 计算搜索方向（L-BFGS two-loop recursion）
    Vector6 q = g;
    std::vector<DataType> alphas(s_history.size());
    
    // 第一个循环
    for (int i = static_cast<int>(s_history.size()) - 1; i >= 0; --i) {
      alphas[i] = rho_history[i] * s_history[i].dot(q);
      q -= alphas[i] * y_history[i];
    }
    
    // 初始Hessian近似
    Vector6 r = q;
    if (!s_history.empty()) {
      DataType gamma = s_history.back().dot(y_history.back()) / 
                       y_history.back().dot(y_history.back());
      r *= gamma;
    }
    
    // 第二个循环
    for (std::size_t i = 0; i < s_history.size(); ++i) {
      DataType beta = rho_history[i] * y_history[i].dot(r);
      r += s_history[i] * (alphas[i] - beta);
    }
    
    Vector6 p = -r;  // 搜索方向
    
    // 线搜索
    DataType alpha = alpha_init;
    Vector6 x_new = x + alpha * p;
    Vector6 g_new;
    DataType f_new = objective_function(x_new, source, target, correspondences, base_transform, &g_new);
    
    // 简单的回溯线搜索
    while (f_new > f + c1 * alpha * g.dot(p)) {
      alpha *= 0.5;
      if (alpha < 1e-10) break;
      x_new = x + alpha * p;
      f_new = objective_function(x_new, source, target, correspondences, base_transform, &g_new);
    }
    
    // 更新历史
    Vector6 s = x_new - x;
    Vector6 y = g_new - g;
    DataType rho = 1.0 / y.dot(s);
    
    if (std::isfinite(rho) && rho > 0) {
      s_history.push_back(s);
      y_history.push_back(y);
      rho_history.push_back(rho);
      
      if (s_history.size() > m) {
        s_history.pop_front();
        y_history.pop_front();
        rho_history.pop_front();
      }
    }
    
    // 检查收敛
    if (g_new.norm() < 1e-6 || std::abs(f_new - f) < 1e-8) {
      break;
    }
    
    // 更新状态
    x = x_new;
    f = f_new;
    g = g_new;
  }
  
  return x;
}

template<typename DataType, typename KNNSearcher>
typename generalized_icp_t<DataType, KNNSearcher>::Vector6
generalized_icp_t<DataType, KNNSearcher>::transformation_to_vector(
    const transformation_t& transform) const
{
  Vector6 vec;
  
  // 提取平移部分
  vec.template head<3>() = transform.template block<3, 1>(0, 3);
  
  // 提取旋转部分（使用旋转矩阵的对数映射）
  Matrix3 R = transform.template block<3, 3>(0, 0);
  DataType trace = R.trace();
  
  if (trace > 3.0 - 1e-6) {
    // 接近单位矩阵
    vec.template tail<3>() = Vector3::Zero();
  } else if (trace < -1.0 + 1e-6) {
    // 接近180度旋转
    Vector3 axis;
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
typename generalized_icp_t<DataType, KNNSearcher>::transformation_t
generalized_icp_t<DataType, KNNSearcher>::vector_to_transformation(
    const Vector6& vec) const
{
  transformation_t transform = transformation_t::Identity();
  
  // 设置平移部分
  transform.template block<3, 1>(0, 3) = vec.template head<3>();
  
  // 设置旋转部分（使用罗德里格斯公式）
  Vector3 omega = vec.template tail<3>();
  DataType theta = omega.norm();
  
  if (theta < 1e-6) {
    // 小角度近似
    transform.template block<3, 3>(0, 0) = Matrix3::Identity() + detail::skew_symmetric(omega);
  } else {
    // 罗德里格斯公式
    Vector3 axis = omega / theta;
    Matrix3 K = detail::skew_symmetric(axis);
    
    transform.template block<3, 3>(0, 0) = 
        Matrix3::Identity() + 
        std::sin(theta) * K + 
        (1 - std::cos(theta)) * K * K;
  }
  
  return transform;
}


}  // namespace toolbox::pcl