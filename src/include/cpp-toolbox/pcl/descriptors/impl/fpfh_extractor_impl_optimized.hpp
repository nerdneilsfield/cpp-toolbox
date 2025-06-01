#pragma once

#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <numeric>
#include <unordered_set>
#include <unordered_map>
#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>

namespace toolbox::pcl
{

// Optimized version that only computes SPFH for necessary points
template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_impl_optimized(
    const point_cloud& cloud,
    const std::vector<std::size_t>& keypoint_indices,
    std::vector<signature_type>& descriptors) const
{
  if (!m_knn || keypoint_indices.empty())
  {
    descriptors.clear();
    return;
  }

  // Compute normals if not provided
  point_cloud_ptr normals = m_normals;
  if (!normals)
  {
    normals = std::make_shared<point_cloud>();
    normals->points.resize(cloud.size());
    pca_norm_extractor_t<data_type, knn_type> norm_extractor;
    norm_extractor.set_input(cloud);
    norm_extractor.set_knn(*m_knn);
    norm_extractor.set_num_neighbors(m_num_neighbors);
    norm_extractor.enable_parallel(m_enable_parallel);
    norm_extractor.extract(normals);
  }

  // Step 1: Find all points that need SPFH computation
  std::unordered_set<std::size_t> points_needing_spfh;
  
  // First pass: add keypoints and find their neighbors
  for (auto keypoint_idx : keypoint_indices)
  {
    points_needing_spfh.insert(keypoint_idx);
    
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, neighbor_indices, neighbor_distances);
    
    // Limit neighbors
    std::size_t num_neighbors = std::min(neighbor_indices.size(), m_num_neighbors);
    for (std::size_t i = 0; i < num_neighbors; ++i)
    {
      points_needing_spfh.insert(neighbor_indices[i]);
    }
  }
  
  // Convert to vector for indexing
  std::vector<std::size_t> spfh_point_indices(points_needing_spfh.begin(), points_needing_spfh.end());
  
  // Create mapping from original point index to SPFH array index
  std::unordered_map<std::size_t, std::size_t> point_to_spfh_idx;
  for (std::size_t i = 0; i < spfh_point_indices.size(); ++i)
  {
    point_to_spfh_idx[spfh_point_indices[i]] = i;
  }
  
  // Step 2: Compute SPFH only for necessary points
  std::vector<spfh_signature_t> spfh_features(spfh_point_indices.size());
  
  auto compute_spfh_for_points = [&](const std::vector<std::size_t>& indices) {
    for (std::size_t i : indices)
    {
      std::size_t point_idx = spfh_point_indices[i];
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      
      m_knn->radius_neighbors(cloud.points[point_idx], m_search_radius, neighbor_indices, neighbor_distances);
      
      if (neighbor_indices.size() > m_num_neighbors)
      {
        neighbor_indices.resize(m_num_neighbors);
      }
      
      if (!neighbor_indices.empty())
      {
        compute_spfh(cloud, *normals, point_idx, neighbor_indices, spfh_features[i]);
      }
    }
  };
  
  if (m_enable_parallel)
  {
    std::vector<std::size_t> indices(spfh_point_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
        [&](std::size_t i) {
          std::size_t point_idx = spfh_point_indices[i];
          std::vector<std::size_t> neighbor_indices;
          std::vector<data_type> neighbor_distances;
          
          m_knn->radius_neighbors(cloud.points[point_idx], m_search_radius, neighbor_indices, neighbor_distances);
          
          if (neighbor_indices.size() > m_num_neighbors)
          {
            neighbor_indices.resize(m_num_neighbors);
          }
          
          if (!neighbor_indices.empty())
          {
            compute_spfh(cloud, *normals, point_idx, neighbor_indices, spfh_features[i]);
          }
        });
  }
  else
  {
    std::vector<std::size_t> indices(spfh_point_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    compute_spfh_for_points(indices);
  }

  // Step 3: Compute FPFH for keypoints
  descriptors.resize(keypoint_indices.size());
  
  auto compute_fpfh_for_keypoint = [&](std::size_t idx) {
    std::size_t keypoint_idx = keypoint_indices[idx];
    auto& fpfh = descriptors[idx];
    
    // Initialize FPFH histogram
    std::fill(fpfh.histogram.begin(), fpfh.histogram.end(), DataType(0));
    
    // Get neighbors
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, neighbor_indices, neighbor_distances);
    
    if (neighbor_indices.size() > m_num_neighbors)
    {
      neighbor_indices.resize(m_num_neighbors);
      neighbor_distances.resize(m_num_neighbors);
    }
    
    if (neighbor_indices.empty()) return;
    
    // Copy own SPFH
    auto it = point_to_spfh_idx.find(keypoint_idx);
    if (it != point_to_spfh_idx.end())
    {
      const auto& own_spfh = spfh_features[it->second];
      for (std::size_t i = 0; i < 11; ++i)
      {
        fpfh.histogram[i] = own_spfh.f1[i];
        fpfh.histogram[i + 11] = own_spfh.f2[i];
        fpfh.histogram[i + 22] = own_spfh.f3[i];
      }
    }
    
    // Weight and accumulate neighbor SPFHs
    data_type weight_sum = DataType(0);
    
    for (std::size_t i = 0; i < neighbor_indices.size(); ++i)
    {
      std::size_t neighbor_idx = neighbor_indices[i];
      if (neighbor_idx == keypoint_idx) continue;
      
      auto neighbor_it = point_to_spfh_idx.find(neighbor_idx);
      if (neighbor_it == point_to_spfh_idx.end()) continue;
      
      // Use inverse distance as weight
      data_type weight = DataType(1) / (neighbor_distances[i] + DataType(1e-6));
      weight_sum += weight;
      
      const auto& neighbor_spfh = spfh_features[neighbor_it->second];
      for (std::size_t j = 0; j < 11; ++j)
      {
        fpfh.histogram[j] += weight * neighbor_spfh.f1[j];
        fpfh.histogram[j + 11] += weight * neighbor_spfh.f2[j];
        fpfh.histogram[j + 22] += weight * neighbor_spfh.f3[j];
      }
    }
    
    // Normalize the final histogram
    if (weight_sum > DataType(0))
    {
      data_type norm_factor = DataType(1) / (DataType(1) + weight_sum);
      for (std::size_t i = 0; i < 33; ++i)
      {
        fpfh.histogram[i] *= norm_factor;
      }
    }
  };
  
  if (m_enable_parallel)
  {
    std::vector<std::size_t> indices(keypoint_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(), compute_fpfh_for_keypoint);
  }
  else
  {
    for (std::size_t i = 0; i < keypoint_indices.size(); ++i)
    {
      compute_fpfh_for_keypoint(i);
    }
  }
}

}  // namespace toolbox::pcl