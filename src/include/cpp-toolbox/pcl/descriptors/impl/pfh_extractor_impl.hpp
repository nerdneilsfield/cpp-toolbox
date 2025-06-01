#pragma once

#include <cpp-toolbox/pcl/descriptors/pfh_extractor.hpp>
#include <numeric>
#include <cpp-toolbox/concurrent/parallel.hpp>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
std::size_t pfh_extractor_t<DataType, KNN>::set_input(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t pfh_extractor_t<DataType, KNN>::set_input(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t pfh_extractor_t<DataType, KNN>::set_knn(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud && m_knn)
  {
    // Use the points vector directly from the point cloud for new KNN API
    m_knn->set_input(m_cloud->points);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t pfh_extractor_t<DataType, KNN>::set_search_radius(data_type radius)
{
  m_search_radius = radius;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t pfh_extractor_t<DataType, KNN>::set_num_neighbors(std::size_t num_neighbors)
{
  m_num_neighbors = num_neighbors;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
void pfh_extractor_t<DataType, KNN>::set_normals(const point_cloud_ptr& normals)
{
  m_normals = normals;
}

template<typename DataType, typename KNN>
void pfh_extractor_t<DataType, KNN>::set_num_subdivisions(std::size_t subdivisions)
{
  m_num_subdivisions = subdivisions;
}

template<typename DataType, typename KNN>
void pfh_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

template<typename DataType, typename KNN>
void pfh_extractor_t<DataType, KNN>::compute_impl(
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

  descriptors.resize(keypoint_indices.size());

  auto compute_pfh_range = [&](std::size_t start, std::size_t end) {
    for (std::size_t i = start; i < end; ++i)
    {
      std::size_t keypoint_idx = keypoint_indices[i];
      
      // Get neighbors
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, neighbor_indices, neighbor_distances);
      
      // Limit to m_num_neighbors
      if (neighbor_indices.size() > m_num_neighbors)
      {
        neighbor_indices.resize(m_num_neighbors);
        neighbor_distances.resize(m_num_neighbors);
      }
      
      if (neighbor_indices.size() < 3)  // Need at least 3 neighbors
      {
        // Set empty descriptor
        std::fill(descriptors[i].histogram.begin(), descriptors[i].histogram.end(), DataType(0));
        continue;
      }

      // Compute PFH descriptor
      compute_pfh_feature(cloud, *normals, keypoint_idx, neighbor_indices, descriptors[i]);
    }
  };

  if (m_enable_parallel)
  {
    // Create index range for keypoints
    std::vector<std::size_t> indices(keypoint_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                          [&](std::size_t i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      
      // Get neighbors
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, neighbor_indices, neighbor_distances);
      
      // Limit to m_num_neighbors
      if (neighbor_indices.size() > m_num_neighbors)
      {
        neighbor_indices.resize(m_num_neighbors);
        neighbor_distances.resize(m_num_neighbors);
      }
      
      if (neighbor_indices.size() < 3)  // Need at least 3 neighbors
      {
        // Set empty descriptor
        std::fill(descriptors[i].histogram.begin(), descriptors[i].histogram.end(), DataType(0));
        return;
      }

      // Compute PFH descriptor
      compute_pfh_feature(cloud, *normals, keypoint_idx, neighbor_indices, descriptors[i]);
    });
  }
  else
  {
    compute_pfh_range(0, keypoint_indices.size());
  }
}

template<typename DataType, typename KNN>
void pfh_extractor_t<DataType, KNN>::compute_impl(
    const point_cloud& cloud,
    const std::vector<std::size_t>& keypoint_indices,
    std::unique_ptr<std::vector<signature_type>>& descriptors) const
{
  descriptors = std::make_unique<std::vector<signature_type>>();
  compute_impl(cloud, keypoint_indices, *descriptors);
}

template<typename DataType, typename KNN>
void pfh_extractor_t<DataType, KNN>::compute_pfh_feature(
    const point_cloud& cloud,
    const point_cloud& normals,
    std::size_t index,
    const std::vector<std::size_t>& neighbor_indices,
    signature_type& pfh) const
{
  // Initialize histogram
  std::fill(pfh.histogram.begin(), pfh.histogram.end(), DataType(0));

  // Compute features for all pairs of neighbors
  std::size_t num_pairs = 0;
  
  for (std::size_t i = 0; i < neighbor_indices.size(); ++i)
  {
    for (std::size_t j = i + 1; j < neighbor_indices.size(); ++j)
    {
      std::size_t idx1 = neighbor_indices[i];
      std::size_t idx2 = neighbor_indices[j];

      const auto& p1 = cloud.points[idx1];
      const auto& n1 = normals.points[idx1];
      const auto& p2 = cloud.points[idx2];
      const auto& n2 = normals.points[idx2];

      data_type f1, f2, f3, f4;
      compute_pair_features(p1, n1, p2, n2, f1, f2, f3, f4);

      // Compute histogram bin
      std::size_t bin_idx = compute_feature_bin_index(f1, f2, f3, f4);
      if (bin_idx < signature_type::HISTOGRAM_SIZE)
      {
        pfh.histogram[bin_idx] += DataType(1);
        num_pairs++;
      }
    }
  }

  // Normalize histogram
  if (num_pairs > 0)
  {
    data_type norm_factor = DataType(1) / static_cast<data_type>(num_pairs);
    for (std::size_t i = 0; i < signature_type::HISTOGRAM_SIZE; ++i)
    {
      pfh.histogram[i] *= norm_factor;
    }
  }
}

template<typename DataType, typename KNN>
void pfh_extractor_t<DataType, KNN>::compute_pair_features(
    const point_t<data_type>& p1,
    const point_t<data_type>& n1,
    const point_t<data_type>& p2,
    const point_t<data_type>& n2,
    data_type& f1,
    data_type& f2,
    data_type& f3,
    data_type& f4) const
{
  // Compute the Darboux frame
  point_t<data_type> dp;
  dp.x = p2.x - p1.x;
  dp.y = p2.y - p1.y;
  dp.z = p2.z - p1.z;
  data_type distance = dp.norm();
  
  if (distance < DataType(1e-6))
  {
    f1 = f2 = f3 = f4 = DataType(0);
    return;
  }
  
  auto dp_normalized = dp.normalize();
  dp.x = static_cast<data_type>(dp_normalized.x);
  dp.y = static_cast<data_type>(dp_normalized.y);
  dp.z = static_cast<data_type>(dp_normalized.z);

  // Source frame
  point_t<data_type> u = n1;
  point_t<data_type> v = dp.cross(u);
  data_type v_norm = v.norm();
  
  if (v_norm < DataType(1e-6))
  {
    // Points are aligned with normal, create arbitrary perpendicular vector
    if (std::abs(u.x) < DataType(0.9))
      v = point_t<data_type>(DataType(1), DataType(0), DataType(0)).cross(u);
    else
      v = point_t<data_type>(DataType(0), DataType(1), DataType(0)).cross(u);
    auto v_normalized = v.normalize();
    v.x = static_cast<data_type>(v_normalized.x);
    v.y = static_cast<data_type>(v_normalized.y);
    v.z = static_cast<data_type>(v_normalized.z);
  }
  else
  {
    auto v_normalized = v.normalize();
    v.x = static_cast<data_type>(v_normalized.x);
    v.y = static_cast<data_type>(v_normalized.y);
    v.z = static_cast<data_type>(v_normalized.z);
  }
  
  point_t<data_type> w = u.cross(v);

  // Compute the angles
  data_type n2_u = n2.dot(u);
  data_type n2_v = n2.dot(v);
  data_type n2_w = n2.dot(w);
  data_type n1_dp = n1.dot(dp);

  // Clamp values to avoid numerical errors
  n2_u = std::max(DataType(-1), std::min(DataType(1), n2_u));
  n1_dp = std::max(DataType(-1), std::min(DataType(1), n1_dp));

  // The four features
  f1 = n2_v;
  f2 = n2_u;
  f3 = std::atan2(n2_w, n2_u);  // atan2 gives angle in [-pi, pi]
  f4 = distance;

  // Normalize f4 to [0, 1] range (assuming max distance is search radius)
  f4 = std::min(f4 / m_search_radius, DataType(1));
}

template<typename DataType, typename KNN>
std::size_t pfh_extractor_t<DataType, KNN>::compute_feature_bin_index(
    data_type f1,
    data_type f2,
    data_type f3,
    data_type f4) const
{
  // Map features to bin indices
  // f1, f2 are in [-1, 1]
  // f3 is in [-pi, pi]
  // f4 is in [0, 1]
  
  // For simplicity, we only use f1, f2, f3 (as in original PFH)
  // Each feature is divided into m_num_subdivisions bins
  
  // Map f1 from [-1, 1] to [0, m_num_subdivisions)
  std::size_t bin_f1 = static_cast<std::size_t>((f1 + DataType(1)) * DataType(0.5) * m_num_subdivisions);
  bin_f1 = std::min(bin_f1, m_num_subdivisions - 1);

  // Map f2 from [-1, 1] to [0, m_num_subdivisions)
  std::size_t bin_f2 = static_cast<std::size_t>((f2 + DataType(1)) * DataType(0.5) * m_num_subdivisions);
  bin_f2 = std::min(bin_f2, m_num_subdivisions - 1);

  // Map f3 from [-pi, pi] to [0, m_num_subdivisions)
  std::size_t bin_f3 = static_cast<std::size_t>((f3 + DataType(M_PI)) / (DataType(2) * DataType(M_PI)) * m_num_subdivisions);
  bin_f3 = std::min(bin_f3, m_num_subdivisions - 1);

  // Compute linear index
  return bin_f1 * m_num_subdivisions * m_num_subdivisions + 
         bin_f2 * m_num_subdivisions + 
         bin_f3;
}

}  // namespace toolbox::pcl