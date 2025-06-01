#pragma once

#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <numeric>
#include <unordered_set>
#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/pcl/norm/pca_norm.hpp>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_impl(
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
  // This includes keypoints and their neighbors
  std::unordered_set<std::size_t> points_needing_spfh;
  
  // Add keypoints
  for (auto idx : keypoint_indices)
  {
    points_needing_spfh.insert(idx);
  }
  
  // Add neighbors of keypoints
  for (auto keypoint_idx : keypoint_indices)
  {
    std::vector<std::size_t> neighbor_indices;
    std::vector<data_type> neighbor_distances;
    m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, neighbor_indices, neighbor_distances);
    
    // Limit neighbors
    if (neighbor_indices.size() > m_num_neighbors)
    {
      neighbor_indices.resize(m_num_neighbors);
    }
    
    for (auto neighbor_idx : neighbor_indices)
    {
      points_needing_spfh.insert(neighbor_idx);
    }
  }
  
  // Convert to vector for parallel processing
  std::vector<std::size_t> spfh_indices(points_needing_spfh.begin(), points_needing_spfh.end());
  
  // Create mapping from point index to SPFH index
  std::unordered_map<std::size_t, std::size_t> point_to_spfh_idx;
  for (std::size_t i = 0; i < spfh_indices.size(); ++i)
  {
    point_to_spfh_idx[spfh_indices[i]] = i;
  }
  
  // Step 2: Compute SPFH only for necessary points
  std::vector<spfh_signature_t> spfh_features(spfh_indices.size());
  
  if (m_enable_parallel)
  {
    toolbox::concurrent::parallel_for_each(
        spfh_indices.begin(), spfh_indices.end(),
        [&](std::size_t point_idx) {
          std::vector<std::size_t> neighbor_indices;
          std::vector<data_type> neighbor_distances;
          
          m_knn->radius_neighbors(cloud.points[point_idx], m_search_radius, neighbor_indices, neighbor_distances);
          
          if (neighbor_indices.size() > m_num_neighbors)
          {
            neighbor_indices.resize(m_num_neighbors);
          }
          
          if (!neighbor_indices.empty())
          {
            auto spfh_idx = point_to_spfh_idx[point_idx];
            compute_spfh(cloud, *normals, point_idx, neighbor_indices, spfh_features[spfh_idx]);
          }
        });
  }
  else
  {
    for (std::size_t i = 0; i < spfh_indices.size(); ++i)
    {
      std::size_t point_idx = spfh_indices[i];
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
  }

  // Step 3: Compute FPFH for keypoints using the optimized SPFH data
  descriptors.resize(keypoint_indices.size());
  
  if (m_enable_parallel)
  {
    std::vector<std::size_t> indices(keypoint_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(
        indices.begin(), indices.end(),
        [&](std::size_t i) {
          std::size_t keypoint_idx = keypoint_indices[i];
          
          // Get neighbors
          std::vector<std::size_t> neighbor_indices;
          std::vector<data_type> neighbor_distances;
          m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, neighbor_indices, neighbor_distances);
          
          if (neighbor_indices.size() > m_num_neighbors)
          {
            neighbor_indices.resize(m_num_neighbors);
          }
          
          if (neighbor_indices.empty())
          {
            std::fill(descriptors[i].histogram.begin(), descriptors[i].histogram.end(), DataType(0));
            return;
          }
          
          // Initialize FPFH
          std::fill(descriptors[i].histogram.begin(), descriptors[i].histogram.end(), DataType(0));
          
          // Copy own SPFH
          auto keypoint_spfh_idx = point_to_spfh_idx[keypoint_idx];
          const auto& own_spfh = spfh_features[keypoint_spfh_idx];
          for (std::size_t j = 0; j < 11; ++j)
          {
            descriptors[i].histogram[j] = own_spfh.f1[j];
            descriptors[i].histogram[j + 11] = own_spfh.f2[j];
            descriptors[i].histogram[j + 22] = own_spfh.f3[j];
          }
          
          // Weight and accumulate neighbor SPFHs
          data_type weight_sum = DataType(0);
          
          for (std::size_t j = 0; j < neighbor_indices.size(); ++j)
          {
            std::size_t neighbor_idx = neighbor_indices[j];
            if (neighbor_idx == keypoint_idx) continue;
            
            // Check if neighbor has SPFH computed
            auto it = point_to_spfh_idx.find(neighbor_idx);
            if (it == point_to_spfh_idx.end()) continue;
            
            // Use inverse distance as weight
            data_type weight = DataType(1) / (neighbor_distances[j] + DataType(1e-6));
            weight_sum += weight;
            
            const auto& neighbor_spfh = spfh_features[it->second];
            for (std::size_t k = 0; k < 11; ++k)
            {
              descriptors[i].histogram[k] += weight * neighbor_spfh.f1[k];
              descriptors[i].histogram[k + 11] += weight * neighbor_spfh.f2[k];
              descriptors[i].histogram[k + 22] += weight * neighbor_spfh.f3[k];
            }
          }
          
          // Normalize
          if (weight_sum > DataType(0))
          {
            data_type norm_factor = DataType(1) / (DataType(1) + weight_sum);
            for (std::size_t j = 0; j < 33; ++j)
            {
              descriptors[i].histogram[j] *= norm_factor;
            }
          }
        });
  }
  else
  {
    for (std::size_t i = 0; i < keypoint_indices.size(); ++i)
    {
      std::size_t keypoint_idx = keypoint_indices[i];
      compute_fpfh_feature_optimized(cloud, *normals, keypoint_idx, spfh_features, point_to_spfh_idx, descriptors[i]);
    }
  }
}

}  // namespace toolbox::pcl