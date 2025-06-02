#pragma once

#include <cpp-toolbox/pcl/descriptors/fpfh_extractor.hpp>
#include <numeric>
#include <algorithm>
#include <iostream>
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

  // 计算密度以决定算法策略 / Calculate density to determine algorithm strategy
  data_type density = static_cast<data_type>(keypoint_indices.size()) / static_cast<data_type>(cloud.size());
  
  // Debug output
  // std::cout << "[FPFH] Density: " << density << " (" << keypoint_indices.size() << "/" << cloud.size() << ")\n";
  
  // 对于低密度情况，使用直接计算避免缓存开销 / For low density, use direct computation to avoid cache overhead
  if (density < DataType(0.01) && keypoint_indices.size() < 100) {
    // std::cout << "[FPFH] Using direct computation mode\n";
    // 直接计算模式 - 类似PFH但使用FPFH的简化计算 / Direct computation mode - similar to PFH but with FPFH's simplified calculation
    descriptors.resize(keypoint_indices.size());
    
    auto compute_direct = [&](std::size_t start, std::size_t end) {
      std::vector<std::size_t> temp_indices;
      std::vector<data_type> temp_distances;
      temp_indices.reserve(m_num_neighbors);
      temp_distances.reserve(m_num_neighbors);
      
      for (std::size_t i = start; i < end; ++i) {
        std::size_t keypoint_idx = keypoint_indices[i];
        temp_indices.clear();
        temp_distances.clear();
        
        m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, temp_indices, temp_distances);
        if (temp_indices.size() > m_num_neighbors) {
          temp_indices.resize(m_num_neighbors);
          temp_distances.resize(m_num_neighbors);
        }
        
        // 直接计算FPFH，不预计算SPFH / Compute FPFH directly without pre-computing SPFH
        compute_fpfh_direct(cloud, *normals, keypoint_idx, temp_indices, temp_distances, descriptors[i]);
      }
    };
    
    if (m_enable_parallel) {
      std::vector<std::size_t> indices(keypoint_indices.size());
      std::iota(indices.begin(), indices.end(), 0);
      toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                            [&](std::size_t i) {
        std::vector<std::size_t> temp_indices;
        std::vector<data_type> temp_distances;
        temp_indices.reserve(m_num_neighbors);
        temp_distances.reserve(m_num_neighbors);
        
        std::size_t keypoint_idx = keypoint_indices[i];
        m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius, temp_indices, temp_distances);
        if (temp_indices.size() > m_num_neighbors) {
          temp_indices.resize(m_num_neighbors);
          temp_distances.resize(m_num_neighbors);
        }
        compute_fpfh_direct(cloud, *normals, keypoint_idx, temp_indices, temp_distances, descriptors[i]);
      });
    } else {
      compute_direct(0, keypoint_indices.size());
    }
    return;
  }
  
  // std::cout << "[FPFH] Using optimized caching strategy\n";
  // 高密度情况使用优化的缓存策硅 / High density case uses optimized caching strategy
  
  // 步骤1：收集所有邻居信息并统计引用次数 / Step 1: Collect all neighbor info and count references
  // 使用已定义的neighbor_info_t类型 / Use already defined neighbor_info_t type
  std::vector<neighbor_info_t<data_type>> all_neighbors(cloud.size());
  std::vector<int> reference_count(cloud.size(), 0);
  
  // 首先为关键点缓存邻居并统计引用 / First cache neighbors for keypoints and count references
  std::vector<std::vector<std::size_t>> keypoint_neighbor_indices(keypoint_indices.size());
  std::vector<std::vector<data_type>> keypoint_neighbor_distances(keypoint_indices.size());
  
  for (std::size_t i = 0; i < keypoint_indices.size(); ++i) {
    keypoint_neighbor_indices[i].reserve(m_num_neighbors);
    keypoint_neighbor_distances[i].reserve(m_num_neighbors);
  }
  
  if (m_enable_parallel) {
    std::vector<std::size_t> indices(keypoint_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                          [&](std::size_t i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius,
                             keypoint_neighbor_indices[i], keypoint_neighbor_distances[i]);
      if (keypoint_neighbor_indices[i].size() > m_num_neighbors) {
        keypoint_neighbor_indices[i].resize(m_num_neighbors);
        keypoint_neighbor_distances[i].resize(m_num_neighbors);
      }
    });
  } else {
    for (std::size_t i = 0; i < keypoint_indices.size(); ++i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      m_knn->radius_neighbors(cloud.points[keypoint_idx], m_search_radius,
                             keypoint_neighbor_indices[i], keypoint_neighbor_distances[i]);
      if (keypoint_neighbor_indices[i].size() > m_num_neighbors) {
        keypoint_neighbor_indices[i].resize(m_num_neighbors);
        keypoint_neighbor_distances[i].resize(m_num_neighbors);
      }
    }
  }
  
  // 统计引用次数 / Count references
  for (std::size_t i = 0; i < keypoint_indices.size(); ++i) {
    reference_count[keypoint_indices[i]]++;
    for (std::size_t neighbor_idx : keypoint_neighbor_indices[i]) {
      reference_count[neighbor_idx]++;
    }
  }

  // 步骤2：只为被多次引用的点预计算SPFH / Step 2: Pre-compute SPFH only for points referenced multiple times
  std::vector<bool> needs_spfh(cloud.size(), false);
  std::vector<spfh_signature_t> spfh_array(cloud.size());
  std::vector<bool> spfh_computed(cloud.size(), false);
  
  // 收集需要预计算SPFH的点（引用次数>2） / Collect points needing SPFH pre-computation (reference count > 2)
  std::vector<std::size_t> points_to_precompute;
  points_to_precompute.reserve(keypoint_indices.size() * 2);  // 估计大小 / Estimated size
  
  for (std::size_t i = 0; i < cloud.size(); ++i) {
    if (reference_count[i] > 2) {
      needs_spfh[i] = true;
      points_to_precompute.push_back(i);
    }
  }

  // 步骤3：批量预计算高引用点的SPFH，并为其缓存邻居 / Step 3: Batch pre-compute SPFH for high-reference points and cache their neighbors
  if (!points_to_precompute.empty()) {
    auto compute_spfh_batch = [&](std::size_t start, std::size_t end) {
      for (std::size_t i = start; i < end; ++i) {
        std::size_t point_idx = points_to_precompute[i];
        
        // 检查是否已缓存邻居 / Check if neighbors are already cached
        if (!all_neighbors[point_idx].computed) {
          m_knn->radius_neighbors(cloud.points[point_idx], m_search_radius,
                                 all_neighbors[point_idx].indices, all_neighbors[point_idx].distances);
          if (all_neighbors[point_idx].indices.size() > m_num_neighbors) {
            all_neighbors[point_idx].indices.resize(m_num_neighbors);
            all_neighbors[point_idx].distances.resize(m_num_neighbors);
          }
          all_neighbors[point_idx].computed = true;
        }
        
        if (!all_neighbors[point_idx].indices.empty()) {
          compute_spfh(cloud, *normals, point_idx, all_neighbors[point_idx].indices, spfh_array[point_idx]);
          spfh_computed[point_idx] = true;
        }
      }
    };
    
    if (m_enable_parallel) {
      std::vector<std::size_t> indices(points_to_precompute.size());
      std::iota(indices.begin(), indices.end(), 0);
      toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                            [&](std::size_t i) {
        std::size_t point_idx = points_to_precompute[i];
        
        if (!all_neighbors[point_idx].computed) {
          m_knn->radius_neighbors(cloud.points[point_idx], m_search_radius,
                                 all_neighbors[point_idx].indices, all_neighbors[point_idx].distances);
          if (all_neighbors[point_idx].indices.size() > m_num_neighbors) {
            all_neighbors[point_idx].indices.resize(m_num_neighbors);
            all_neighbors[point_idx].distances.resize(m_num_neighbors);
          }
          all_neighbors[point_idx].computed = true;
        }
        
        if (!all_neighbors[point_idx].indices.empty()) {
          compute_spfh(cloud, *normals, point_idx, all_neighbors[point_idx].indices, spfh_array[point_idx]);
          spfh_computed[point_idx] = true;
        }
      });
    } else {
      compute_spfh_batch(0, points_to_precompute.size());
    }
  }

  // 步骤4：计算FPFH，混合使用预计算和延迟计算 / Step 4: Compute FPFH using mix of pre-computed and lazy computation
  descriptors.resize(keypoint_indices.size());
  
  auto compute_fpfh_batch = [&](std::size_t start, std::size_t end) {
    for (std::size_t i = start; i < end; ++i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      
      // 计算关键点的SPFH（如果还未计算） / Compute keypoint's SPFH if not yet computed
      if (!spfh_computed[keypoint_idx]) {
        if (!all_neighbors[keypoint_idx].computed) {
          // 使用已缓存的邻居信息 / Use cached neighbor info
          all_neighbors[keypoint_idx].indices = keypoint_neighbor_indices[i];
          all_neighbors[keypoint_idx].distances = keypoint_neighbor_distances[i];
          all_neighbors[keypoint_idx].computed = true;
        }
        compute_spfh(cloud, *normals, keypoint_idx, all_neighbors[keypoint_idx].indices, spfh_array[keypoint_idx]);
        spfh_computed[keypoint_idx] = true;
      }
      
      // 计算FPFH / Compute FPFH
      compute_fpfh_feature_adaptive(cloud, *normals, keypoint_idx,
                                   keypoint_neighbor_indices[i], keypoint_neighbor_distances[i],
                                   all_neighbors, spfh_array, spfh_computed, descriptors[i]);
    }
  };
  
  if (m_enable_parallel) {
    std::vector<std::size_t> indices(keypoint_indices.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    toolbox::concurrent::parallel_for_each(indices.begin(), indices.end(),
                                          [&](std::size_t i) {
      std::size_t keypoint_idx = keypoint_indices[i];
      
      // 使用原子操作确保线程安全的延迟计算 / Use atomic operations for thread-safe lazy computation
      if (!spfh_computed[keypoint_idx]) {
        // 这里需要加锁或使用原子操作 / Need locking or atomic operations here
        if (!all_neighbors[keypoint_idx].computed) {
          all_neighbors[keypoint_idx].indices = keypoint_neighbor_indices[i];
          all_neighbors[keypoint_idx].distances = keypoint_neighbor_distances[i];
          all_neighbors[keypoint_idx].computed = true;
        }
        compute_spfh(cloud, *normals, keypoint_idx, all_neighbors[keypoint_idx].indices, spfh_array[keypoint_idx]);
        spfh_computed[keypoint_idx] = true;
      }
      
      compute_fpfh_feature_adaptive(cloud, *normals, keypoint_idx,
                                   keypoint_neighbor_indices[i], keypoint_neighbor_distances[i],
                                   all_neighbors, spfh_array, spfh_computed, descriptors[i]);
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

/**
 * @brief 直接计算FPFH（低密度优化）/ Direct FPFH computation (low density optimization)
 */
template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_fpfh_direct(
    const point_cloud& cloud,
    const point_cloud& normals,
    std::size_t index,
    const std::vector<std::size_t>& neighbor_indices,
    const std::vector<data_type>& neighbor_distances,
    signature_type& fpfh) const
{
  // 初始化直方图 / Initialize histogram
  std::fill(fpfh.histogram.begin(), fpfh.histogram.end(), DataType(0));
  
  if (neighbor_indices.empty()) return;
  
  const auto& p1 = cloud.points[index];
  const auto& n1 = normals.points[index];
  
  // 计算自身的SPFH部分 / Compute own SPFH part
  std::size_t valid_neighbors = 0;
  for (std::size_t i = 0; i < neighbor_indices.size(); ++i)
  {
    std::size_t neighbor_idx = neighbor_indices[i];
    if (neighbor_idx == index) continue;
    
    const auto& p2 = cloud.points[neighbor_idx];
    const auto& n2 = normals.points[neighbor_idx];
    
    data_type f1, f2, f3;
    compute_pair_features(p1, n1, p2, n2, f1, f2, f3);
    
    std::size_t bin_f1 = compute_bin_index(f1, DataType(-1), DataType(1), 11);
    std::size_t bin_f2 = compute_bin_index(f2, DataType(-1), DataType(1), 11);
    std::size_t bin_f3 = compute_bin_index(f3, DataType(-M_PI), DataType(M_PI), 11);
    
    fpfh.histogram[bin_f1] += DataType(1);
    fpfh.histogram[bin_f2 + 11] += DataType(1);
    fpfh.histogram[bin_f3 + 22] += DataType(1);
    
    valid_neighbors++;
  }
  
  // 加权累积邻居的贡献 / Weight and accumulate neighbor contributions
  data_type total_weight = static_cast<data_type>(valid_neighbors);
  
  for (std::size_t i = 0; i < neighbor_indices.size(); ++i)
  {
    std::size_t neighbor_idx = neighbor_indices[i];
    if (neighbor_idx == index) continue;
    
    const auto& p2 = cloud.points[neighbor_idx];
    const auto& n2 = normals.points[neighbor_idx];
    
    // 使用距离作为权重 / Use distance as weight
    data_type weight = DataType(1) / (neighbor_distances[i] + DataType(1e-6));
    
    // 计算邻居的邻居特征（简化版） / Compute neighbor's neighbor features (simplified)
    std::vector<std::size_t> nn_indices;
    std::vector<data_type> nn_distances;
    m_knn->radius_neighbors(p2, m_search_radius, nn_indices, nn_distances);
    
    if (nn_indices.size() > m_num_neighbors) {
      nn_indices.resize(m_num_neighbors);
      nn_distances.resize(m_num_neighbors);
    }
    
    for (std::size_t nn_idx : nn_indices)
    {
      if (nn_idx == neighbor_idx) continue;
      
      const auto& p3 = cloud.points[nn_idx];
      const auto& n3 = normals.points[nn_idx];
      
      data_type f1, f2, f3;
      compute_pair_features(p2, n2, p3, n3, f1, f2, f3);
      
      std::size_t bin_f1 = compute_bin_index(f1, DataType(-1), DataType(1), 11);
      std::size_t bin_f2 = compute_bin_index(f2, DataType(-1), DataType(1), 11);
      std::size_t bin_f3 = compute_bin_index(f3, DataType(-M_PI), DataType(M_PI), 11);
      
      fpfh.histogram[bin_f1] += weight / nn_indices.size();
      fpfh.histogram[bin_f2 + 11] += weight / nn_indices.size();
      fpfh.histogram[bin_f3 + 22] += weight / nn_indices.size();
    }
    
    total_weight += weight;
  }
  
  // 归一化 / Normalize
  if (total_weight > DataType(0))
  {
    data_type norm_factor = DataType(1) / total_weight;
    for (std::size_t i = 0; i < 33; ++i)
    {
      fpfh.histogram[i] *= norm_factor;
    }
  }
}

/**
 * @brief 自适应FPFH计算 / Adaptive FPFH computation
 */
template<typename DataType, typename KNN>
void fpfh_extractor_t<DataType, KNN>::compute_fpfh_feature_adaptive(
    const point_cloud& cloud,
    const point_cloud& normals,
    std::size_t index,
    const std::vector<std::size_t>& neighbor_indices,
    const std::vector<data_type>& neighbor_distances,
    std::vector<neighbor_info_t<data_type>>& all_neighbors,
    std::vector<spfh_signature_t>& spfh_array,
    std::vector<bool>& spfh_computed,
    signature_type& fpfh) const
{
  // 初始化 / Initialize
  std::fill(fpfh.histogram.begin(), fpfh.histogram.end(), DataType(0));
  
  if (neighbor_indices.empty()) return;
  
  // 复制自身的SPFH / Copy own SPFH
  const auto& own_spfh = spfh_array[index];
  for (std::size_t i = 0; i < 11; ++i)
  {
    fpfh.histogram[i] = own_spfh.f1[i];
    fpfh.histogram[i + 11] = own_spfh.f2[i];
    fpfh.histogram[i + 22] = own_spfh.f3[i];
  }
  
  // 加权累积邻居的SPFH / Weight and accumulate neighbor SPFHs
  data_type weight_sum = DataType(0);
  
  for (std::size_t i = 0; i < neighbor_indices.size(); ++i)
  {
    std::size_t neighbor_idx = neighbor_indices[i];
    if (neighbor_idx == index) continue;
    
    // 延迟计算邻居的SPFH（如果需要） / Lazy compute neighbor's SPFH if needed
    if (!spfh_computed[neighbor_idx])
    {
      // 检查是否有缓存的邻居信息 / Check if we have cached neighbor info
      if (!all_neighbors[neighbor_idx].computed)
      {
        m_knn->radius_neighbors(cloud.points[neighbor_idx], m_search_radius,
                               all_neighbors[neighbor_idx].indices,
                               all_neighbors[neighbor_idx].distances);
        if (all_neighbors[neighbor_idx].indices.size() > m_num_neighbors)
        {
          all_neighbors[neighbor_idx].indices.resize(m_num_neighbors);
          all_neighbors[neighbor_idx].distances.resize(m_num_neighbors);
        }
        all_neighbors[neighbor_idx].computed = true;
      }
      
      compute_spfh(cloud, normals, neighbor_idx,
                   all_neighbors[neighbor_idx].indices,
                   spfh_array[neighbor_idx]);
      spfh_computed[neighbor_idx] = true;
    }
    
    // 使用距离倒数作为权重 / Use inverse distance as weight
    data_type weight = DataType(1) / (neighbor_distances[i] + DataType(1e-6));
    weight_sum += weight;
    
    const auto& neighbor_spfh = spfh_array[neighbor_idx];
    
    // 累积 / Accumulate
    for (std::size_t j = 0; j < 11; ++j)
    {
      fpfh.histogram[j] += weight * neighbor_spfh.f1[j];
      fpfh.histogram[j + 11] += weight * neighbor_spfh.f2[j];
      fpfh.histogram[j + 22] += weight * neighbor_spfh.f3[j];
    }
  }
  
  // 归一化 / Normalize
  if (weight_sum > DataType(0))
  {
    data_type norm_factor = DataType(1) / (DataType(1) + weight_sum);
    for (std::size_t i = 0; i < 33; ++i)
    {
      fpfh.histogram[i] *= norm_factor;
    }
  }
}

}  // namespace toolbox::pcl