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
std::size_t curvature_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t curvature_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t curvature_keypoint_extractor_t<DataType, KNN>::set_knn_impl(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(*m_cloud);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t curvature_keypoint_extractor_t<DataType, KNN>::set_search_radius_impl(data_type radius)
{
  m_search_radius = radius;
  return 0;
}

template<typename DataType, typename KNN>
void curvature_keypoint_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
typename curvature_keypoint_extractor_t<DataType, KNN>::CurvatureInfo
curvature_keypoint_extractor_t<DataType, KNN>::compute_curvature(std::size_t point_idx)
{
  if (!m_cloud || !m_knn || point_idx >= m_cloud->size()) {
    return CurvatureInfo{0, 0, 0, 0, 0};
  }

  const auto& query_point = m_cloud->points[point_idx];
  std::vector<std::size_t> neighbor_indices;
  std::vector<data_type> neighbor_distances;

  // 在半径内查找邻居点 / Find neighbors within radius
  m_knn->radius_neighbors(query_point, m_search_radius, neighbor_indices, neighbor_distances);

  if (neighbor_indices.size() < m_min_neighbors) {
    return CurvatureInfo{0, 0, 0, 0, 0};
  }

  // 计算协方差矩阵 / Compute covariance matrix
  Eigen::Vector3d centroid(0, 0, 0);
  for (const auto& idx : neighbor_indices) {
    const auto& p = m_cloud->points[idx];
    centroid += Eigen::Vector3d(static_cast<double>(p.x), 
                               static_cast<double>(p.y), 
                               static_cast<double>(p.z));
  }
  centroid /= static_cast<double>(neighbor_indices.size());

  Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();
  for (const auto& idx : neighbor_indices) {
    const auto& p = m_cloud->points[idx];
    Eigen::Vector3d point_vec(static_cast<double>(p.x), 
                             static_cast<double>(p.y), 
                             static_cast<double>(p.z));
    Eigen::Vector3d diff = point_vec - centroid;
    covariance += diff * diff.transpose();
  }
  covariance /= static_cast<double>(neighbor_indices.size() - 1);

  // 计算特征值和特征向量 / Compute eigenvalues and eigenvectors
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigen_solver(covariance);
  if (eigen_solver.info() != Eigen::Success) {
    return CurvatureInfo{0, 0, 0, 0, 0};
  }

  const Eigen::Vector3d& eigenvalues = eigen_solver.eigenvalues();
  
  // 按降序排序特征值：λ0 >= λ1 >= λ2 / Sort eigenvalues in descending order: λ0 >= λ1 >= λ2
  std::vector<double> sorted_eigenvals = {eigenvalues(2), eigenvalues(1), eigenvalues(0)};
  std::sort(sorted_eigenvals.rbegin(), sorted_eigenvals.rend());

  const double lambda0 = sorted_eigenvals[0];
  const double lambda1 = sorted_eigenvals[1];
  const double lambda2 = sorted_eigenvals[2];

  // 避免除零 / Avoid division by zero
  const double eigensum = lambda0 + lambda1 + lambda2;
  if (eigensum < 1e-10) {
    return CurvatureInfo{0, 0, 0, 0, 0};
  }

  // 计算曲率测度 / Compute curvature measures
  const double principal_curvature_1 = lambda1 / eigensum;
  const double principal_curvature_2 = lambda2 / eigensum;
  const double mean_curvature = (principal_curvature_1 + principal_curvature_2) / 2.0;
  const double gaussian_curvature = principal_curvature_1 * principal_curvature_2;
  const double curvature_magnitude = std::sqrt(principal_curvature_1 * principal_curvature_1 + 
                                               principal_curvature_2 * principal_curvature_2);

  return CurvatureInfo{
    static_cast<data_type>(principal_curvature_1),
    static_cast<data_type>(principal_curvature_2),
    static_cast<data_type>(mean_curvature),
    static_cast<data_type>(gaussian_curvature),
    static_cast<data_type>(curvature_magnitude)
  };
}

template<typename DataType, typename KNN>
void curvature_keypoint_extractor_t<DataType, KNN>::compute_curvatures_range(
    std::vector<CurvatureInfo>& curvatures, 
    std::size_t start_idx, 
    std::size_t end_idx)
{
  for (std::size_t i = start_idx; i < end_idx; ++i) {
    curvatures[i] = compute_curvature(i);
  }
}

template<typename DataType, typename KNN>
std::vector<typename curvature_keypoint_extractor_t<DataType, KNN>::CurvatureInfo>
curvature_keypoint_extractor_t<DataType, KNN>::compute_all_curvatures()
{
  if (!m_cloud) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<CurvatureInfo> curvatures(num_points);

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
            [this, &curvatures, start_idx, end_idx]() {
              compute_curvatures_range(curvatures, start_idx, end_idx);
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
    compute_curvatures_range(curvatures, 0, num_points);
  }

  return curvatures;
}

template<typename DataType, typename KNN>
typename curvature_keypoint_extractor_t<DataType, KNN>::indices_vector
curvature_keypoint_extractor_t<DataType, KNN>::apply_non_maxima_suppression(
    const std::vector<CurvatureInfo>& curvatures)
{
  if (!m_cloud || curvatures.empty()) {
    return {};
  }

  indices_vector keypoints;
  const std::size_t num_points = m_cloud->size();

  for (std::size_t i = 0; i < num_points; ++i) {
    const auto& current_curvature = curvatures[i];
    
    // 跳过低曲率的点 / Skip points with low curvature
    if (current_curvature.curvature_magnitude < m_curvature_threshold) {
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
      if (neighbor_idx != i && neighbor_idx < curvatures.size()) {
        if (curvatures[neighbor_idx].curvature_magnitude > current_curvature.curvature_magnitude) {
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
typename curvature_keypoint_extractor_t<DataType, KNN>::indices_vector
curvature_keypoint_extractor_t<DataType, KNN>::extract_impl()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  // 计算所有点的曲率 / Compute curvatures for all points
  auto curvatures = compute_all_curvatures();
  
  // 应用非极大值抑制找到关键点 / Apply non-maxima suppression to find keypoints
  return apply_non_maxima_suppression(curvatures);
}

template<typename DataType, typename KNN>
void curvature_keypoint_extractor_t<DataType, KNN>::extract_impl(indices_vector& keypoint_indices)
{
  keypoint_indices = extract_impl();
}

template<typename DataType, typename KNN>
typename curvature_keypoint_extractor_t<DataType, KNN>::point_cloud
curvature_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl()
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
void curvature_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl(point_cloud_ptr output)
{
  auto keypoint_indices = extract_impl();
  
  output->points.clear();
  output->points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    output->points.push_back(m_cloud->points[idx]);
  }
}

}  // namespace toolbox::pcl