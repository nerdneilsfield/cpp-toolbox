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
std::size_t susan_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t susan_keypoint_extractor_t<DataType, KNN>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t susan_keypoint_extractor_t<DataType, KNN>::set_knn_impl(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(m_cloud->points);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t susan_keypoint_extractor_t<DataType, KNN>::set_search_radius_impl(data_type radius)
{
  m_search_radius = radius;
  return 0;
}

template<typename DataType, typename KNN>
void susan_keypoint_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
std::vector<typename susan_keypoint_extractor_t<DataType, KNN>::NormalInfo>
susan_keypoint_extractor_t<DataType, KNN>::compute_normals()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<NormalInfo> normals(num_points);

  // Use PCA to compute normals
  pca_norm_extractor_t<data_type, knn_type> norm_estimator;
  norm_estimator.set_input(*m_cloud);
  norm_estimator.set_num_neighbors(30);  // Use fixed number of neighbors for normal estimation
  norm_estimator.set_knn(*m_knn);

  auto estimated_normals = norm_estimator.extract();
  
  // Convert to our NormalInfo structure
  for (std::size_t i = 0; i < num_points && i < estimated_normals.points.size(); ++i) {
    const auto& n = estimated_normals.points[i];
    normals[i] = NormalInfo{
      static_cast<data_type>(n.x), 
      static_cast<data_type>(n.y), 
      static_cast<data_type>(n.z),
      true
    };
    
    // Check for invalid normals
    if (std::isnan(n.x) || std::isnan(n.y) || std::isnan(n.z)) {
      normals[i].is_valid = false;
    }
  }

  return normals;
}

template<typename DataType, typename KNN>
typename susan_keypoint_extractor_t<DataType, KNN>::SUSANInfo
susan_keypoint_extractor_t<DataType, KNN>::compute_susan_response(
    std::size_t point_idx, 
    const std::vector<NormalInfo>& normals)
{
  if (!m_cloud || !m_knn || point_idx >= m_cloud->size()) {
    return SUSANInfo{0, false};
  }

  const auto& query_point = m_cloud->points[point_idx];
  std::vector<std::size_t> neighbor_indices;
  std::vector<data_type> neighbor_distances;
  
  // Find neighbors within search radius
  m_knn->radius_neighbors(query_point, m_search_radius, neighbor_indices, neighbor_distances);
  
  if (neighbor_indices.size() < 5) {  // Need minimum neighbors
    return SUSANInfo{0, false};
  }

  // SUSAN principle: count similar neighbors (USAN - Univalue Segment Assimilating Nucleus)
  data_type susan_value = 0;
  const data_type max_distance = m_search_radius;
  
  if (m_use_normal_similarity && point_idx < normals.size() && normals[point_idx].is_valid) {
    // Use normal-based similarity
    const auto& center_normal = normals[point_idx];
    const data_type center_norm = std::sqrt(
      center_normal.nx * center_normal.nx + 
      center_normal.ny * center_normal.ny + 
      center_normal.nz * center_normal.nz
    );
    
    if (center_norm < 1e-6) {
      return SUSANInfo{0, false};
    }
    
    for (std::size_t i = 0; i < neighbor_indices.size(); ++i) {
      const std::size_t neighbor_idx = neighbor_indices[i];
      if (neighbor_idx == point_idx || neighbor_idx >= normals.size() || !normals[neighbor_idx].is_valid) {
        continue;
      }
      
      const auto& neighbor_normal = normals[neighbor_idx];
      
      // Compute normal similarity (dot product)
      const data_type dot_product = 
        center_normal.nx * neighbor_normal.nx +
        center_normal.ny * neighbor_normal.ny +
        center_normal.nz * neighbor_normal.nz;
      
      // Angular similarity
      if (dot_product > m_angular_threshold) {
        // Geometric similarity (distance-based weight)
        const data_type distance_ratio = neighbor_distances[i] / max_distance;
        const data_type geometric_weight = std::exp(-distance_ratio * distance_ratio / (m_geometric_threshold * m_geometric_threshold));
        
        susan_value += geometric_weight;
      }
    }
  } else {
    // Use geometric distance similarity only
    for (std::size_t i = 0; i < neighbor_indices.size(); ++i) {
      if (neighbor_indices[i] == point_idx) continue;
      
      const data_type distance_ratio = neighbor_distances[i] / max_distance;
      const data_type similarity = std::exp(-distance_ratio * distance_ratio / (m_geometric_threshold * m_geometric_threshold));
      
      susan_value += similarity;
    }
  }
  
  // Normalize by number of neighbors
  if (neighbor_indices.size() > 1) {
    susan_value /= static_cast<data_type>(neighbor_indices.size() - 1);
  }
  
  return SUSANInfo{susan_value, true};
}

template<typename DataType, typename KNN>
void susan_keypoint_extractor_t<DataType, KNN>::compute_susan_range(
    std::vector<SUSANInfo>& susan_responses,
    const std::vector<NormalInfo>& normals,
    std::size_t start_idx, 
    std::size_t end_idx)
{
  for (std::size_t i = start_idx; i < end_idx; ++i) {
    susan_responses[i] = compute_susan_response(i, normals);
  }
}

template<typename DataType, typename KNN>
std::vector<typename susan_keypoint_extractor_t<DataType, KNN>::SUSANInfo>
susan_keypoint_extractor_t<DataType, KNN>::compute_all_susan_responses(
    const std::vector<NormalInfo>& normals)
{
  if (!m_cloud) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<SUSANInfo> susan_responses(num_points);

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
            [this, &susan_responses, &normals, start_idx, end_idx]() {
              compute_susan_range(susan_responses, normals, start_idx, end_idx);
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
    compute_susan_range(susan_responses, normals, 0, num_points);
  }

  return susan_responses;
}

template<typename DataType, typename KNN>
typename susan_keypoint_extractor_t<DataType, KNN>::indices_vector
susan_keypoint_extractor_t<DataType, KNN>::apply_non_maxima_suppression(
    const std::vector<SUSANInfo>& susan_responses)
{
  if (!m_cloud || susan_responses.empty()) {
    return {};
  }

  indices_vector keypoints;
  const std::size_t num_points = m_cloud->size();

  for (std::size_t i = 0; i < num_points; ++i) {
    const auto& current_response = susan_responses[i];
    
    // Skip invalid points or those above threshold (high SUSAN value means flat region)
    if (!current_response.is_valid || current_response.susan_value > m_susan_threshold) {
      continue;
    }

    // Find neighbors within non-maxima suppression radius
    const auto& query_point = m_cloud->points[i];
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    
    m_knn->radius_neighbors(query_point, m_non_maxima_radius, neighbor_indices, neighbor_distances);

    // Check if current point is local minimum (for SUSAN, lower values indicate features)
    bool is_local_minimum = true;
    for (const auto& neighbor_idx : neighbor_indices) {
      if (neighbor_idx != i && neighbor_idx < susan_responses.size()) {
        if (susan_responses[neighbor_idx].is_valid && 
            susan_responses[neighbor_idx].susan_value < current_response.susan_value) {
          is_local_minimum = false;
          break;
        }
      }
    }

    if (is_local_minimum) {
      keypoints.push_back(i);
    }
  }

  return keypoints;
}

template<typename DataType, typename KNN>
typename susan_keypoint_extractor_t<DataType, KNN>::indices_vector
susan_keypoint_extractor_t<DataType, KNN>::extract_impl()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  // Compute normals if using normal similarity
  std::vector<NormalInfo> normals;
  if (m_use_normal_similarity) {
    normals = compute_normals();
  }

  // Compute SUSAN responses for all points
  auto susan_responses = compute_all_susan_responses(normals);
  
  // Apply non-maxima suppression to find keypoints
  return apply_non_maxima_suppression(susan_responses);
}

template<typename DataType, typename KNN>
void susan_keypoint_extractor_t<DataType, KNN>::extract_impl(indices_vector& keypoint_indices)
{
  keypoint_indices = extract_impl();
}

template<typename DataType, typename KNN>
typename susan_keypoint_extractor_t<DataType, KNN>::point_cloud
susan_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl()
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
void susan_keypoint_extractor_t<DataType, KNN>::extract_keypoints_impl(point_cloud_ptr output)
{
  auto keypoint_indices = extract_impl();
  
  output->points.clear();
  output->points.reserve(keypoint_indices.size());
  
  for (const auto& idx : keypoint_indices) {
    output->points.push_back(m_cloud->points[idx]);
  }
}

}  // namespace toolbox::pcl