#pragma once

#include <cpp-toolbox/pcl/registration/point_to_point_icp.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>

#include <algorithm>
#include <numeric>
#include <future>
#include <mutex>

namespace toolbox::pcl
{

template<typename DataType, typename KNNSearcher>
bool point_to_point_icp_t<DataType, KNNSearcher>::align_impl(
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
    DataType current_error = compute_error(distances);
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
    
    // 计算新的变换（从变换后的源到目标的变换）
    transformation_t delta_transform = compute_transformation(
        transformed_source, *this->m_target_cloud, correspondences, distances);
    
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
void point_to_point_icp_t<DataType, KNNSearcher>::find_correspondences(
    const point_cloud& transformed_source,
    std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    std::vector<DataType>& distances)
{
  correspondences.clear();
  distances.clear();
  correspondences.reserve(transformed_source.size());
  distances.reserve(transformed_source.size());
  
  if (m_enable_parallel) {
    // 并行版本 - 先收集到线程局部存储，再合并
    auto& pool = base::thread_pool_singleton_t::instance();
    const std::size_t num_threads = pool.get_thread_count();
    const std::size_t chunk_size = std::max(std::size_t(1), 
                                           transformed_source.size() / num_threads);
    
    std::vector<std::future<void>> futures;
    std::vector<std::vector<std::pair<std::size_t, std::size_t>>> thread_correspondences(num_threads);
    std::vector<std::vector<DataType>> thread_distances(num_threads);
    
    for (std::size_t t = 0; t < num_threads; ++t) {
      std::size_t start = t * chunk_size;
      std::size_t end = (t == num_threads - 1) ? transformed_source.size() : (t + 1) * chunk_size;
      
      futures.emplace_back(pool.submit([this, &transformed_source, start, end, 
                                       &thread_correspondences, &thread_distances, t]() {
        std::vector<std::size_t> indices;
        std::vector<DataType> dists;
        
        for (std::size_t i = start; i < end; ++i) {
          const auto& point = transformed_source.points[i];
          this->m_knn_searcher->kneighbors(point, 1, indices, dists);
          
          if (!indices.empty() && dists[0] <= this->m_max_correspondence_distance * this->m_max_correspondence_distance) {
            thread_correspondences[t].emplace_back(i, indices[0]);
            thread_distances[t].push_back(std::sqrt(dists[0]));
          }
        }
      }));
    }
    
    // 等待所有任务完成
    for (auto& fut : futures) {
      fut.wait();
    }
    
    // 合并结果
    for (std::size_t t = 0; t < num_threads; ++t) {
      correspondences.insert(correspondences.end(),
                            thread_correspondences[t].begin(),
                            thread_correspondences[t].end());
      distances.insert(distances.end(),
                      thread_distances[t].begin(),
                      thread_distances[t].end());
    }
  } else {
    // 串行查找对应关系
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
}

template<typename DataType, typename KNNSearcher>
typename point_to_point_icp_t<DataType, KNNSearcher>::transformation_t
point_to_point_icp_t<DataType, KNNSearcher>::compute_transformation(
    const point_cloud& source,
    const point_cloud& target,
    const std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    const std::vector<DataType>& distances)
{
  using Vector3 = Eigen::Matrix<DataType, 3, 1>;
  using Matrix3 = Eigen::Matrix<DataType, 3, 3>;
  
  // 计算质心
  Vector3 source_centroid = Vector3::Zero();
  Vector3 target_centroid = Vector3::Zero();
  
  for (const auto& [src_idx, tgt_idx] : correspondences) {
    const auto& src_pt = source.points[src_idx];
    const auto& tgt_pt = target.points[tgt_idx];
    
    source_centroid += Vector3(src_pt.x, src_pt.y, src_pt.z);
    target_centroid += Vector3(tgt_pt.x, tgt_pt.y, tgt_pt.z);
  }
  
  const DataType num_corr = static_cast<DataType>(correspondences.size());
  source_centroid /= num_corr;
  target_centroid /= num_corr;
  
  // 计算去质心的协方差矩阵
  Matrix3 W = Matrix3::Zero();
  
  for (const auto& [src_idx, tgt_idx] : correspondences) {
    const auto& src_pt = source.points[src_idx];
    const auto& tgt_pt = target.points[tgt_idx];
    
    Vector3 src_centered(src_pt.x - source_centroid[0],
                        src_pt.y - source_centroid[1],
                        src_pt.z - source_centroid[2]);
    
    Vector3 tgt_centered(tgt_pt.x - target_centroid[0],
                        tgt_pt.y - target_centroid[1],
                        tgt_pt.z - target_centroid[2]);
    
    W += src_centered * tgt_centered.transpose();
  }
  
  // SVD分解
  Eigen::JacobiSVD<Matrix3> svd(W, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Matrix3 U = svd.matrixU();
  Matrix3 V = svd.matrixV();
  
  // 计算旋转矩阵
  Matrix3 R = V * U.transpose();
  
  // 处理反射情况
  if (R.determinant() < 0) {
    V.col(2) *= -1;
    R = V * U.transpose();
  }
  
  // 计算平移向量
  Vector3 t = target_centroid - R * source_centroid;
  
  // 构建变换矩阵
  transformation_t transform = transformation_t::Identity();
  transform.template block<3, 3>(0, 0) = R;
  transform.template block<3, 1>(0, 3) = t;
  
  return transform;
}

template<typename DataType, typename KNNSearcher>
DataType point_to_point_icp_t<DataType, KNNSearcher>::compute_error(
    const std::vector<DataType>& distances) const
{
  if (distances.empty()) {
    return std::numeric_limits<DataType>::max();
  }
  
  DataType sum = std::accumulate(distances.begin(), distances.end(), 
                                static_cast<DataType>(0),
                                [](DataType a, DataType b) { return a + b * b; });
  
  return sum / static_cast<DataType>(distances.size());
}

template<typename DataType, typename KNNSearcher>
void point_to_point_icp_t<DataType, KNNSearcher>::reject_outliers(
    std::vector<std::pair<std::size_t, std::size_t>>& correspondences,
    std::vector<DataType>& distances)
{
  if (correspondences.empty() || m_outlier_rejection_ratio <= 0) {
    return;
  }
  
  // 创建索引数组用于排序
  std::vector<std::size_t> indices(distances.size());
  std::iota(indices.begin(), indices.end(), 0);
  
  // 按距离排序索引
  std::sort(indices.begin(), indices.end(),
            [&distances](std::size_t a, std::size_t b) {
              return distances[a] < distances[b];
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

}  // namespace toolbox::pcl