#pragma once

#include <numeric>
#include <unordered_set>

#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/metrics/point_cloud_metrics.hpp>
#include <cpp-toolbox/pcl/registration/four_pcs_registration.hpp>
#include <cpp-toolbox/utils/random.hpp>
#include <cpp-toolbox/utils/timer.hpp>

namespace toolbox::pcl
{

template<typename DataType>
bool four_pcs_registration_t<DataType>::align_impl(result_type& result)
{
  // 初始化结果 / Initialize result
  result.transformation.setIdentity();
  result.fitness_score = std::numeric_limits<DataType>::max();
  result.inliers.clear();
  result.num_iterations = 0;
  result.converged = false;

  LOG_DEBUG_S << "4PCS: 开始配准 / Starting registration"
              << ", 源点云大小 / source size: "
              << this->get_source_cloud()->size()
              << ", 目标点云大小 / target size: "
              << this->get_target_cloud()->size()
              << ", 重叠率 / overlap: " << m_overlap
              << ", 精度 / delta: " << m_delta;

  // 计时器 / Timer
  toolbox::utils::stop_watch_timer_t timer("4PCS");
  timer.start();

  // 采样点云 / Sample point clouds
  if (m_source_samples.empty() || m_target_samples.empty()) {
    LOG_ERROR_S << "4PCS: 点云采样失败 / Point cloud sampling failed";
    return false;
  }

  LOG_DEBUG_S << "4PCS: 采样完成 / Sampling complete"
              << ", 源采样数 / source samples: " << m_source_samples.size()
              << ", 目标采样数 / target samples: " << m_target_samples.size();

  // 提取源点云的共面4点基 / Extract coplanar 4-point bases from source
  auto source_bases = extract_coplanar_bases(
      m_source_samples, this->get_source_cloud(), m_num_bases);
  if (source_bases.empty()) {
    LOG_ERROR_S
        << "4PCS: 无法提取有效的4点基 / Failed to extract valid 4-point bases";
    return false;
  }

  LOG_DEBUG_S << "4PCS: 提取了 " << source_bases.size()
              << " 个4点基 / 4-point bases";

  // 最佳候选 / Best candidate
  candidate_t best_candidate;
  best_candidate.lcp_score = std::numeric_limits<DataType>::max();
  best_candidate.num_inliers = 0;

  // 对每个源基寻找匹配 / Find matches for each source base
  std::size_t total_candidates = 0;
  for (std::size_t base_idx = 0; base_idx < source_bases.size(); ++base_idx) {
    result.num_iterations = base_idx + 1;
    const auto& source_base = source_bases[base_idx];

    // 寻找匹配的目标基 / Find congruent target bases
    auto target_bases = find_congruent_sets(
        source_base, m_target_samples, this->get_target_cloud());
    total_candidates += target_bases.size();

    // 评估每个候选匹配 / Evaluate each candidate match
    for (const auto& target_base : target_bases) {
      candidate_t candidate;
      candidate.source_base = source_base;
      candidate.target_base = target_base;

      // 估计变换 / Estimate transformation
      candidate.transform = estimate_transformation(source_base, target_base);

      if (!is_valid_transformation(candidate.transform)) {
        continue;
      }

      // 计算LCP评分 / Compute LCP score
      std::vector<std::size_t> inliers;
      candidate.lcp_score = compute_lcp_score(candidate.transform, inliers);
      candidate.num_inliers = inliers.size();

      // 更新最佳候选 / Update best candidate
      if (candidate.num_inliers > best_candidate.num_inliers
          || (candidate.num_inliers == best_candidate.num_inliers
              && candidate.lcp_score < best_candidate.lcp_score))
      {
        best_candidate = candidate;
        result.inliers = std::move(inliers);
      }
    }

    // 早停检查 / Early stopping check
    if (best_candidate.num_inliers >= m_source_samples.size() * m_overlap * 0.9)
    {
      LOG_DEBUG_S
          << "4PCS: 早停，找到足够好的匹配 / Early stopping, found good match";
      result.converged = true;
      break;
    }
  }

  timer.stop();
  double elapsed_time = timer.elapsed_time();
  LOG_DEBUG_S << "4PCS: 完成 " << result.num_iterations << " 个基的匹配，共 "
              << total_candidates
              << " 个候选，耗时 / bases with candidates in: " << elapsed_time
              << " 秒/s";

  // 检查是否找到有效的匹配 / Check if valid match found
  if (best_candidate.num_inliers < this->get_min_inliers()) {
    LOG_ERROR_S << "4PCS: 内点数量不足 / Insufficient inliers: "
                << best_candidate.num_inliers << " < "
                << this->get_min_inliers();
    return false;
  }

  // 精炼最佳候选 / Refine best candidate
  refine_candidate(best_candidate);

  // 设置结果 / Set result
  result.transformation = best_candidate.transform;
  result.fitness_score = best_candidate.lcp_score;
  result.converged = result.converged
      || (best_candidate.num_inliers >= this->get_min_inliers());

  LOG_DEBUG_S << "4PCS: 配准完成 / Registration complete"
              << ", 内点 / inliers: " << result.inliers.size()
              << ", LCP评分 / LCP score: " << result.fitness_score;

  return true;
}

template<typename DataType>
bool four_pcs_registration_t<DataType>::validate_input_impl() const
{
  // 检查点云大小 / Check point cloud sizes
  auto source_cloud = this->get_source_cloud();
  auto target_cloud = this->get_target_cloud();

  if (source_cloud->size() < 4 || target_cloud->size() < 4) {
    LOG_ERROR_S << "4PCS: 点云太小，至少需要4个点 / Point clouds too small, "
                   "need at least 4 points";
    return false;
  }

  // 检查参数 / Check parameters
  if (m_delta <= 0) {
    LOG_ERROR_S << "4PCS: 无效的delta值 / Invalid delta value: " << m_delta;
    return false;
  }

  if (m_overlap <= 0 || m_overlap > 1) {
    LOG_ERROR_S << "4PCS: 无效的重叠率 / Invalid overlap ratio: " << m_overlap;
    return false;
  }

  return true;
}

template<typename DataType>
void four_pcs_registration_t<DataType>::set_source_impl(
    const point_cloud_ptr& source)
{
  // 采样源点云 / Sample source point cloud
  sample_points(source, m_sample_size, m_source_samples);
}

template<typename DataType>
void four_pcs_registration_t<DataType>::set_target_impl(
    const point_cloud_ptr& target)
{
  // 采样目标点云 / Sample target point cloud
  sample_points(target, m_sample_size, m_target_samples);

  // 构建目标点云的KD树 / Build KD-tree for target cloud
  m_target_kdtree = std::make_shared<kdtree_t>();
  m_target_kdtree->set_input(target);
}

template<typename DataType>
void four_pcs_registration_t<DataType>::sample_points(
    const point_cloud_ptr& cloud,
    std::size_t num_samples,
    std::vector<std::size_t>& indices)
{
  indices.clear();

  if (!cloud || cloud->empty()) {
    return;
  }

  // 如果请求的样本数大于等于点云大小，使用所有点 / If requested samples >=
  // cloud size, use all points
  if (num_samples >= cloud->size()) {
    indices.resize(cloud->size());
    std::iota(indices.begin(), indices.end(), 0);
    return;
  }

  // 均匀随机采样 / Uniform random sampling
  indices.reserve(num_samples);
  std::vector<std::size_t> all_indices(cloud->size());
  std::iota(all_indices.begin(), all_indices.end(), 0);

  // 使用随机采样 / Use random sampling
  std::mt19937 generator(this->get_random_seed());
  std::shuffle(all_indices.begin(), all_indices.end(), generator);

  indices.assign(all_indices.begin(), all_indices.begin() + num_samples);
}

template<typename DataType>
std::vector<typename four_pcs_registration_t<DataType>::base_4pcs_t>
four_pcs_registration_t<DataType>::extract_coplanar_bases(
    const std::vector<std::size_t>& indices,
    const point_cloud_ptr& cloud,
    std::size_t num_bases) const
{
  std::vector<base_4pcs_t> bases;
  bases.reserve(num_bases);

  if (indices.size() < 4) {
    return bases;
  }

  std::mt19937 generator(this->get_random_seed());
  std::uniform_int_distribution<std::size_t> dist(0, indices.size() - 1);

  const DataType coplanar_tolerance =
      m_delta * 2;  // 共面容差 / Coplanarity tolerance
  std::size_t attempts = 0;
  const std::size_t max_attempts =
      num_bases * 100;  // 最大尝试次数 / Max attempts

  while (bases.size() < num_bases && attempts < max_attempts) {
    attempts++;

    // 随机选择4个不同的点 / Randomly select 4 different points
    std::array<std::size_t, 4> selected_indices;
    std::unordered_set<std::size_t> used;

    for (std::size_t i = 0; i < 4; ++i) {
      std::size_t idx;
      do {
        idx = dist(generator);
      } while (used.find(idx) != used.end());
      used.insert(idx);
      selected_indices[i] = indices[idx];
    }

    // 提取点坐标 / Extract point coordinates
    base_4pcs_t base;
    base.indices = selected_indices;
    for (std::size_t i = 0; i < 4; ++i) {
      const auto& pt = cloud->points[selected_indices[i]];
      base.points[i] = vector3_t(pt.x, pt.y, pt.z);
    }

    // 检查是否共面 / Check if coplanar
    if (!are_coplanar(base.points, coplanar_tolerance)) {
      continue;
    }

    // 计算平面参数 / Compute plane parameters
    vector3_t v1 = base.points[1] - base.points[0];
    vector3_t v2 = base.points[2] - base.points[0];
    base.normal = v1.cross(v2).normalized();
    base.d = -base.normal.dot(base.points[0]);

    // 计算仿射不变量 / Compute affine invariants
    compute_invariants(base);

    // 检查基的有效性（点不能太近） / Check base validity (points not too close)
    bool valid = true;
    for (std::size_t i = 0; i < 4; ++i) {
      for (std::size_t j = i + 1; j < 4; ++j) {
        DataType dist = (base.points[i] - base.points[j]).norm();
        if (dist
            < m_delta * 10) {  // 点间最小距离 / Min distance between points
          valid = false;
          break;
        }
      }
      if (!valid)
        break;
    }

    if (valid) {
      bases.push_back(base);
    }
  }

  return bases;
}

template<typename DataType>
bool four_pcs_registration_t<DataType>::are_coplanar(
    const std::array<vector3_t, 4>& points, DataType tolerance) const
{
  // 使用前三个点定义平面 / Use first three points to define plane
  vector3_t v1 = points[1] - points[0];
  vector3_t v2 = points[2] - points[0];
  vector3_t normal = v1.cross(v2);

  // 检查是否退化（三点共线） / Check if degenerate (three points collinear)
  if (normal.norm() < std::numeric_limits<DataType>::epsilon()) {
    return false;
  }

  normal.normalize();
  DataType d = -normal.dot(points[0]);

  // 检查第四个点到平面的距离 / Check distance of fourth point to plane
  DataType dist = std::abs(normal.dot(points[3]) + d);

  return dist <= tolerance;
}

template<typename DataType>
void four_pcs_registration_t<DataType>::compute_invariants(
    base_4pcs_t& base) const
{
  // 4PCS使用两个交点的比率作为仿射不变量 / 4PCS uses ratios of intersection
  // points as affine invariants 对角线AC和BD的交点 / Intersection of diagonals
  // AC and BD

  // 计算对角线AC: P0-P2 / Compute diagonal AC: P0-P2
  vector3_t ac_dir = base.points[2] - base.points[0];
  DataType ac_len = ac_dir.norm();

  // 计算对角线BD: P1-P3 / Compute diagonal BD: P1-P3
  vector3_t bd_dir = base.points[3] - base.points[1];
  DataType bd_len = bd_dir.norm();

  // 计算交点（使用参数方程） / Compute intersection (using parametric
  // equations) P0 + s * AC_dir = P1 + t * BD_dir

  // 构建线性系统 / Build linear system
  matrix3_t A;
  A.col(0) = ac_dir;
  A.col(1) = -bd_dir;
  A.col(2) = base.normal;  // 添加法向量确保系统非奇异 / Add normal to ensure
                           // non-singular

  vector3_t b = base.points[1] - base.points[0];

  // 求解 / Solve
  vector3_t params = A.fullPivLu().solve(b);
  DataType s = params[0];
  DataType t = params[1];

  // 计算不变量（交点在对角线上的位置比率） / Compute invariants (position
  // ratios on diagonals)
  base.invariant1 = s;  // 交点在AC上的位置 / Position on AC
  base.invariant2 = t;  // 交点在BD上的位置 / Position on BD
}

template<typename DataType>
std::vector<typename four_pcs_registration_t<DataType>::base_4pcs_t>
four_pcs_registration_t<DataType>::find_congruent_sets(
    const base_4pcs_t& source_base,
    const std::vector<std::size_t>& target_indices,
    const point_cloud_ptr& target_cloud) const
{
  std::vector<base_4pcs_t> congruent_bases;

  // 容差 / Tolerances
  const DataType invariant_tolerance = 0.1;  // 不变量容差 / Invariant tolerance
  const DataType distance_tolerance =
      m_delta * 2;  // 距离容差 / Distance tolerance

  // 计算源基的边长 / Compute source base edge lengths
  std::array<DataType, 6> source_distances;  // 6条边 / 6 edges
  std::size_t edge_idx = 0;
  for (std::size_t i = 0; i < 4; ++i) {
    for (std::size_t j = i + 1; j < 4; ++j) {
      source_distances[edge_idx++] =
          (source_base.points[i] - source_base.points[j]).norm();
    }
  }

  // 寻找所有可能的匹配 / Find all possible matches
  // 这里简化实现，实际4PCS使用更复杂的搜索策略 / Simplified implementation,
  // actual 4PCS uses more complex search

  if (target_indices.size() < 4) {
    return congruent_bases;
  }

  // 随机尝试一些组合 / Try some random combinations
  std::mt19937 generator(this->get_random_seed());
  std::uniform_int_distribution<std::size_t> dist(0, target_indices.size() - 1);

  const std::size_t max_tries = 1000;
  for (std::size_t try_idx = 0; try_idx < max_tries; ++try_idx) {
    // 随机选择4个目标点 / Randomly select 4 target points
    std::array<std::size_t, 4> selected_indices;
    std::unordered_set<std::size_t> used;

    bool valid_selection = true;
    for (std::size_t i = 0; i < 4; ++i) {
      std::size_t idx;
      std::size_t attempts = 0;
      do {
        idx = dist(generator);
        attempts++;
        if (attempts > 100) {
          valid_selection = false;
          break;
        }
      } while (used.find(idx) != used.end());

      if (!valid_selection)
        break;
      used.insert(idx);
      selected_indices[i] = target_indices[idx];
    }

    if (!valid_selection)
      continue;

    // 构建目标基 / Build target base
    base_4pcs_t target_base;
    target_base.indices = selected_indices;
    for (std::size_t i = 0; i < 4; ++i) {
      const auto& pt = target_cloud->points[selected_indices[i]];
      target_base.points[i] = vector3_t(pt.x, pt.y, pt.z);
    }

    // 检查是否共面 / Check if coplanar
    if (!are_coplanar(target_base.points, distance_tolerance)) {
      continue;
    }

    // 计算目标基的边长 / Compute target base edge lengths
    std::array<DataType, 6> target_distances;
    edge_idx = 0;
    for (std::size_t i = 0; i < 4; ++i) {
      for (std::size_t j = i + 1; j < 4; ++j) {
        target_distances[edge_idx++] =
            (target_base.points[i] - target_base.points[j]).norm();
      }
    }

    // 检查边长是否匹配（允许不同的点对应关系） / Check if edge lengths match
    // (allow different correspondences)
    bool distances_match = true;
    for (std::size_t i = 0; i < 6; ++i) {
      bool found_match = false;
      for (std::size_t j = 0; j < 6; ++j) {
        if (std::abs(source_distances[i] - target_distances[j])
            < distance_tolerance)
        {
          found_match = true;
          break;
        }
      }
      if (!found_match) {
        distances_match = false;
        break;
      }
    }

    if (!distances_match) {
      continue;
    }

    // 计算不变量 / Compute invariants
    compute_invariants(target_base);

    // 检查不变量是否匹配 / Check if invariants match
    if (std::abs(source_base.invariant1 - target_base.invariant1)
            < invariant_tolerance
        && std::abs(source_base.invariant2 - target_base.invariant2)
            < invariant_tolerance)
    {
      congruent_bases.push_back(target_base);
    }
  }

  return congruent_bases;
}

template<typename DataType>
typename four_pcs_registration_t<DataType>::transformation_t
four_pcs_registration_t<DataType>::estimate_transformation(
    const base_4pcs_t& source_base, const base_4pcs_t& target_base) const
{
  transformation_t transform;
  transform.setIdentity();

  // 使用SVD估计变换（类似RANSAC） / Estimate transformation using SVD (similar
  // to RANSAC)

  // 计算质心 / Compute centroids
  vector3_t source_centroid = vector3_t::Zero();
  vector3_t target_centroid = vector3_t::Zero();

  for (std::size_t i = 0; i < 4; ++i) {
    source_centroid += source_base.points[i];
    target_centroid += target_base.points[i];
  }

  source_centroid /= 4;
  target_centroid /= 4;

  // 构建协方差矩阵 / Build covariance matrix
  matrix3_t H = matrix3_t::Zero();

  for (std::size_t i = 0; i < 4; ++i) {
    vector3_t src_centered = source_base.points[i] - source_centroid;
    vector3_t tgt_centered = target_base.points[i] - target_centroid;
    H += src_centered * tgt_centered.transpose();
  }

  // SVD分解 / SVD decomposition
  Eigen::JacobiSVD<matrix3_t> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
  matrix3_t U = svd.matrixU();
  matrix3_t V = svd.matrixV();

  // 计算旋转矩阵 / Compute rotation matrix
  matrix3_t R = V * U.transpose();

  // 处理反射情况 / Handle reflection case
  if (R.determinant() < 0) {
    V.col(2) *= -1;
    R = V * U.transpose();
  }

  // 计算平移向量 / Compute translation vector
  vector3_t t = target_centroid - R * source_centroid;

  // 构建4x4变换矩阵 / Build 4x4 transformation matrix
  transform.template block<3, 3>(0, 0) = R;
  transform.template block<3, 1>(0, 3) = t;

  return transform;
}

template<typename DataType>
DataType four_pcs_registration_t<DataType>::compute_lcp_score(
    const transformation_t& transform, std::vector<std::size_t>& inliers) const
{
  // 使用 LCPMetric 计算评分 / Use LCPMetric to compute score
  toolbox::metrics::LCPMetric<DataType> lcp_metric(m_delta);
  
  auto source_cloud = this->get_source_cloud();
  auto target_cloud = this->get_target_cloud();
  
  // 创建采样点云用于LCP计算 / Create sampled point cloud for LCP computation
  toolbox::types::point_cloud_t<DataType> sampled_source, sampled_target;
  sampled_source.points.reserve(m_source_samples.size());
  sampled_target.points.reserve(m_target_samples.size());
  
  for (std::size_t src_idx : m_source_samples) {
    sampled_source.points.push_back(source_cloud->points[src_idx]);
  }
  
  for (std::size_t tgt_idx : m_target_samples) {
    sampled_target.points.push_back(target_cloud->points[tgt_idx]);
  }
  
  // 使用基本版本的LCP计算 / Use basic version of LCP computation
  std::vector<std::size_t> sampled_inliers;
  DataType score = lcp_metric.compute_lcp_score(
      sampled_source, sampled_target, transform, &sampled_inliers);
  
  // 将采样索引映射回原始索引 / Map sampled indices back to original indices
  inliers.clear();
  inliers.reserve(sampled_inliers.size());
  for (std::size_t idx : sampled_inliers) {
    inliers.push_back(m_source_samples[idx]);
  }
  
  return score;
}

template<typename DataType>
void four_pcs_registration_t<DataType>::refine_candidate(
    candidate_t& candidate) const
{
  // 可以在这里添加ICP或其他精炼方法 / Can add ICP or other refinement methods
  // here 目前保持简单，只重新计算LCP / Keep it simple for now, just recompute
  // LCP

  std::vector<std::size_t> refined_inliers;
  candidate.lcp_score = compute_lcp_score(candidate.transform, refined_inliers);
  candidate.num_inliers = refined_inliers.size();
}

template<typename DataType>
bool four_pcs_registration_t<DataType>::is_valid_transformation(
    const transformation_t& transform) const
{
  // 检查旋转部分是否有效 / Check if rotation part is valid
  matrix3_t R = transform.template block<3, 3>(0, 0);

  // 检查行列式（应该接近1） / Check determinant (should be close to 1)
  DataType det = R.determinant();
  if (std::abs(det - 1.0) > 0.1) {
    return false;
  }

  // 检查正交性 / Check orthogonality
  matrix3_t should_be_identity = R * R.transpose();
  matrix3_t identity = matrix3_t::Identity();
  DataType error = (should_be_identity - identity).norm();

  return error < 0.1;
}

// 显式实例化 / Explicit instantiation
template class four_pcs_registration_t<float>;
template class four_pcs_registration_t<double>;

}  // namespace toolbox::pcl