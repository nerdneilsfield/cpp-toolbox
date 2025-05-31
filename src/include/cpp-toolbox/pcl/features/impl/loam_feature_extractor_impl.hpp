#pragma once

#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <algorithm>
#include <cmath>
#include <future>
#include <vector>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
std::size_t loam_feature_extractor_t<DataType, KNN>::set_input_impl(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t loam_feature_extractor_t<DataType, KNN>::set_input_impl(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t loam_feature_extractor_t<DataType, KNN>::set_knn_impl(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud) {
    m_knn->set_input(*m_cloud);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t loam_feature_extractor_t<DataType, KNN>::set_search_radius_impl(data_type radius)
{
  // LOAM uses fixed number of neighbors instead of radius
  // But we can use radius for secondary validation
  return 0;
}

template<typename DataType, typename KNN>
void loam_feature_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::CurvatureInfo
loam_feature_extractor_t<DataType, KNN>::compute_point_curvature(std::size_t point_idx)
{
  if (!m_cloud || !m_knn || point_idx >= m_cloud->size()) {
    return CurvatureInfo{0, false};
  }

  const auto& query_point = m_cloud->points[point_idx];
  std::vector<std::size_t> neighbor_indices;
  std::vector<data_type> neighbor_distances;
  
  // Find k nearest neighbors
  m_knn->kneighbors(query_point, m_num_scan_neighbors, neighbor_indices, neighbor_distances);
  
  if (neighbor_indices.size() < 5) {  // Need minimum neighbors
    return CurvatureInfo{0, false};
  }

  // LOAM curvature formula:
  // c = |Σ(X_i - X_j)|/(k * |X_j|)
  // where X_j is the query point and X_i are neighbors
  
  data_type sum_diff_x = 0;
  data_type sum_diff_y = 0;
  data_type sum_diff_z = 0;
  
  for (const auto& idx : neighbor_indices) {
    if (idx != point_idx) {  // Skip self
      const auto& neighbor = m_cloud->points[idx];
      sum_diff_x += neighbor.x - query_point.x;
      sum_diff_y += neighbor.y - query_point.y;
      sum_diff_z += neighbor.z - query_point.z;
    }
  }
  
  // Compute curvature magnitude
  const data_type diff_magnitude = std::sqrt(
    sum_diff_x * sum_diff_x + 
    sum_diff_y * sum_diff_y + 
    sum_diff_z * sum_diff_z
  );
  
  const data_type point_magnitude = std::sqrt(
    query_point.x * query_point.x + 
    query_point.y * query_point.y + 
    query_point.z * query_point.z
  );
  
  data_type curvature = 0;
  if (point_magnitude > 1e-6) {
    curvature = diff_magnitude / (static_cast<data_type>(neighbor_indices.size()) * point_magnitude);
  }
  
  return CurvatureInfo{curvature, true};
}

template<typename DataType, typename KNN>
void loam_feature_extractor_t<DataType, KNN>::compute_curvatures_range(
    std::vector<CurvatureInfo>& curvatures, 
    std::size_t start_idx, 
    std::size_t end_idx)
{
  for (std::size_t i = start_idx; i < end_idx; ++i) {
    curvatures[i] = compute_point_curvature(i);
  }
}

template<typename DataType, typename KNN>
std::vector<typename loam_feature_extractor_t<DataType, KNN>::CurvatureInfo>
loam_feature_extractor_t<DataType, KNN>::compute_curvatures()
{
  if (!m_cloud) {
    return {};
  }

  const std::size_t num_points = m_cloud->size();
  std::vector<CurvatureInfo> curvatures(num_points);

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
            [this, &curvatures, start_idx, end_idx]() {
              compute_curvatures_range(curvatures, start_idx, end_idx);
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
    compute_curvatures_range(curvatures, 0, num_points);
  }

  return curvatures;
}

template<typename DataType, typename KNN>
std::vector<uint8_t> loam_feature_extractor_t<DataType, KNN>::classify_features(
    const std::vector<CurvatureInfo>& curvatures)
{
  std::vector<uint8_t> labels(curvatures.size(), static_cast<uint8_t>(feature_label::none));

  for (std::size_t i = 0; i < curvatures.size(); ++i) {
    if (!curvatures[i].is_valid) {
      continue;
    }
    
    const data_type curvature = curvatures[i].curvature;
    
    if (curvature < m_curvature_threshold) {
      // Too low curvature, skip
      continue;
    }
    
    if (curvature > m_edge_threshold) {
      // High curvature -> edge point
      labels[i] = static_cast<uint8_t>(feature_label::edge);
    } else if (curvature < m_planar_threshold) {
      // Low curvature -> planar point
      labels[i] = static_cast<uint8_t>(feature_label::planar);
    }
    // else: remains as none
  }
  
  return labels;
}

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::indices_vector
loam_feature_extractor_t<DataType, KNN>::extract_impl()
{
  if (!m_cloud || !m_knn) {
    return {};
  }

  // Compute curvatures
  auto curvatures = compute_curvatures();
  
  // Get all feature indices (both edge and planar)
  indices_vector feature_indices;
  
  for (std::size_t i = 0; i < curvatures.size(); ++i) {
    if (!curvatures[i].is_valid) continue;
    
    const data_type curvature = curvatures[i].curvature;
    if (curvature < m_curvature_threshold) continue;
    
    if (curvature > m_edge_threshold || curvature < m_planar_threshold) {
      feature_indices.push_back(i);
    }
  }
  
  return feature_indices;
}

template<typename DataType, typename KNN>
void loam_feature_extractor_t<DataType, KNN>::extract_impl(indices_vector& keypoint_indices)
{
  keypoint_indices = extract_impl();
}

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::point_cloud
loam_feature_extractor_t<DataType, KNN>::extract_keypoints_impl()
{
  if (!m_cloud || !m_knn) {
    return point_cloud{};
  }

  // For compatibility with base class, return all feature points
  auto feature_indices = extract_impl();
  
  point_cloud features;
  features.points.reserve(feature_indices.size());
  
  for (const auto& idx : feature_indices) {
    features.points.push_back(m_cloud->points[idx]);
  }
  
  return features;
}

template<typename DataType, typename KNN>
void loam_feature_extractor_t<DataType, KNN>::extract_keypoints_impl(point_cloud_ptr output)
{
  auto features = extract_keypoints_impl();
  *output = std::move(features);
}

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::loam_result
loam_feature_extractor_t<DataType, KNN>::extract_labeled_cloud()
{
  loam_result result;
  
  if (!m_cloud || !m_knn) {
    return result;
  }

  // Copy the cloud
  result.cloud = *m_cloud;
  
  // Compute curvatures
  auto curvatures = compute_curvatures();
  
  // Classify features
  result.labels = classify_features(curvatures);
  
  return result;
}

// Static utility methods

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::point_cloud
loam_feature_extractor_t<DataType, KNN>::extract_edge_points(const loam_result& result)
{
  point_cloud edges;
  for (std::size_t i = 0; i < result.cloud.size() && i < result.labels.size(); ++i) {
    if (static_cast<feature_label>(result.labels[i]) == feature_label::edge) {
      edges.points.push_back(result.cloud.points[i]);
    }
  }
  return edges;
}

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::point_cloud
loam_feature_extractor_t<DataType, KNN>::extract_planar_points(const loam_result& result)
{
  point_cloud planars;
  for (std::size_t i = 0; i < result.cloud.size() && i < result.labels.size(); ++i) {
    if (static_cast<feature_label>(result.labels[i]) == feature_label::planar) {
      planars.points.push_back(result.cloud.points[i]);
    }
  }
  return planars;
}

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::point_cloud
loam_feature_extractor_t<DataType, KNN>::extract_non_feature_points(const loam_result& result)
{
  point_cloud non_features;
  for (std::size_t i = 0; i < result.cloud.size() && i < result.labels.size(); ++i) {
    if (static_cast<feature_label>(result.labels[i]) == feature_label::none) {
      non_features.points.push_back(result.cloud.points[i]);
    }
  }
  return non_features;
}

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::indices_vector
loam_feature_extractor_t<DataType, KNN>::extract_edge_indices(const std::vector<uint8_t>& labels)
{
  indices_vector indices;
  for (std::size_t i = 0; i < labels.size(); ++i) {
    if (static_cast<feature_label>(labels[i]) == feature_label::edge) {
      indices.push_back(i);
    }
  }
  return indices;
}

template<typename DataType, typename KNN>
typename loam_feature_extractor_t<DataType, KNN>::indices_vector
loam_feature_extractor_t<DataType, KNN>::extract_planar_indices(const std::vector<uint8_t>& labels)
{
  indices_vector indices;
  for (std::size_t i = 0; i < labels.size(); ++i) {
    if (static_cast<feature_label>(labels[i]) == feature_label::planar) {
      indices.push_back(i);
    }
  }
  return indices;
}

}  // namespace toolbox::pcl