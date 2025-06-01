#pragma once

#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <numeric>
#include <cpp-toolbox/concurrent/parallel.hpp>

namespace toolbox::pcl
{

template<typename DataType, typename KNN>
std::size_t fpfh_extractor_t<DataType, KNN>::set_input(const point_cloud& cloud)
{
  m_cloud = std::make_shared<point_cloud>(cloud);
  return m_cloud->size();
}

template<typename DataType, typename KNN>
std::size_t fpfh_extractor_t<DataType, KNN>::set_input(const point_cloud_ptr& cloud)
{
  m_cloud = cloud;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t fpfh_extractor_t<DataType, KNN>::set_knn(const knn_type& knn)
{
  m_knn = const_cast<knn_type*>(&knn);
  if (m_cloud && m_knn)
  {
    // Convert point cloud to vector of points for new KNN API
    std::vector<point_t<DataType>> points(m_cloud->points.begin(), m_cloud->points.end());
    m_knn->set_input(points);
  }
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t fpfh_extractor_t<DataType, KNN>::set_search_radius(data_type radius)
{
  m_search_radius = radius;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
std::size_t fpfh_extractor_t<DataType, KNN>::set_num_neighbors(std::size_t num_neighbors)
{
  m_num_neighbors = num_neighbors;
  return m_cloud ? m_cloud->size() : 0;
}

template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::set_normals(const point_cloud_ptr& normals)
{
  m_normals = normals;
}

template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::enable_parallel_impl(bool enable)
{
  m_enable_parallel = enable;
}

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

  // Step 1: Compute SPFH for all points
  std::vector<spfh_signature_t> spfh_features(cloud.size());
  
  auto compute_spfh_range = [&](std::size_t start, std::size_t end) {
    for (std::size_t i = start; i < end; ++i)
    {
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      
      m_knn->radius_neighbors(cloud.points[i], m_search_radius, neighbor_indices, neighbor_distances);
      
      // Limit to m_num_neighbors
      if (neighbor_indices.size() > m_num_neighbors)
      {
        neighbor_indices.resize(m_num_neighbors);
        neighbor_distances.resize(m_num_neighbors);
      }
      
      if (!neighbor_indices.empty())
      {
        compute_spfh(cloud, *normals, i, neighbor_indices, spfh_features[i]);
      }
    }
  };

  if (m_enable_parallel)
  {
    // Create index range
    std::vector<std::size_t> indices(cloud.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                          [&](std::size_t i) {
      std::vector<std::size_t> neighbor_indices;
      std::vector<data_type> neighbor_distances;
      
      m_knn->radius_neighbors(cloud.points[i], m_search_radius, neighbor_indices, neighbor_distances);
      
      // Limit to m_num_neighbors
      if (neighbor_indices.size() > m_num_neighbors)
      {
        neighbor_indices.resize(m_num_neighbors);
        neighbor_distances.resize(m_num_neighbors);
      }
      
      if (!neighbor_indices.empty())
      {
        compute_spfh(cloud, *normals, i, neighbor_indices, spfh_features[i]);
      }
    });
  }
  else
  {
    compute_spfh_range(0, cloud.size());
  }

  // Step 2: Compute FPFH for keypoints
  descriptors.resize(keypoint_indices.size());
  
  auto compute_fpfh_range = [&](std::size_t start, std::size_t end) {
    for (std::size_t i = start; i < end; ++i)
    {
      std::size_t keypoint_idx = keypoint_indices[i];
      compute_fpfh_feature(cloud, *normals, keypoint_idx, spfh_features, descriptors[i]);
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
      compute_fpfh_feature(cloud, *normals, keypoint_idx, spfh_features, descriptors[i]);
    });
  }
  else
  {
    compute_fpfh_range(0, keypoint_indices.size());
  }
}

template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_impl(
    const point_cloud& cloud,
    const std::vector<std::size_t>& keypoint_indices,
    std::unique_ptr<std::vector<signature_type>>& descriptors) const
{
  descriptors = std::make_unique<std::vector<signature_type>>();
  compute_impl(cloud, keypoint_indices, *descriptors);
}

template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_spfh(
    const point_cloud& cloud,
    const point_cloud& normals,
    std::size_t index,
    const std::vector<std::size_t>& neighbor_indices,
    spfh_signature_t& spfh) const
{
  const auto& p1 = cloud.points[index];
  const auto& n1 = normals.points[index];

  // Initialize histograms
  std::fill(spfh.f1.begin(), spfh.f1.end(), DataType(0));
  std::fill(spfh.f2.begin(), spfh.f2.end(), DataType(0));
  std::fill(spfh.f3.begin(), spfh.f3.end(), DataType(0));

  std::size_t valid_neighbors = 0;

  for (std::size_t neighbor_idx : neighbor_indices)
  {
    if (neighbor_idx == index) continue;

    const auto& p2 = cloud.points[neighbor_idx];
    const auto& n2 = normals.points[neighbor_idx];

    data_type f1, f2, f3;
    compute_pair_features(p1, n1, p2, n2, f1, f2, f3);

    // Compute histogram bins
    std::size_t bin_f1 = compute_bin_index(f1, DataType(-1), DataType(1), 11);
    std::size_t bin_f2 = compute_bin_index(f2, DataType(-1), DataType(1), 11);
    std::size_t bin_f3 = compute_bin_index(f3, DataType(-M_PI), DataType(M_PI), 11);

    spfh.f1[bin_f1] += DataType(1);
    spfh.f2[bin_f2] += DataType(1);
    spfh.f3[bin_f3] += DataType(1);
    
    valid_neighbors++;
  }

  // Normalize histograms
  if (valid_neighbors > 0)
  {
    data_type norm_factor = DataType(1) / static_cast<data_type>(valid_neighbors);
    for (std::size_t i = 0; i < 11; ++i)
    {
      spfh.f1[i] *= norm_factor;
      spfh.f2[i] *= norm_factor;
      spfh.f3[i] *= norm_factor;
    }
  }
}

template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_fpfh_feature(
    const point_cloud& cloud,
    const point_cloud& normals,
    std::size_t index,
    const std::vector<spfh_signature_t>& spfh_features,
    signature_type& fpfh) const
{
  // Initialize FPFH histogram
  std::fill(fpfh.histogram.begin(), fpfh.histogram.end(), DataType(0));

  // Get neighbors
  std::vector<std::size_t> neighbor_indices;
  std::vector<data_type> neighbor_distances;
  m_knn->radius_neighbors(cloud.points[index], m_search_radius, neighbor_indices, neighbor_distances);
  
  // Limit to m_num_neighbors
  if (neighbor_indices.size() > m_num_neighbors)
  {
    neighbor_indices.resize(m_num_neighbors);
    neighbor_distances.resize(m_num_neighbors);
  }

  if (neighbor_indices.empty()) return;

  // Copy own SPFH
  const auto& own_spfh = spfh_features[index];
  for (std::size_t i = 0; i < 11; ++i)
  {
    fpfh.histogram[i] = own_spfh.f1[i];
    fpfh.histogram[i + 11] = own_spfh.f2[i];
    fpfh.histogram[i + 22] = own_spfh.f3[i];
  }

  // Weight and accumulate neighbor SPFHs
  data_type weight_sum = DataType(0);
  
  for (std::size_t i = 0; i < neighbor_indices.size(); ++i)
  {
    std::size_t neighbor_idx = neighbor_indices[i];
    if (neighbor_idx == index) continue;

    // Use inverse distance as weight
    data_type weight = DataType(1) / (neighbor_distances[i] + DataType(1e-6));
    weight_sum += weight;

    const auto& neighbor_spfh = spfh_features[neighbor_idx];
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
}

template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_pair_features(
    const point_t<data_type>& p1,
    const point_t<data_type>& n1,
    const point_t<data_type>& p2,
    const point_t<data_type>& n2,
    data_type& f1,
    data_type& f2,
    data_type& f3) const
{
  // Compute the Darboux frame
  point_t<data_type> dp;
  dp.x = p2.x - p1.x;
  dp.y = p2.y - p1.y;
  dp.z = p2.z - p1.z;
  auto dp_normalized = dp.normalize();
  dp.x = static_cast<data_type>(dp_normalized.x);
  dp.y = static_cast<data_type>(dp_normalized.y);
  dp.z = static_cast<data_type>(dp_normalized.z);

  // u = n1
  point_t<data_type> u = n1;
  
  // v = (p2 - p1) x u
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
  
  // w = u x v
  point_t<data_type> w = u.cross(v);

  // Compute angles
  f1 = v.dot(n2);  // cos(alpha)
  f2 = u.dot(dp);  // cos(phi)
  f3 = std::atan2(w.dot(n2), u.dot(n2));  // atan2(w·n2, u·n2) = theta
}

template<typename DataType, typename KNN>
std::size_t fpfh_extractor_t<DataType, KNN>::compute_bin_index(
    data_type value, 
    data_type min_val, 
    data_type max_val, 
    std::size_t num_bins) const
{
  // Clamp value to range
  value = std::max(min_val, std::min(max_val, value));
  
  // Compute bin index
  data_type normalized = (value - min_val) / (max_val - min_val);
  std::size_t bin = static_cast<std::size_t>(normalized * num_bins);
  
  // Ensure bin is within valid range
  return std::min(bin, num_bins - 1);
}

}  // namespace toolbox::pcl