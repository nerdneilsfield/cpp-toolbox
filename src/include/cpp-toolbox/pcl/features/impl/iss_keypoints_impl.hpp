#pragma once

#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <algorithm>
#include <cmath>
#include <future>
#include <vector>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
std::size_t iss_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t iss_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t iss_keypoint_extractor_t<DataType, KNN>::set_knn_impl(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(m_cloud->points);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t iss_keypoint_extractor_t<DataType, KNN>::set_search_radius_impl(data_type radius)
{
  m_salient_radius = radius;
  return 0;
}

template<typename DataType, typename KNN>
void iss_keypoint_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
typename iss_keypoint_extractor_t<DataType, KNN>::ISSInfo
iss_keypoint_extractor_t<DataType, KNN>::compute_iss_response(std::size_t point_idx)
{
  if (!m_cloud || !m_knn || point_idx >= m_cloud->size()) {
    return ISSInfo{0, 0, 0, 0, false};
  }

  const auto& query_point = m_cloud->points[point_idx];
  std::vector<std::size_t> neighbor_indices;
  std::vector<data_type> neighbor_distances;

  // 在显著性半径内查找邻居 / Find neighbors within salient radius
  m_knn->radius_neighbors(query_point, m_salient_radius, neighbor_indices, neighbor_distances);

  if (neighbor_indices.size() < m_min_neighbors) {
    return ISSInfo{0, 0, 0, 0, false};
  }

  // 基于距离计算权重函数 / Compute weight function based on distance
  auto compute_weight = [this](data_type distance) -> double {
    if (distance >= m_salient_radius) return 0.0;
    const double ratio = static_cast<double>(distance) / static_cast<double>(m_salient_radius);
    return 1.0 - ratio;  // 线性权重函数 / Linear weight function
  };

  // 计算加权散布矩阵 / Compute weighted scatter matrix
  Eigen::Matrix3d scatter_matrix = Eigen::Matrix3d::Zero();
  double total_weight = 0.0;

  for (std::size_t i = 0; i < neighbor_indices.size(); ++i) {
    const auto& neighbor_point = m_cloud->points[neighbor_indices[i]];
    const double weight = compute_weight(neighbor_distances[i]);
    
    if (weight > 0.0) {
      // 从查询点到邻居的向量 / Vector from query point to neighbor
      Eigen::Vector3d diff(
        static_cast<double>(neighbor_point.x - query_point.x),
        static_cast<double>(neighbor_point.y - query_point.y),
        static_cast<double>(neighbor_point.z - query_point.z)
      );
      
      scatter_matrix += weight * (diff * diff.transpose());
      total_weight += weight;
    }
  }

  if (total_weight < 1e-10) {
    return ISSInfo{0, 0, 0, 0, false};
  }

  scatter_matrix /= total_weight;

  // 计算特征值 / Compute eigenvalues
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigen_solver(scatter_matrix);
  if (eigen_solver.info() != Eigen::Success) {
    return ISSInfo{0, 0, 0, 0, false};
  }

  const Eigen::Vector3d& eigenvalues = eigen_solver.eigenvalues();
  
  // 按降序排序特征值：λ1 >= λ2 >= λ3 / Sort eigenvalues in descending order: λ1 >= λ2 >= λ3
  std::vector<double> sorted_eigenvals = {eigenvalues(2), eigenvalues(1), eigenvalues(0)};
  std::sort(sorted_eigenvals.rbegin(), sorted_eigenvals.rend());

  const double lambda1 = sorted_eigenvals[0];
  const double lambda2 = sorted_eigenvals[1];
  const double lambda3 = sorted_eigenvals[2];

  // 检查ISS准则 / Check ISS criteria
  bool is_valid = true;
  
  // 避免除零 / Avoid division by zero
  if (lambda1 < 1e-10) {
    is_valid = false;
  } else {
    // 检查特征值比率 / Check eigenvalue ratios
    const double ratio21 = lambda2 / lambda1;
    const double ratio32 = (lambda2 > 1e-10) ? (lambda3 / lambda2) : 0.0;
    
    if (ratio21 > m_threshold21 || ratio32 > m_threshold32) {
      is_valid = false;
    }
  }

  // ISS显著性是最小的特征值 / ISS saliency is the smallest eigenvalue
  const double saliency = lambda3;

  return ISSInfo{
    static_cast<data_type>(lambda1),
    static_cast<data_type>(lambda2),
    static_cast<data_type>(lambda3),
    static_cast<data_type>(saliency),
    is_valid
  };
}

template<typename DataType, typename KNN>
void iss_keypoint_extractor_t<DataType, KNN>::compute_iss_range(
    std::vector<ISSInfo>& iss_responses, 
    std::size_t start_idx, 
    std::size_t end_idx)
{
  for (std::size_t i = start_idx; i < end_idx; ++i) {
    iss_responses[i] = compute_iss_response(i);
  }
}

template<typename DataType, typename KNN>
std::vector<typename iss_keypoint_extractor_t<DataType, KNN>::ISSInfo>
iss_keypoint_extractor_t<DataType, KNN>::compute_all_iss_responses()
{
  if (!m_cloud) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<ISSInfo> iss_responses(num_points);

  if (m_enable_parallel && num_points > k_parallel_threshold) {
    // 并行计算 / Parallel computation
    const std::size_t num_threads = toolbox::base::thread_pool_singleton_t::instance().get_thread_count();
    const std::size_t chunk_size = (num_points + num_threads - 1) / num_threads;
    
    std::vector<std::future<void>> futures;
    for (std::size_t t = 0; t < num_threads; ++t) {
      const std::size_t start_idx = t * chunk_size;
      const std::size_t end_idx = std::min(start_idx + chunk_size, num_points);
      
      if (start_idx < end_idx) {
        futures.emplace_back(
          toolbox::base::thread_pool_singleton_t::instance().submit(
            [this, &iss_responses, start_idx, end_idx]() {
              compute_iss_range(iss_responses, start_idx, end_idx);
            }
          )
        );
      }
    }
    
    // 等待所有线程完成 / Wait for all threads to complete
    for (auto& future : futures) {
      future.wait();
    }
  } else {
    // 顺序计算 / Sequential computation
    compute_iss_range(iss_responses, 0, num_points);
  }

  return iss_responses;
}

template<typename DataType, typename KNN>
typename iss_keypoint_extractor_t<DataType, KNN>::indices_vector
iss_keypoint_extractor_t<DataType, KNN>::apply_non_maxima_suppression(
    const std::vector<ISSInfo>& iss_responses)
{
  if (!m_cloud || iss_responses.empty()) {
    return {};
  }

  indices_vector keypoints;
  const std::size_t num_points = m_cloud->size();

  for (std::size_t i = 0; i < num_points; ++i) {
    const auto& current_iss = iss_responses[i];
    
    // 跳过无效的点 / Skip invalid points
    if (!current_iss.is_valid || current_iss.saliency <= 0) {
      continue;
    }

    // 在非极大值抑制半径内查找邻居 / Find neighbors within non-maxima suppression radius
    const auto& query_point = m_cloud->points[i];
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    
    m_knn->radius_neighbors(query_point, m_non_maxima_radius, neighbor_indices, neighbor_distances);

    // 检查当前点是否为局部最大值 / Check if current point is local maximum
    bool is_local_maximum = true;
    for (const auto& neighbor_idx : neighbor_indices) {
      if (neighbor_idx != i && neighbor_idx < iss_responses.size()) {
        const auto& neighbor_iss = iss_responses[neighbor_idx];
        if (neighbor_iss.is_valid && neighbor_iss.saliency > current_iss.saliency) {
          is_local_maximum = false;
          break;
        }
      }
    }

    if (is_local_maximum) {
      keypoints.push_back(i);
    }
  }

  return keypoints;
}

template<typename DataType, typename KNN>
typename iss_keypoint_extractor_t<DataType, KNN>::indices_vector
iss_keypoint_extractor_t<DataType, KNN>::extract_impl()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  // 计算所有点的ISS响应 / Compute ISS responses for all points
  auto iss_responses = compute_all_iss_responses();
  
  // 应用非极大值抑制找到关键点 / Apply non-maxima suppression to find keypoints
  return apply_non_maxima_suppression(iss_responses);
}

template<typename DataType, typename KNN>
void iss_keypoint_extractor_t<DataType, KNN>::extract_impl(indices_vector& keypoint_indices)
{
  keypoint_indices = extract_impl();
}

template<typename DataType, typename KNN>
typename iss_keypoint_extractor_t<DataType, KNN>::point_cloud
iss_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl()
{
  auto keypoint_indices = extract_impl();
  
  point_cloud keypoints;
  keypoints.points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    keypoints.points.push_back(m_cloud->points[idx]);
  }
  
  return keypoints;
}

template<typename DataType, typename KNN>
void iss_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl(point_cloud_ptr output)
{
  auto keypoint_indices = extract_impl();
  
  output->points.clear();
  output->points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    output->points.push_back(m_cloud->points[idx]);
  }
}

}  // namespace toolbox::pcl