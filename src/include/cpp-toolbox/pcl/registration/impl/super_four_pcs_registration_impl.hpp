#pragma once

#include <cpp-toolbox/pcl/registration/super_four_pcs_registration.hpp>
#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/utils/timer.hpp>
#include <optional>
#include <numeric>
#include <unordered_set>
#include <algorithm>

namespace toolbox::pcl
{

// Smart Index 实现 / Smart Index implementation
template<typename DataType>
void super_four_pcs_registration_t<DataType>::smart_index_t::build(
    const point_cloud_ptr& cloud, const std::vector<std::size_t>& indices)
{
  m_cloud = cloud;
  m_grid.clear();
  
  if (indices.empty()) return;
  
  // 计算边界 / Compute bounds
  m_min_bound = vector3_t(std::numeric_limits<DataType>::max(),
                          std::numeric_limits<DataType>::max(),
                          std::numeric_limits<DataType>::max());
  m_max_bound = vector3_t(std::numeric_limits<DataType>::lowest(),
                          std::numeric_limits<DataType>::lowest(),
                          std::numeric_limits<DataType>::lowest());
  
  for (std::size_t idx : indices) {
    const auto& pt = cloud->points[idx];
    vector3_t p(pt.x, pt.y, pt.z);
    m_min_bound = m_min_bound.cwiseMin(p);
    m_max_bound = m_max_bound.cwiseMax(p);
  }
  
  // 添加边界填充 / Add boundary padding
  m_min_bound -= vector3_t::Constant(m_cell_size);
  m_max_bound += vector3_t::Constant(m_cell_size);
  
  // 将点插入网格 / Insert points into grid
  for (std::size_t idx : indices) {
    const auto& pt = cloud->points[idx];
    vector3_t p(pt.x, pt.y, pt.z);
    auto key = compute_grid_key(p);
    m_grid[key].point_indices.push_back(idx);
  }
}

template<typename DataType>
std::vector<typename super_four_pcs_registration_t<DataType>::point_pair_t>
super_four_pcs_registration_t<DataType>::smart_index_t::find_pairs_in_range(
    DataType distance, DataType epsilon, std::size_t max_pairs) const
{
  std::vector<point_pair_t> pairs;
  pairs.reserve(max_pairs);
  
  const DataType min_dist = distance - epsilon;
  const DataType max_dist = distance + epsilon;
  const DataType min_dist_sq = min_dist * min_dist;
  const DataType max_dist_sq = max_dist * max_dist;
  
  // 计算搜索半径（网格单元数） / Compute search radius (in grid cells)
  int search_radius = static_cast<int>(std::ceil(max_dist / m_cell_size));
  
  // 遍历所有网格单元 / Iterate through all grid cells
  for (const auto& [center_key, center_cell] : m_grid) {
    if (center_cell.point_indices.empty()) continue;
    
    // 获取邻域单元 / Get neighborhood cells
    auto neighbor_keys = get_neighbor_cells(center_key, search_radius);
    
    // 在邻域内搜索点对 / Search for pairs in neighborhood
    for (const auto& neighbor_key : neighbor_keys) {
      auto it = m_grid.find(neighbor_key);
      if (it == m_grid.end()) continue;
      
      const auto& neighbor_cell = it->second;
      
      // 检查所有点对 / Check all point pairs
      for (std::size_t i : center_cell.point_indices) {
        const auto& pt1 = m_cloud->points[i];
        vector3_t p1(pt1.x, pt1.y, pt1.z);
        
        // 确定搜索起点（避免重复） / Determine search start (avoid duplicates)
        std::size_t start_j = (center_key == neighbor_key) ? 
            std::distance(center_cell.point_indices.begin(),
                         std::find(center_cell.point_indices.begin(),
                                  center_cell.point_indices.end(), i)) + 1 : 0;
        
        for (std::size_t j_idx = start_j; j_idx < neighbor_cell.point_indices.size(); ++j_idx) {
          std::size_t j = neighbor_cell.point_indices[j_idx];
          if (i == j) continue;
          
          const auto& pt2 = m_cloud->points[j];
          vector3_t p2(pt2.x, pt2.y, pt2.z);
          
          DataType dist_sq = (p2 - p1).squaredNorm();
          
          if (dist_sq >= min_dist_sq && dist_sq <= max_dist_sq) {
            point_pair_t pair;
            pair.idx1 = i;
            pair.idx2 = j;
            pair.distance = std::sqrt(dist_sq);
            pairs.push_back(pair);
            
            if (pairs.size() >= max_pairs) {
              return pairs;
            }
          }
        }
      }
    }
  }
  
  return pairs;
}

template<typename DataType>
std::string super_four_pcs_registration_t<DataType>::smart_index_t::get_statistics() const
{
  std::stringstream ss;
  ss << "Smart Index Statistics:\n";
  ss << "  Grid cells: " << m_grid.size() << "\n";
  ss << "  Cell size: " << m_cell_size << "\n";
  ss << "  Bounds: [" << m_min_bound.transpose() << "] to [" << m_max_bound.transpose() << "]\n";
  
  std::size_t total_points = 0;
  std::size_t max_points_per_cell = 0;
  for (const auto& [key, cell] : m_grid) {
    total_points += cell.point_indices.size();
    max_points_per_cell = std::max(max_points_per_cell, cell.point_indices.size());
  }
  
  ss << "  Total indexed points: " << total_points << "\n";
  ss << "  Max points per cell: " << max_points_per_cell << "\n";
  ss << "  Avg points per cell: " << static_cast<double>(total_points) / m_grid.size() << "\n";
  
  return ss.str();
}

template<typename DataType>
std::tuple<int, int, int> super_four_pcs_registration_t<DataType>::smart_index_t::compute_grid_key(
    const vector3_t& pt) const
{
  int x = static_cast<int>(std::floor((pt[0] - m_min_bound[0]) / m_cell_size));
  int y = static_cast<int>(std::floor((pt[1] - m_min_bound[1]) / m_cell_size));
  int z = static_cast<int>(std::floor((pt[2] - m_min_bound[2]) / m_cell_size));
  return std::make_tuple(x, y, z);
}

template<typename DataType>
std::vector<std::tuple<int, int, int>> 
super_four_pcs_registration_t<DataType>::smart_index_t::get_neighbor_cells(
    const std::tuple<int, int, int>& center_key, int radius) const
{
  std::vector<std::tuple<int, int, int>> neighbors;
  
  auto [cx, cy, cz] = center_key;
  
  for (int dx = -radius; dx <= radius; ++dx) {
    for (int dy = -radius; dy <= radius; ++dy) {
      for (int dz = -radius; dz <= radius; ++dz) {
        neighbors.emplace_back(cx + dx, cy + dy, cz + dz);
      }
    }
  }
  
  return neighbors;
}

// Super4PCS 主实现 / Super4PCS main implementation
template<typename DataType>
bool super_four_pcs_registration_t<DataType>::align_impl(result_type& result)
{
  // 如果禁用智能索引，使用基类实现 / If smart indexing disabled, use base class implementation
  if (!m_use_smart_indexing) {
    LOG_INFO_S << "Super4PCS: 智能索引已禁用，使用标准4PCS / Smart indexing disabled, using standard 4PCS";
    return base_type::align_impl(result);
  }

  // 初始化结果 / Initialize result
  result.transformation.setIdentity();
  result.fitness_score = std::numeric_limits<DataType>::max();
  result.inliers.clear();
  result.num_iterations = 0;
  result.converged = false;

  LOG_INFO_S << "Super4PCS: 开始配准（智能索引模式） / Starting registration (smart indexing mode)"
             << ", 源点云大小 / source size: " << this->get_source_cloud()->size()
             << ", 目标点云大小 / target size: " << this->get_target_cloud()->size();

  // 计时器 / Timer
  toolbox::utils::stop_watch_timer_t timer("Super4PCS");
  timer.start();

  // 获取采样点 / Get sampled points
  const auto& source_samples = this->get_source_samples();
  const auto& target_samples = this->get_target_samples();
  
  if (source_samples.empty() || target_samples.empty()) {
    LOG_ERROR_S << "Super4PCS: 点云采样失败 / Point cloud sampling failed";
    return false;
  }

  // 计算网格分辨率 / Compute grid resolution
  DataType grid_res = m_grid_resolution;
  if (grid_res <= 0) {
    grid_res = compute_adaptive_grid_resolution(this->get_source_cloud(), source_samples);
    LOG_INFO_S << "Super4PCS: 自动计算网格分辨率 / Auto-computed grid resolution: " << grid_res;
  }

  // 构建智能索引 / Build smart indices
  m_source_index = std::make_unique<smart_index_t>(grid_res);
  m_target_index = std::make_unique<smart_index_t>(grid_res);
  
  m_source_index->build(this->get_source_cloud(), source_samples);
  m_target_index->build(this->get_target_cloud(), target_samples);
  
  LOG_INFO_S << "Super4PCS: 索引构建完成 / Index building complete";
  LOG_INFO_S << "源索引 / Source index: " << m_source_index->get_statistics();
  LOG_INFO_S << "目标索引 / Target index: " << m_target_index->get_statistics();

  // 提取源点云的共面4点基（使用优化方法） / Extract coplanar bases (using optimized method)
  std::vector<base_4pcs_t> source_bases;
  
  // 计算典型距离 / Compute typical distances
  std::vector<DataType> base_distances;
  const std::size_t num_distance_samples = 10;
  std::mt19937 generator(this->get_random_seed());
  std::uniform_int_distribution<std::size_t> dist(0, source_samples.size() - 1);
  
  for (std::size_t i = 0; i < num_distance_samples && i < source_samples.size() / 2; ++i) {
    std::size_t idx1 = source_samples[dist(generator)];
    std::size_t idx2 = source_samples[dist(generator)];
    if (idx1 != idx2) {
      const auto& pt1 = this->get_source_cloud()->points[idx1];
      const auto& pt2 = this->get_source_cloud()->points[idx2];
      DataType d = std::sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) +
                            (pt2.y - pt1.y) * (pt2.y - pt1.y) +
                            (pt2.z - pt1.z) * (pt2.z - pt1.z));
      base_distances.push_back(d);
    }
  }
  
  // 使用智能索引构建基 / Build bases using smart indexing
  for (const DataType& distance : base_distances) {
    // 提取指定距离的点对 / Extract point pairs at specified distance
    auto source_pairs = extract_pairs_smart(source_samples, this->get_source_cloud(),
                                           distance, m_pair_distance_epsilon);
    
    // 从点对构建4点基 / Build 4-point bases from pairs
    for (std::size_t i = 0; i < source_pairs.size() && source_bases.size() < this->get_num_bases(); ++i) {
      for (std::size_t j = i + 1; j < source_pairs.size() && source_bases.size() < this->get_num_bases(); ++j) {
        if (verify_pair_compatibility(source_pairs[i], source_pairs[j])) {
          auto base_opt = build_base_from_pairs(source_pairs[i], source_pairs[j],
                                               this->get_source_cloud());
          if (base_opt.has_value()) {
            source_bases.push_back(base_opt.value());
          }
        }
      }
    }
  }

  if (source_bases.empty()) {
    LOG_ERROR_S << "Super4PCS: 无法提取有效的4点基 / Failed to extract valid 4-point bases";
    return false;
  }

  LOG_INFO_S << "Super4PCS: 提取了 " << source_bases.size() << " 个4点基 / 4-point bases";

  // 最佳候选 / Best candidate
  candidate_t best_candidate;
  best_candidate.lcp_score = std::numeric_limits<DataType>::max();
  best_candidate.num_inliers = 0;

  // 对每个源基寻找匹配（使用优化方法） / Find matches for each source base (using optimized method)
  std::size_t total_candidates = 0;
  
  for (std::size_t base_idx = 0; base_idx < source_bases.size(); ++base_idx) {
    result.num_iterations = base_idx + 1;
    const auto& source_base = source_bases[base_idx];

    // 使用优化方法寻找匹配 / Find matches using optimized method
    auto target_bases = find_congruent_sets_optimized(source_base);
    total_candidates += target_bases.size();

    // 评估每个候选 / Evaluate each candidate
    for (const auto& target_base : target_bases) {
      candidate_t candidate;
      candidate.source_base = source_base;
      candidate.target_base = target_base;

      // 估计变换 / Estimate transformation
      candidate.transform = this->estimate_transformation(source_base, target_base);
      
      if (!this->is_valid_transformation(candidate.transform)) {
        continue;
      }

      // 计算LCP评分 / Compute LCP score
      std::vector<std::size_t> inliers;
      candidate.lcp_score = this->compute_lcp_score(candidate.transform, inliers);
      candidate.num_inliers = inliers.size();

      // 更新最佳候选 / Update best candidate
      if (candidate.num_inliers > best_candidate.num_inliers ||
          (candidate.num_inliers == best_candidate.num_inliers && 
           candidate.lcp_score < best_candidate.lcp_score)) {
        best_candidate = candidate;
        result.inliers = std::move(inliers);
      }
    }

    // 早停检查 / Early stopping check
    if (best_candidate.num_inliers >= source_samples.size() * this->get_overlap() * 0.9) {
      LOG_INFO_S << "Super4PCS: 早停，找到足够好的匹配 / Early stopping, found good match";
      result.converged = true;
      break;
    }
  }

  timer.stop();
  double elapsed_time = timer.elapsed_time();
  LOG_INFO_S << "Super4PCS: 完成 " << result.num_iterations << " 个基的匹配，共 "
             << total_candidates << " 个候选，耗时 / bases with candidates in: "
             << elapsed_time << " 秒/s";

  // 检查是否找到有效的匹配 / Check if valid match found
  if (best_candidate.num_inliers < this->get_min_inliers()) {
    LOG_ERROR_S << "Super4PCS: 内点数量不足 / Insufficient inliers: " 
                << best_candidate.num_inliers << " < " << this->get_min_inliers();
    return false;
  }

  // 精炼最佳候选 / Refine best candidate
  this->refine_candidate(best_candidate);

  // 设置结果 / Set result
  result.transformation = best_candidate.transform;
  result.fitness_score = best_candidate.lcp_score;
  result.converged = result.converged || (best_candidate.num_inliers >= this->get_min_inliers());

  LOG_INFO_S << "Super4PCS: 配准完成 / Registration complete"
             << ", 内点 / inliers: " << result.inliers.size()
             << ", LCP评分 / LCP score: " << result.fitness_score;

  return true;
}

template<typename DataType>
std::vector<typename super_four_pcs_registration_t<DataType>::base_4pcs_t>
super_four_pcs_registration_t<DataType>::find_congruent_sets_optimized(
    const base_4pcs_t& source_base) const
{
  std::vector<base_4pcs_t> congruent_bases;
  
  // 计算源基的两条对角线长度 / Compute diagonal lengths of source base
  DataType diag1_len = (source_base.points[2] - source_base.points[0]).norm();  // AC
  DataType diag2_len = (source_base.points[3] - source_base.points[1]).norm();  // BD
  
  // 使用智能索引查找匹配的对角线 / Find matching diagonals using smart indexing
  auto target_diag1_pairs = m_target_index->find_pairs_in_range(
      diag1_len, m_pair_distance_epsilon, 100);
  auto target_diag2_pairs = m_target_index->find_pairs_in_range(
      diag2_len, m_pair_distance_epsilon, 100);
  
  // 尝试组合匹配的对角线对 / Try combining matching diagonal pairs
  for (const auto& pair1 : target_diag1_pairs) {
    for (const auto& pair2 : target_diag2_pairs) {
      // 检查是否形成有效的4点集 / Check if they form valid 4-point set
      std::unordered_set<std::size_t> indices_set;
      indices_set.insert(pair1.idx1);
      indices_set.insert(pair1.idx2);
      indices_set.insert(pair2.idx1);
      indices_set.insert(pair2.idx2);
      
      if (indices_set.size() != 4) {
        continue;  // 有重复点 / Has duplicate points
      }
      
      // 构建目标基 / Build target base
      base_4pcs_t target_base;
      target_base.indices = {pair1.idx1, pair2.idx1, pair1.idx2, pair2.idx2};
      
      auto target_cloud = this->get_target_cloud();
      for (std::size_t i = 0; i < 4; ++i) {
        const auto& pt = target_cloud->points[target_base.indices[i]];
        target_base.points[i] = vector3_t(pt.x, pt.y, pt.z);
      }
      
      // 检查是否共面 / Check if coplanar
      if (!this->are_coplanar(target_base.points, this->get_delta() * 2)) {
        continue;
      }
      
      // 计算不变量并检查匹配 / Compute invariants and check match
      this->compute_invariants(target_base);
      
      const DataType invariant_tolerance = 0.1;
      if (std::abs(source_base.invariant1 - target_base.invariant1) < invariant_tolerance &&
          std::abs(source_base.invariant2 - target_base.invariant2) < invariant_tolerance) {
        congruent_bases.push_back(target_base);
      }
    }
  }
  
  return congruent_bases;
}

template<typename DataType>
std::vector<typename super_four_pcs_registration_t<DataType>::point_pair_t>
super_four_pcs_registration_t<DataType>::extract_pairs_smart(
    const std::vector<std::size_t>& indices, const point_cloud_ptr& cloud,
    DataType distance, DataType epsilon) const
{
  if (m_source_index) {
    return m_source_index->find_pairs_in_range(distance, epsilon);
  }
  
  // 后备方案：暴力搜索 / Fallback: brute force search
  std::vector<point_pair_t> pairs;
  const DataType min_dist = distance - epsilon;
  const DataType max_dist = distance + epsilon;
  
  for (std::size_t i = 0; i < indices.size(); ++i) {
    for (std::size_t j = i + 1; j < indices.size(); ++j) {
      const auto& pt1 = cloud->points[indices[i]];
      const auto& pt2 = cloud->points[indices[j]];
      
      DataType d = std::sqrt((pt2.x - pt1.x) * (pt2.x - pt1.x) +
                            (pt2.y - pt1.y) * (pt2.y - pt1.y) +
                            (pt2.z - pt1.z) * (pt2.z - pt1.z));
      
      if (d >= min_dist && d <= max_dist) {
        point_pair_t pair;
        pair.idx1 = indices[i];
        pair.idx2 = indices[j];
        pair.distance = d;
        pairs.push_back(pair);
      }
    }
  }
  
  return pairs;
}

template<typename DataType>
std::optional<typename super_four_pcs_registration_t<DataType>::base_4pcs_t>
super_four_pcs_registration_t<DataType>::build_base_from_pairs(
    const point_pair_t& pair1, const point_pair_t& pair2,
    const point_cloud_ptr& cloud) const
{
  // 确保没有重复点 / Ensure no duplicate points
  std::unordered_set<std::size_t> indices_set;
  indices_set.insert(pair1.idx1);
  indices_set.insert(pair1.idx2);
  indices_set.insert(pair2.idx1);
  indices_set.insert(pair2.idx2);
  
  if (indices_set.size() != 4) {
    return std::nullopt;
  }
  
  base_4pcs_t base;
  base.indices = {pair1.idx1, pair1.idx2, pair2.idx1, pair2.idx2};
  
  for (std::size_t i = 0; i < 4; ++i) {
    const auto& pt = cloud->points[base.indices[i]];
    base.points[i] = vector3_t(pt.x, pt.y, pt.z);
  }
  
  // 检查共面性 / Check coplanarity
  if (!this->are_coplanar(base.points, this->get_delta() * 2)) {
    return std::nullopt;
  }
  
  // 计算平面参数 / Compute plane parameters
  vector3_t v1 = base.points[1] - base.points[0];
  vector3_t v2 = base.points[2] - base.points[0];
  base.normal = v1.cross(v2).normalized();
  base.d = -base.normal.dot(base.points[0]);
  
  // 计算不变量 / Compute invariants
  this->compute_invariants(base);
  
  return base;
}

template<typename DataType>
bool super_four_pcs_registration_t<DataType>::verify_pair_compatibility(
    const point_pair_t& pair1, const point_pair_t& pair2) const
{
  // 检查点对是否共享点 / Check if pairs share points
  if (pair1.idx1 == pair2.idx1 || pair1.idx1 == pair2.idx2 ||
      pair1.idx2 == pair2.idx1 || pair1.idx2 == pair2.idx2) {
    return true;  // 共享点，可能形成有效基 / Share points, may form valid base
  }
  
  // 检查点对之间的距离是否合理 / Check if distance between pairs is reasonable
  // 这里可以添加更多的几何约束 / More geometric constraints can be added here
  
  return true;
}

template<typename DataType>
DataType super_four_pcs_registration_t<DataType>::compute_adaptive_grid_resolution(
    const point_cloud_ptr& cloud, const std::vector<std::size_t>& indices) const
{
  if (indices.empty()) {
    return this->get_delta() * 10;
  }
  
  // 估计点云密度 / Estimate point cloud density
  const std::size_t sample_size = std::min(static_cast<std::size_t>(100), indices.size());
  std::vector<DataType> nearest_distances;
  nearest_distances.reserve(sample_size);
  
  std::mt19937 generator(this->get_random_seed());
  std::uniform_int_distribution<std::size_t> dist(0, indices.size() - 1);
  
  for (std::size_t i = 0; i < sample_size; ++i) {
    std::size_t idx = indices[dist(generator)];
    const auto& pt = cloud->points[idx];
    
    // 找最近邻（简化版） / Find nearest neighbor (simplified)
    DataType min_dist = std::numeric_limits<DataType>::max();
    for (std::size_t j = 0; j < std::min(static_cast<std::size_t>(10), indices.size()); ++j) {
      if (indices[j] == idx) continue;
      
      const auto& other_pt = cloud->points[indices[j]];
      DataType d = std::sqrt((other_pt.x - pt.x) * (other_pt.x - pt.x) +
                            (other_pt.y - pt.y) * (other_pt.y - pt.y) +
                            (other_pt.z - pt.z) * (other_pt.z - pt.z));
      min_dist = std::min(min_dist, d);
    }
    
    if (min_dist < std::numeric_limits<DataType>::max()) {
      nearest_distances.push_back(min_dist);
    }
  }
  
  if (nearest_distances.empty()) {
    return this->get_delta() * 10;
  }
  
  // 使用中位数作为网格分辨率的基础 / Use median as basis for grid resolution
  std::sort(nearest_distances.begin(), nearest_distances.end());
  DataType median_dist = nearest_distances[nearest_distances.size() / 2];
  
  // 网格单元应该包含几个点 / Grid cell should contain several points
  return median_dist * 5;
}

// 显式实例化 / Explicit instantiation
template class super_four_pcs_registration_t<float>;
template class super_four_pcs_registration_t<double>;

}  // namespace toolbox::pcl