#pragma once

#include <cpp-toolbox/pcl/registration/prosac_registration.hpp>

namespace toolbox::pcl
{

template<typename DataType>
bool prosac_registration_t<DataType>::align_impl(result_type& result)
{
  // 获取基类成员访问 / Get base class member access
  const auto& source_cloud = this->get_source_cloud();
  const auto& target_cloud = this->get_target_cloud();
  const auto& correspondences = this->get_correspondences();
  const auto max_iterations = this->get_max_iterations();
  const auto inlier_threshold = this->get_inlier_threshold();
  const auto min_inliers = this->get_min_inliers();
  const auto random_seed = this->get_random_seed();

  // 初始化结果 / Initialize result
  result.transformation = transformation_t::Identity();
  result.fitness_score = std::numeric_limits<DataType>::max();
  result.inliers.clear();
  result.converged = false;
  result.num_iterations = 0;

  // 检查是否有足够的对应关系 / Check if there are enough correspondences
  const std::size_t num_correspondences = correspondences->size();
  if (num_correspondences < m_sample_size) {
    LOG_ERROR_S << "错误：对应关系数量不足 / Error: Insufficient correspondences: "
                << num_correspondences << " < " << m_sample_size;
    return false;
  }

  // 预计算采样调度 / Precompute sampling schedule
  precompute_sampling_schedule(num_correspondences);

  // 初始化随机数生成器 / Initialize random number generator
  std::mt19937 generator(random_seed);

  // PROSAC主循环变量 / PROSAC main loop variables
  std::size_t n = m_sample_size;  // 当前采样池大小 / Current sampling pool size
  std::size_t t = 0;              // 迭代计数器 / Iteration counter
  std::size_t best_inlier_count = 0;
  transformation_t best_transformation = transformation_t::Identity();
  std::vector<std::size_t> best_inliers;

  // 用于早停的变量 / Variables for early stopping
  auto start_time = std::chrono::steady_clock::now();
  const auto max_time = std::chrono::seconds(300);  // 5分钟超时 / 5 minutes timeout

  LOG_INFO_S << "开始PROSAC配准 / Starting PROSAC registration with "
             << num_correspondences << " correspondences, sample size "
             << m_sample_size;

  // PROSAC主循环 / PROSAC main loop
  while (t < max_iterations) {
    // 检查超时 / Check timeout
    auto current_time = std::chrono::steady_clock::now();
    if (current_time - start_time > max_time) {
      LOG_WARN_S << "警告：PROSAC达到时间限制 / Warning: PROSAC reached time limit";
      break;
    }

    // 更新采样池大小 / Update sampling pool size
    if (t == m_T_n[n - 1] && n < num_correspondences) {
      n++;
    }

    // 渐进式采样 / Progressive sampling
    std::vector<correspondence_t> sample;
    sample.reserve(m_sample_size);
    progressive_sample(sample, n, t, generator);

    // 检查样本有效性 / Check sample validity
    if (!is_sample_valid(sample)) {
      t++;
      continue;
    }

    // 估计变换 / Estimate transformation
    transformation_t transform = estimate_transformation(sample);

    // 计算内点 / Count inliers
    std::vector<std::size_t> inliers;
    std::size_t inlier_count = count_inliers(transform, inliers);

    // 更新最佳模型 / Update best model
    if (inlier_count > best_inlier_count) {
      best_inlier_count = inlier_count;
      best_transformation = transform;
      best_inliers = inliers;

      LOG_INFO_S << "迭代 / Iteration " << t << ": 找到更好的模型 / found better model with "
                 << inlier_count << " inliers (n=" << n << ")";

      // 检查早停条件 / Check early stopping condition
      DataType inlier_ratio = static_cast<DataType>(inlier_count) / 
                              static_cast<DataType>(num_correspondences);
      if (inlier_ratio >= m_early_stop_ratio) {
        LOG_INFO_S << "达到早停条件 / Reached early stop condition: inlier ratio = "
                   << inlier_ratio;
        break;
      }

      // 检查非随机性准则 / Check non-randomness criterion
      if (check_non_randomness(inlier_count, n)) {
        LOG_INFO_S << "满足非随机性准则 / Non-randomness criterion satisfied";
        break;
      }
    }

    // 检查最大性准则 / Check maximality criterion
    if (best_inlier_count >= min_inliers && 
        check_maximality(best_inlier_count, n, t)) {
      LOG_INFO_S << "满足最大性准则 / Maximality criterion satisfied";
      break;
    }

    t++;
    m_total_samples++;
  }

  // 设置结果 / Set results
  result.num_iterations = t;
  m_best_inlier_count = best_inlier_count;

  if (best_inlier_count >= min_inliers) {
    // 如果需要，使用所有内点精炼结果 / Refine result using all inliers if needed
    if (m_refine_result && best_inlier_count > m_sample_size) {
      LOG_INFO_S << "使用 / Using " << best_inlier_count 
                 << " 个内点精炼变换 / inliers to refine transformation";
      best_transformation = refine_transformation(best_inliers);
      
      // 重新计算内点 / Recompute inliers
      best_inlier_count = count_inliers(best_transformation, best_inliers);
    }

    result.transformation = best_transformation;
    result.inliers = best_inliers;
    result.fitness_score = compute_fitness_score(best_transformation, best_inliers);
    result.converged = true;

    LOG_INFO_S << "PROSAC配准成功 / PROSAC registration successful: "
               << best_inlier_count << " inliers in " << t << " iterations";
  } else {
    LOG_WARN_S << "警告：PROSAC未找到足够的内点 / Warning: PROSAC did not find enough inliers: "
               << best_inlier_count << " < " << min_inliers;
  }

  return result.converged;
}

template<typename DataType>
bool prosac_registration_t<DataType>::validate_input_impl() const
{
  const auto& correspondences = this->get_correspondences();
  
  if (!correspondences || correspondences->empty()) {
    LOG_ERROR_S << "错误：对应关系为空 / Error: Correspondences are empty";
    return false;
  }

  if (m_sorted_indices.empty()) {
    LOG_WARN_S << "警告：未提供排序索引，假设对应关系已排序 / "
                  "Warning: No sorted indices provided, assuming correspondences are sorted";
  } else if (m_sorted_indices.size() != correspondences->size()) {
    LOG_ERROR_S << "错误：排序索引大小与对应关系不匹配 / "
                   "Error: Sorted indices size doesn't match correspondences";
    return false;
  }

  return true;
}

template<typename DataType>
void prosac_registration_t<DataType>::precompute_sampling_schedule(
    std::size_t n_correspondences)
{
  m_T_n.clear();
  m_T_n.reserve(n_correspondences);

  // T_m初始值 / Initial value of T_m
  DataType T_m = static_cast<DataType>(n_correspondences) * 
                 std::pow(1.0 - m_initial_inlier_ratio, 
                         static_cast<DataType>(m_sample_size));
  
  // 添加前m个T值（都是1） / Add first m T values (all are 1)
  for (std::size_t i = 0; i < m_sample_size; ++i) {
    m_T_n.push_back(1);
  }

  // 计算T_n for n = m+1 to N / Compute T_n for n = m+1 to N
  for (std::size_t n = m_sample_size + 1; n <= n_correspondences; ++n) {
    // T_n = T_{n-1} + ceil(T_m * (n - m) / (m * C(n, m)))
    std::size_t T_n_minus_1 = m_T_n.back();
    
    // 避免整数溢出，使用对数计算 / Avoid integer overflow, use logarithmic computation
    DataType log_numerator = std::log(static_cast<DataType>(n - m_sample_size)) + 
                             std::log(T_m);
    DataType log_denominator = std::log(static_cast<DataType>(m_sample_size));
    
    // 计算组合数的对数 / Compute logarithm of binomial coefficient
    for (std::size_t i = 0; i < m_sample_size; ++i) {
      log_denominator += std::log(static_cast<DataType>(n - i)) - 
                         std::log(static_cast<DataType>(m_sample_size - i));
    }
    
    DataType increment = std::exp(log_numerator - log_denominator);
    std::size_t T_n = T_n_minus_1 + static_cast<std::size_t>(std::ceil(increment));
    
    m_T_n.push_back(T_n);
  }
}

template<typename DataType>
void prosac_registration_t<DataType>::progressive_sample(
    std::vector<correspondence_t>& sample, std::size_t n, std::size_t t,
    std::mt19937& generator) const
{
  const auto& correspondences = this->get_correspondences();
  sample.clear();

  if (t >= m_T_n[n - 1]) {
    // PROSAC采样：选择第n个对应关系和前n-1个中的m-1个 / 
    // PROSAC sampling: select nth correspondence and m-1 from first n-1
    
    // 添加第n个对应关系 / Add nth correspondence
    if (!m_sorted_indices.empty()) {
      sample.push_back((*correspondences)[m_sorted_indices[n - 1]]);
    } else {
      sample.push_back((*correspondences)[n - 1]);
    }

    // 从前n-1个中随机选择m-1个 / Randomly select m-1 from first n-1
    std::vector<std::size_t> indices;
    indices.reserve(n - 1);
    for (std::size_t i = 0; i < n - 1; ++i) {
      indices.push_back(i);
    }

    std::shuffle(indices.begin(), indices.end(), generator);

    for (std::size_t i = 0; i < m_sample_size - 1; ++i) {
      if (!m_sorted_indices.empty()) {
        sample.push_back((*correspondences)[m_sorted_indices[indices[i]]]);
      } else {
        sample.push_back((*correspondences)[indices[i]]);
      }
    }
  } else {
    // 标准RANSAC采样：从前n个中随机选择m个 / 
    // Standard RANSAC sampling: randomly select m from first n
    std::vector<std::size_t> indices;
    indices.reserve(n);
    for (std::size_t i = 0; i < n; ++i) {
      indices.push_back(i);
    }

    std::shuffle(indices.begin(), indices.end(), generator);

    for (std::size_t i = 0; i < m_sample_size; ++i) {
      if (!m_sorted_indices.empty()) {
        sample.push_back((*correspondences)[m_sorted_indices[indices[i]]]);
      } else {
        sample.push_back((*correspondences)[indices[i]]);
      }
    }
  }
}

template<typename DataType>
typename prosac_registration_t<DataType>::transformation_t
prosac_registration_t<DataType>::estimate_transformation(
    const std::vector<correspondence_t>& sample) const
{
  const auto& source_cloud = this->get_source_cloud();
  const auto& target_cloud = this->get_target_cloud();

  // 提取样本点 / Extract sample points
  Eigen::Matrix<DataType, 3, Eigen::Dynamic> src_points(3, sample.size());
  Eigen::Matrix<DataType, 3, Eigen::Dynamic> tgt_points(3, sample.size());

  for (std::size_t i = 0; i < sample.size(); ++i) {
    const auto& src_pt = source_cloud->points[sample[i].src_idx];
    const auto& tgt_pt = target_cloud->points[sample[i].dst_idx];

    src_points.col(i) = vector3_t(src_pt.x, src_pt.y, src_pt.z);
    tgt_points.col(i) = vector3_t(tgt_pt.x, tgt_pt.y, tgt_pt.z);
  }

  // 计算质心 / Compute centroids
  vector3_t src_centroid = src_points.rowwise().mean();
  vector3_t tgt_centroid = tgt_points.rowwise().mean();

  // 中心化点云 / Center point clouds
  Eigen::Matrix<DataType, 3, Eigen::Dynamic> src_centered =
      src_points.colwise() - src_centroid;
  Eigen::Matrix<DataType, 3, Eigen::Dynamic> tgt_centered =
      tgt_points.colwise() - tgt_centroid;

  // 计算协方差矩阵 / Compute covariance matrix
  matrix3_t H = src_centered * tgt_centered.transpose();

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
  vector3_t t = tgt_centroid - R * src_centroid;

  // 构建变换矩阵 / Build transformation matrix
  transformation_t transform = transformation_t::Identity();
  transform.template block<3, 3>(0, 0) = R;
  transform.template block<3, 1>(0, 3) = t;

  return transform;
}

template<typename DataType>
std::size_t prosac_registration_t<DataType>::count_inliers(
    const transformation_t& transform, std::vector<std::size_t>& inliers) const
{
  const auto& source_cloud = this->get_source_cloud();
  const auto& target_cloud = this->get_target_cloud();
  const auto& correspondences = this->get_correspondences();
  const auto inlier_threshold = this->get_inlier_threshold();

  inliers.clear();
  inliers.reserve(correspondences->size());

  // 对每个对应关系检查是否为内点 / Check each correspondence for inlier status
  for (std::size_t i = 0; i < correspondences->size(); ++i) {
    const auto& corr = (*correspondences)[i];
    const auto& src_pt = source_cloud->points[corr.src_idx];
    const auto& tgt_pt = target_cloud->points[corr.dst_idx];

    // 变换源点 / Transform source point
    vector3_t src_vec(src_pt.x, src_pt.y, src_pt.z);
    vector3_t transformed = transform.template block<3, 3>(0, 0) * src_vec +
                            transform.template block<3, 1>(0, 3);

    // 计算距离 / Compute distance
    DataType dist = std::sqrt((transformed[0] - tgt_pt.x) * (transformed[0] - tgt_pt.x) +
                              (transformed[1] - tgt_pt.y) * (transformed[1] - tgt_pt.y) +
                              (transformed[2] - tgt_pt.z) * (transformed[2] - tgt_pt.z));

    if (dist <= inlier_threshold) {
      inliers.push_back(i);
    }
  }

  return inliers.size();
}

template<typename DataType>
bool prosac_registration_t<DataType>::check_non_randomness(
    std::size_t inlier_count, std::size_t n) const
{
  // 计算观察到这么多内点的概率 / Compute probability of observing this many inliers
  DataType p_good = 1.0;

  for (std::size_t j = m_sample_size; j <= inlier_count; ++j) {
    DataType beta = compute_beta(j, m_sample_size, n);
    p_good *= (1.0 - beta);
  }

  p_good = 1.0 - p_good;

  return p_good < m_non_randomness_threshold;
}

template<typename DataType>
bool prosac_registration_t<DataType>::check_maximality(
    std::size_t inlier_count, std::size_t n, std::size_t t) const
{
  // 计算找到更好模型所需的期望迭代次数 / 
  // Compute expected iterations to find better model
  DataType inlier_ratio = static_cast<DataType>(inlier_count) / 
                          static_cast<DataType>(n);
  
  if (inlier_ratio <= 0) {
    return false;
  }

  DataType p_better = std::pow(inlier_ratio, static_cast<DataType>(m_sample_size));
  
  if (p_better <= 0) {
    return true;  // 不可能找到更好的模型 / Impossible to find better model
  }

  DataType k_max = std::log(1.0 - m_confidence) / std::log(1.0 - p_better);

  return static_cast<DataType>(t) >= k_max;
}

template<typename DataType>
typename prosac_registration_t<DataType>::transformation_t
prosac_registration_t<DataType>::refine_transformation(
    const std::vector<std::size_t>& inlier_indices) const
{
  const auto& source_cloud = this->get_source_cloud();
  const auto& target_cloud = this->get_target_cloud();
  const auto& correspondences = this->get_correspondences();

  // 提取所有内点 / Extract all inlier points
  Eigen::Matrix<DataType, 3, Eigen::Dynamic> src_points(3, inlier_indices.size());
  Eigen::Matrix<DataType, 3, Eigen::Dynamic> tgt_points(3, inlier_indices.size());

  for (std::size_t i = 0; i < inlier_indices.size(); ++i) {
    const auto& corr = (*correspondences)[inlier_indices[i]];
    const auto& src_pt = source_cloud->points[corr.src_idx];
    const auto& tgt_pt = target_cloud->points[corr.dst_idx];

    src_points.col(i) = vector3_t(src_pt.x, src_pt.y, src_pt.z);
    tgt_points.col(i) = vector3_t(tgt_pt.x, tgt_pt.y, tgt_pt.z);
  }

  // 使用SVD计算最优变换（与estimate_transformation相同的方法） / 
  // Compute optimal transformation using SVD (same method as estimate_transformation)
  vector3_t src_centroid = src_points.rowwise().mean();
  vector3_t tgt_centroid = tgt_points.rowwise().mean();

  Eigen::Matrix<DataType, 3, Eigen::Dynamic> src_centered =
      src_points.colwise() - src_centroid;
  Eigen::Matrix<DataType, 3, Eigen::Dynamic> tgt_centered =
      tgt_points.colwise() - tgt_centroid;

  matrix3_t H = src_centered * tgt_centered.transpose();

  Eigen::JacobiSVD<matrix3_t> svd(H, Eigen::ComputeFullU | Eigen::ComputeFullV);
  matrix3_t U = svd.matrixU();
  matrix3_t V = svd.matrixV();

  matrix3_t R = V * U.transpose();

  if (R.determinant() < 0) {
    V.col(2) *= -1;
    R = V * U.transpose();
  }

  vector3_t t = tgt_centroid - R * src_centroid;

  transformation_t transform = transformation_t::Identity();
  transform.template block<3, 3>(0, 0) = R;
  transform.template block<3, 1>(0, 3) = t;

  return transform;
}

template<typename DataType>
std::size_t prosac_registration_t<DataType>::compute_binomial_coefficient(
    std::size_t n, std::size_t k) const
{
  if (k > n) return 0;
  if (k == 0 || k == n) return 1;

  // 使用Pascal三角形的性质优化计算 / Optimize using Pascal's triangle property
  k = std::min(k, n - k);

  std::size_t result = 1;
  for (std::size_t i = 0; i < k; ++i) {
    result = result * (n - i) / (i + 1);
  }

  return result;
}

template<typename DataType>
DataType prosac_registration_t<DataType>::compute_beta(
    std::size_t i, std::size_t m, std::size_t n) const
{
  if (i < m) return 0;
  if (i > n) return 0;

  // beta(i, m, n) = C(i-1, m-1) * C(n-i, 1) / C(n, m)
  // = i * C(i-1, m-1) / C(n, m)
  
  // 使用对数避免溢出 / Use logarithm to avoid overflow
  DataType log_beta = std::log(static_cast<DataType>(i));
  
  // log(C(i-1, m-1))
  for (std::size_t j = 0; j < m - 1; ++j) {
    log_beta += std::log(static_cast<DataType>(i - 1 - j)) - 
                std::log(static_cast<DataType>(j + 1));
  }
  
  // log(C(n, m))
  for (std::size_t j = 0; j < m; ++j) {
    log_beta -= std::log(static_cast<DataType>(n - j)) - 
                std::log(static_cast<DataType>(j + 1));
  }
  
  return std::exp(log_beta);
}

template<typename DataType>
bool prosac_registration_t<DataType>::is_sample_valid(
    const std::vector<correspondence_t>& sample) const
{
  if (sample.size() < 3) {
    return false;
  }

  const auto& source_cloud = this->get_source_cloud();

  // 检查是否有重复的对应关系 / Check for duplicate correspondences
  std::set<std::size_t> src_indices, dst_indices;
  for (const auto& corr : sample) {
    if (!src_indices.insert(corr.src_idx).second ||
        !dst_indices.insert(corr.dst_idx).second) {
      return false;
    }
  }

  // 检查源点是否共线 / Check if source points are collinear
  const auto& p1 = source_cloud->points[sample[0].src_idx];
  const auto& p2 = source_cloud->points[sample[1].src_idx];
  const auto& p3 = source_cloud->points[sample[2].src_idx];

  vector3_t v1(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
  vector3_t v2(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);

  vector3_t cross = v1.cross(v2);
  DataType cross_norm = cross.norm();

  const DataType collinear_threshold = static_cast<DataType>(1e-6);
  return cross_norm > collinear_threshold;
}

template<typename DataType>
DataType prosac_registration_t<DataType>::compute_fitness_score(
    const transformation_t& transform,
    const std::vector<std::size_t>& inliers) const
{
  if (inliers.empty()) {
    return std::numeric_limits<DataType>::max();
  }

  const auto& source_cloud = this->get_source_cloud();
  const auto& target_cloud = this->get_target_cloud();
  const auto& correspondences = this->get_correspondences();

  DataType total_distance = 0;
  
  // 计算所有内点的平均距离 / Compute average distance of all inliers
  for (std::size_t idx : inliers) {
    const auto& corr = (*correspondences)[idx];
    const auto& src_pt = source_cloud->points[corr.src_idx];
    const auto& tgt_pt = target_cloud->points[corr.dst_idx];

    // 变换源点 / Transform source point
    vector3_t src_vec(src_pt.x, src_pt.y, src_pt.z);
    vector3_t transformed = transform.template block<3, 3>(0, 0) * src_vec +
                            transform.template block<3, 1>(0, 3);

    // 计算距离 / Compute distance
    DataType dist = std::sqrt((transformed[0] - tgt_pt.x) * (transformed[0] - tgt_pt.x) +
                              (transformed[1] - tgt_pt.y) * (transformed[1] - tgt_pt.y) +
                              (transformed[2] - tgt_pt.z) * (transformed[2] - tgt_pt.z));

    total_distance += dist;
  }

  return total_distance / static_cast<DataType>(inliers.size());
}

}  // namespace toolbox::pcl