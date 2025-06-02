#pragma once

#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <numeric>
#include <unordered_set>
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

  // 计算法向量（如果未提供）/ Compute normals if not provided
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

  // 步骤1：缓存所有关键点的邻居信息 / Step 1: Cache neighbor information for all keypoints
  std::vector<neighbor_info_t<data_type>> keypoint_neighbors(keypoint_indices.size());
  
  // 预分配邻居缓存 / Pre-allocate neighbor cache
  for (auto& neighbor_info : keypoint_neighbors) {
    neighbor_info.reserve(m_num_neighbors);
  }

  auto cache_neighbors = [&](std::size_t start, std::size_t end) {
    for (std::size_t i = start; i < end; ++i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      auto& neighbor_info = keypoint_neighbors[i];
      
      m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, 
                             neighbor_info.indices, neighbor_info.distances);
      
      // 限制邻居数量 / Limit number of neighbors
      if (neighbor_info.indices.size() > m_num_neighbors) {
        neighbor_info.indices.resize(m_num_neighbors);
        neighbor_info.distances.resize(m_num_neighbors);
      }
      neighbor_info.computed = true;
    }
  };

  if (m_enable_parallel) {
    std::vector<std::size_t> indices(keypoint_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                          [&](std::size_t i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      auto& neighbor_info = keypoint_neighbors[i];
      
      m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, 
                             neighbor_info.indices, neighbor_info.distances);
      
      if (neighbor_info.indices.size() > m_num_neighbors) {
        neighbor_info.indices.resize(m_num_neighbors);
        neighbor_info.distances.resize(m_num_neighbors);
      }
      neighbor_info.computed = true;
    });
  } else {
    cache_neighbors(0, keypoint_indices.size());
  }

  // 步骤2：确定需要计算SPFH的点 / Step 2: Determine which points need SPFH computation
  spfh_cache_manager_t<data_type, knn_type> spfh_cache(cloud.size());
  std::unordered_set<std::size_t> unique_points;
  
  // 收集所有需要SPFH的点（关键点和它们的邻居）/ Collect all points that need SPFH
  for (std::size_t i = 0; i < keypoint_indices.size(); ++i) {
    std::size_t keypoint_idx = keypoint_indices[i];
    unique_points.insert(keypoint_idx);
    
    const auto& neighbor_info = keypoint_neighbors[i];
    for (std::size_t neighbor_idx : neighbor_info.indices) {
      unique_points.insert(neighbor_idx);
    }
  }
  
  // 标记需要计算SPFH的点 / Mark points that need SPFH computation
  for (std::size_t point_idx : unique_points) {
    spfh_cache.mark_needed(point_idx);
  }

  // 步骤3：批量计算所需的SPFH / Step 3: Batch compute required SPFHs
  auto needed_points = spfh_cache.get_needed_points();
  
  // 线程局部存储避免false sharing / Thread-local storage to avoid false sharing
  struct thread_local_data_t {
    std::vector<std::size_t> temp_indices;
    std::vector<data_type> temp_distances;
    
    thread_local_data_t() {
      temp_indices.reserve(256);  // 预分配临时缓存 / Pre-allocate temp cache
      temp_distances.reserve(256);
    }
  };

  auto compute_spfh_batch = [&](std::size_t start, std::size_t end) {
    thread_local thread_local_data_t tld;
    
    for (std::size_t i = start; i < end; ++i) {
      std::size_t point_idx = needed_points[i];
      
      // 重用临时缓存避免重复分配 / Reuse temp cache to avoid repeated allocation
      tld.temp_indices.clear();
      tld.temp_distances.clear();
      
      m_knn->radius_neighbors(cloud.points[point_idx], m_search_radius, 
                             tld.temp_indices, tld.temp_distances);
      
      if (tld.temp_indices.size() > m_num_neighbors) {
        tld.temp_indices.resize(m_num_neighbors);
        tld.temp_distances.resize(m_num_neighbors);
      }
      
      if (!tld.temp_indices.empty()) {
        compute_spfh(cloud, *normals, point_idx, tld.temp_indices, spfh_cache.get_spfh(point_idx));
      }
    }
  };

  if (m_enable_parallel) {
    std::vector<std::size_t> indices(needed_points.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                          [&](std::size_t i) {
      thread_local thread_local_data_t tld;
      std::size_t point_idx = needed_points[i];
      
      tld.temp_indices.clear();
      tld.temp_distances.clear();
      
      m_knn->radius_neighbors(cloud.points[point_idx], m_search_radius, 
                             tld.temp_indices, tld.temp_distances);
      
      if (tld.temp_indices.size() > m_num_neighbors) {
        tld.temp_indices.resize(m_num_neighbors);
        tld.temp_distances.resize(m_num_neighbors);
      }
      
      if (!tld.temp_indices.empty()) {
        compute_spfh(cloud, *normals, point_idx, tld.temp_indices, spfh_cache.get_spfh(point_idx));
      }
    });
  } else {
    compute_spfh_batch(0, needed_points.size());
  }

  // 步骤4：使用缓存的邻居信息计算FPFH / Step 4: Compute FPFH using cached neighbor information
  descriptors.resize(keypoint_indices.size());
  
  auto compute_fpfh_batch = [&](std::size_t start, std::size_t end) {
    for (std::size_t i = start; i < end; ++i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      const auto& neighbor_info = keypoint_neighbors[i];
      
      compute_fpfh_feature_optimized(cloud, *normals, keypoint_idx, 
                                   neighbor_info, spfh_cache, descriptors[i]);
    }
  };

  if (m_enable_parallel) {
    std::vector<std::size_t> indices(keypoint_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                          [&](std::size_t i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      const auto& neighbor_info = keypoint_neighbors[i];
      
      compute_fpfh_feature_optimized(cloud, *normals, keypoint_idx, 
                                   neighbor_info, spfh_cache, descriptors[i]);
    });
  } else {
    compute_fpfh_batch(0, keypoint_indices.size());
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

/**
 * @brief 优化的FPFH特征计算（使用缓存的邻居信息和SPFH）
 * Optimized FPFH feature computation using cached neighbor info and SPFH
 */
template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_fpfh_feature_optimized(
    const point_cloud& cloud,
    const point_cloud& normals,
    std::size_t index,
    const neighbor_info_t<data_type>& neighbor_info,
    const spfh_cache_manager_t<data_type, knn_type>& spfh_cache,
    signature_type& fpfh) const
{
  // 初始化FPFH直方图 / Initialize FPFH histogram
  std::fill(fpfh.histogram.begin(), fpfh.histogram.end(), DataType(0));

  if (neighbor_info.indices.empty()) return;

  // 复制自身的SPFH / Copy own SPFH
  const auto& own_spfh = spfh_cache.get_spfh(index);
  for (std::size_t i = 0; i < 11; ++i)
  {
    fpfh.histogram[i] = own_spfh.f1[i];
    fpfh.histogram[i + 11] = own_spfh.f2[i];
    fpfh.histogram[i + 22] = own_spfh.f3[i];
  }

  // 加权累积邻居的SPFH / Weight and accumulate neighbor SPFHs
  data_type weight_sum = DataType(0);
  
  for (std::size_t i = 0; i < neighbor_info.indices.size(); ++i)
  {
    std::size_t neighbor_idx = neighbor_info.indices[i];
    if (neighbor_idx == index) continue;

    // 使用距离倒数作为权重 / Use inverse distance as weight
    data_type weight = DataType(1) / (neighbor_info.distances[i] + DataType(1e-6));
    weight_sum += weight;

    const auto& neighbor_spfh = spfh_cache.get_spfh(neighbor_idx);
    
    // 向量化累积操作 / Vectorized accumulation operation
    for (std::size_t j = 0; j < 11; ++j)
    {
      fpfh.histogram[j] += weight * neighbor_spfh.f1[j];
      fpfh.histogram[j + 11] += weight * neighbor_spfh.f2[j];
      fpfh.histogram[j + 22] += weight * neighbor_spfh.f3[j];
    }
  }

  // 归一化最终直方图 / Normalize the final histogram
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