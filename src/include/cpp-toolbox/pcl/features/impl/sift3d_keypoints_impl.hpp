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
std::size_t sift3d_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t sift3d_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t sift3d_keypoint_extractor_t<DataType, KNN>::set_knn_impl(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(m_cloud->points);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t sift3d_keypoint_extractor_t<DataType, KNN>::set_search_radius_impl(data_type radius)
{
  // Use radius as base scale
  m_base_scale = radius;
  return 0;
}

template<typename DataType, typename KNN>
void sift3d_keypoint_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
void sift3d_keypoint_extractor_t<DataType, KNN>::compute_scale_space_range(
    std::vector<std::vector<data_type>>& scale_space,
    std::size_t start_idx,
    std::size_t end_idx)
{
  for (std::size_t scale_idx = 0; scale_idx < m_num_scales; ++scale_idx) {
    const data_type current_scale = m_base_scale * std::pow(m_scale_factor, scale_idx);
    
    for (std::size_t i = start_idx; i < end_idx; ++i) {
      const auto& query_point = m_cloud->points[i];
      
      // Find neighbors within the current scale radius
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      
      m_knn->radius_neighbors(query_point, current_scale * 3.0, neighbor_indices, neighbor_distances);
      
      if (neighbor_indices.size() < 3) {
        scale_space[scale_idx][i] = 0.0;
        continue;
      }
      
      // Compute local density using Gaussian weighting
      data_type density_current = 0.0;
      data_type density_prev = 0.0;
      
      const data_type sigma_current = current_scale;
      const data_type sigma_prev = current_scale / m_scale_factor;
      
      for (std::size_t j = 0; j < neighbor_indices.size(); ++j) {
        const data_type dist = neighbor_distances[j];
        
        // Gaussian weights
        const data_type weight_current = std::exp(-0.5 * (dist * dist) / (sigma_current * sigma_current));
        const data_type weight_prev = std::exp(-0.5 * (dist * dist) / (sigma_prev * sigma_prev));
        
        density_current += weight_current;
        density_prev += weight_prev;
      }
      
      // Difference of Gaussians
      scale_space[scale_idx][i] = density_current - density_prev;
    }
  }
}

template<typename DataType, typename KNN>
std::vector<std::vector<typename sift3d_keypoint_extractor_t<DataType, KNN>::data_type>>
sift3d_keypoint_extractor_t<DataType, KNN>::build_scale_space()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<std::vector<data_type>> scale_space(m_num_scales);
  
  for (auto& scale_level : scale_space) {
    scale_level.resize(num_points, 0.0);
  }

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
            [this, &scale_space, start_idx, end_idx]() {
              compute_scale_space_range(scale_space, start_idx, end_idx);
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
    compute_scale_space_range(scale_space, 0, num_points);
  }

  return scale_space;
}

template<typename DataType, typename KNN>
std::vector<typename sift3d_keypoint_extractor_t<DataType, KNN>::ScaleSpacePoint>
sift3d_keypoint_extractor_t<DataType, KNN>::find_scale_space_extrema(
    const std::vector<std::vector<data_type>>& scale_space)
{
  const std::size_t num_points = m_cloud->size();
  std::vector<ScaleSpacePoint> extrema;
  
  // Check for extrema across scales (excluding first and last scale)
  for (std::size_t scale_idx = 1; scale_idx < m_num_scales - 1; ++scale_idx) {
    for (std::size_t point_idx = 0; point_idx < num_points; ++point_idx) {
      const data_type current_value = scale_space[scale_idx][point_idx];
      
      // Check if it's a local extremum across scales
      const data_type prev_scale_value = scale_space[scale_idx - 1][point_idx];
      const data_type next_scale_value = scale_space[scale_idx + 1][point_idx];
      
      bool is_max = (current_value > prev_scale_value) && (current_value > next_scale_value);
      bool is_min = (current_value < prev_scale_value) && (current_value < next_scale_value);
      
      if (!is_max && !is_min) {
        continue;
      }
      
      // Check spatial neighbors at the same scale
      const auto& query_point = m_cloud->points[point_idx];
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      
      const data_type search_radius = m_base_scale * std::pow(m_scale_factor, scale_idx);
      m_knn->radius_neighbors(query_point, search_radius, neighbor_indices, neighbor_distances);
      
      bool is_spatial_extremum = true;
      for (const auto& neighbor_idx : neighbor_indices) {
        if (neighbor_idx == point_idx) continue;
        
        const data_type neighbor_value = scale_space[scale_idx][neighbor_idx];
        
        if (is_max && neighbor_value > current_value) {
          is_spatial_extremum = false;
          break;
        }
        if (is_min && neighbor_value < current_value) {
          is_spatial_extremum = false;
          break;
        }
      }
      
      if (is_spatial_extremum && std::abs(current_value) > m_contrast_threshold) {
        extrema.push_back(ScaleSpacePoint{point_idx, scale_idx, current_value, true});
      }
    }
  }
  
  return extrema;
}

template<typename DataType, typename KNN>
typename sift3d_keypoint_extractor_t<DataType, KNN>::indices_vector
sift3d_keypoint_extractor_t<DataType, KNN>::refine_keypoints(
    const std::vector<ScaleSpacePoint>& extrema)
{
  indices_vector refined_keypoints;
  
  for (const auto& extremum : extrema) {
    if (extremum.is_extremum && std::abs(extremum.response) > m_contrast_threshold) {
      refined_keypoints.push_back(extremum.point_idx);
    }
  }
  
  // Remove duplicates
  std::sort(refined_keypoints.begin(), refined_keypoints.end());
  refined_keypoints.erase(std::unique(refined_keypoints.begin(), refined_keypoints.end()), 
                         refined_keypoints.end());
  
  return refined_keypoints;
}

template<typename DataType, typename KNN>
typename sift3d_keypoint_extractor_t<DataType, KNN>::indices_vector
sift3d_keypoint_extractor_t<DataType, KNN>::remove_edge_responses(
    const indices_vector& keypoint_indices)
{
  indices_vector final_keypoints;
  
  for (const auto& point_idx : keypoint_indices) {
    const auto& query_point = m_cloud->points[point_idx];
    
    // Find local neighborhood
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    
    m_knn->kneighbors(query_point, m_num_neighbors, neighbor_indices, neighbor_distances);
    
    if (neighbor_indices.size() < 3) {
      continue;
    }
    
    // Compute local structure tensor
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
    
    // Compute eigenvalues
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> eigen_solver(covariance);
    if (eigen_solver.info() != Eigen::Success) {
      continue;
    }
    
    const Eigen::Vector3d& eigenvalues = eigen_solver.eigenvalues();
    
    // Sort eigenvalues in descending order
    std::vector<double> sorted_eigenvals = {eigenvalues(2), eigenvalues(1), eigenvalues(0)};
    std::sort(sorted_eigenvals.rbegin(), sorted_eigenvals.rend());
    
    const double lambda_max = sorted_eigenvals[0];
    const double lambda_min = sorted_eigenvals[2];
    
    // Edge response test: ratio of largest to smallest eigenvalue
    if (lambda_min > 1e-10) {
      const double edge_ratio = lambda_max / lambda_min;
      
      if (edge_ratio < m_edge_threshold) {
        final_keypoints.push_back(point_idx);
      }
    }
  }
  
  return final_keypoints;
}

template<typename DataType, typename KNN>
typename sift3d_keypoint_extractor_t<DataType, KNN>::indices_vector
sift3d_keypoint_extractor_t<DataType, KNN>::extract_impl()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  // Step 1: Build scale space
  auto scale_space = build_scale_space();
  
  // Step 2: Find scale-space extrema
  auto extrema = find_scale_space_extrema(scale_space);
  
  // Step 3: Refine keypoint locations
  auto refined_keypoints = refine_keypoints(extrema);
  
  // Step 4: Remove edge responses
  return remove_edge_responses(refined_keypoints);
}

template<typename DataType, typename KNN>
void sift3d_keypoint_extractor_t<DataType, KNN>::extract_impl(indices_vector& keypoint_indices)
{
  keypoint_indices = extract_impl();
}

template<typename DataType, typename KNN>
typename sift3d_keypoint_extractor_t<DataType, KNN>::point_cloud
sift3d_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl()
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
void sift3d_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl(point_cloud_ptr output)
{
  auto keypoint_indices = extract_impl();
  
  output->points.clear();
  output->points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    output->points.push_back(m_cloud->points[idx]);
  }
}

}  // namespace toolbox::pcl