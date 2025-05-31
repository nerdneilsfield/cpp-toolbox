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
std::size_t harris3d_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t harris3d_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t harris3d_keypoint_extractor_t<DataType, KNN>::set_knn_impl(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(*m_cloud);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t harris3d_keypoint_extractor_t<DataType, KNN>::set_search_radius_impl(data_type radius)
{
  // Harris3D uses num_neighbors instead of radius for main computation
  // But we can use radius for suppression
  m_suppression_radius = radius;
  return 0;
}

template<typename DataType, typename KNN>
void harris3d_keypoint_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
typename harris3d_keypoint_extractor_t<DataType, KNN>::Harris3DInfo
harris3d_keypoint_extractor_t<DataType, KNN>::compute_harris3d_response(std::size_t point_idx)
{
  if (!m_cloud || !m_knn || point_idx >= m_cloud->size()) {
    return Harris3DInfo{0, false};
  }

  const auto& query_point = m_cloud->points[point_idx];
  std::vector<std::size_t> neighbor_indices;
  std::vector<data_type> neighbor_distances;
  
  // Find k nearest neighbors
  m_knn->kneighbors(query_point, m_num_neighbors, neighbor_indices, neighbor_distances);
  
  if (neighbor_indices.size() < 3) {
    return Harris3DInfo{0, false};
  }
  
  // Compute centroid
  Eigen::Vector3d centroid(0, 0, 0);
  for (const auto& idx : neighbor_indices) {
    const auto& p = m_cloud->points[idx];
    centroid += Eigen::Vector3d(static_cast<double>(p.x), 
                               static_cast<double>(p.y), 
                               static_cast<double>(p.z));
  }
  centroid /= static_cast<double>(neighbor_indices.size());
  
  // Compute covariance matrix
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
  
  // Compute eigenvalues and eigenvectors
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigen_solver(covariance);
  if (eigen_solver.info() != Eigen::Success) {
    return Harris3DInfo{0, false};
  }
  
  const Eigen::Vector3d& eigenvalues = eigen_solver.eigenvalues();
  const Eigen::Matrix3d& eigenvectors = eigen_solver.eigenvectors();
  
  // Sort eigenvalues to get normal direction (smallest eigenvalue)
  std::vector<std::pair<double, int>> sorted_eigenvals;
  for (int j = 0; j < 3; ++j) {
    sorted_eigenvals.emplace_back(eigenvalues(j), j);
  }
  std::sort(sorted_eigenvals.begin(), sorted_eigenvals.end());
  
  // Get normal and tangent vectors
  const int normal_idx = sorted_eigenvals[0].second;
  const int tangent1_idx = sorted_eigenvals[1].second;
  const int tangent2_idx = sorted_eigenvals[2].second;
  
  const Eigen::Vector3d normal = eigenvectors.col(normal_idx);
  const Eigen::Vector3d tangent1 = eigenvectors.col(tangent1_idx);
  const Eigen::Vector3d tangent2 = eigenvectors.col(tangent2_idx);
  
  // Project points onto tangent plane and compute 2D structure tensor
  Eigen::Matrix2d structure_tensor = Eigen::Matrix2d::Zero();
  
  for (const auto& idx : neighbor_indices) {
    const auto& p = m_cloud->points[idx];
    Eigen::Vector3d point_vec(static_cast<double>(p.x), 
                             static_cast<double>(p.y), 
                             static_cast<double>(p.z));
    Eigen::Vector3d local_vec = point_vec - centroid;
    
    // Project onto tangent plane
    double u = local_vec.dot(tangent1);
    double v = local_vec.dot(tangent2);
    
    // Add to structure tensor
    structure_tensor(0, 0) += u * u;
    structure_tensor(0, 1) += u * v;
    structure_tensor(1, 0) += u * v;
    structure_tensor(1, 1) += v * v;
  }
  
  structure_tensor /= static_cast<double>(neighbor_indices.size());
  
  // Compute Harris response: R = det(M) - k * trace(M)^2
  const double det = structure_tensor.determinant();
  const double trace = structure_tensor.trace();
  const double harris_response = det - m_harris_k * trace * trace;
  
  return Harris3DInfo{static_cast<data_type>(harris_response), true};
}

template<typename DataType, typename KNN>
void harris3d_keypoint_extractor_t<DataType, KNN>::compute_harris_range(
    std::vector<Harris3DInfo>& harris_responses, 
    std::size_t start_idx, 
    std::size_t end_idx)
{
  for (std::size_t i = start_idx; i < end_idx; ++i) {
    harris_responses[i] = compute_harris3d_response(i);
  }
}

template<typename DataType, typename KNN>
std::vector<typename harris3d_keypoint_extractor_t<DataType, KNN>::Harris3DInfo>
harris3d_keypoint_extractor_t<DataType, KNN>::compute_all_harris_responses()
{
  if (!m_cloud) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<Harris3DInfo> harris_responses(num_points);

  if (m_enable_parallel && num_points > k_parallel_threshold) {
    // Parallel computation
    const std::size_t num_threads = toolbox::base::thread_pool_singleton_t::instance().get_thread_count();
    const std::size_t chunk_size = (num_points + num_threads - 1) / num_threads;
    
    std::vector<std::future<void>> futures;
    for (std::size_t t = 0; t < num_threads; ++t) {
      const std::size_t start_idx = t * chunk_size;
      const std::size_t end_idx = std::min(start_idx + chunk_size, num_points);
      
      if (start_idx < end_idx) {
        futures.emplace_back(
          toolbox::base::thread_pool_singleton_t::instance().submit(
            [this, &harris_responses, start_idx, end_idx]() {
              compute_harris_range(harris_responses, start_idx, end_idx);
            }
          )
        );
      }
    }
    
    // Wait for all threads to complete
    for (auto& future : futures) {
      future.wait();
    }
  } else {
    // Sequential computation
    compute_harris_range(harris_responses, 0, num_points);
  }

  return harris_responses;
}

template<typename DataType, typename KNN>
typename harris3d_keypoint_extractor_t<DataType, KNN>::indices_vector
harris3d_keypoint_extractor_t<DataType, KNN>::apply_non_maxima_suppression(
    const std::vector<Harris3DInfo>& harris_responses)
{
  if (!m_cloud || harris_responses.empty()) {
    return {};
  }

  indices_vector keypoints;
  const std::size_t num_points = m_cloud->size();

  for (std::size_t i = 0; i < num_points; ++i) {
    const auto& current_response = harris_responses[i];
    
    // Skip invalid points or those below threshold
    if (!current_response.is_valid || current_response.harris_response < m_threshold) {
      continue;
    }

    // Find neighbors within suppression radius
    const auto& query_point = m_cloud->points[i];
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    
    m_knn->radius_neighbors(query_point, m_suppression_radius, neighbor_indices, neighbor_distances);

    // Check if current point is local maximum
    bool is_local_maximum = true;
    for (const auto& neighbor_idx : neighbor_indices) {
      if (neighbor_idx != i && neighbor_idx < harris_responses.size()) {
        if (harris_responses[neighbor_idx].is_valid && 
            harris_responses[neighbor_idx].harris_response > current_response.harris_response) {
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
typename harris3d_keypoint_extractor_t<DataType, KNN>::indices_vector
harris3d_keypoint_extractor_t<DataType, KNN>::extract_impl()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  // Compute Harris responses for all points
  auto harris_responses = compute_all_harris_responses();
  
  // Apply non-maxima suppression to find keypoints
  return apply_non_maxima_suppression(harris_responses);
}

template<typename DataType, typename KNN>
void harris3d_keypoint_extractor_t<DataType, KNN>::extract_impl(indices_vector& keypoint_indices)
{
  keypoint_indices = extract_impl();
}

template<typename DataType, typename KNN>
typename harris3d_keypoint_extractor_t<DataType, KNN>::point_cloud
harris3d_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl()
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
void harris3d_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl(point_cloud_ptr output)
{
  auto keypoint_indices = extract_impl();
  
  output->points.clear();
  output->points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    output->points.push_back(m_cloud->points[idx]);
  }
}

}  // namespace toolbox::pcl