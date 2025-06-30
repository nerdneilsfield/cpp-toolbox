#pragma once

#include <future>
#include <numeric>
#include <thread>

#include <cpp-toolbox/concurrent/parallel.hpp>
#include <cpp-toolbox/metrics/point_cloud_metrics.hpp>
#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/utils/timer.hpp>

namespace toolbox::pcl
{

template<typename DataType>
bool ransac_registration_t<DataType>::align_impl(result_type& result)
{
  // 初始化结果 / Initialize result
  result.transformation.setIdentity();
  result.fitness_score = std::numeric_limits<DataType>::max();
  result.inliers.clear();
  result.num_iterations = 0;
  result.converged = false;

  // 获取对应关系 / Get correspondences
  auto correspondences = this->get_correspondences();
  if (!correspondences || correspondences->empty()) {
    LOG_ERROR_S << "RANSAC: 没有提供对应关系 / No correspondences provided";
    return false;
  }

  // 如果对应关系太少 / If too few correspondences
  if (correspondences->size() < m_sample_size) {
    LOG_ERROR_S << "RANSAC: 对应关系数量不足 / Insufficient correspondences: "
                << correspondences->size() << " < " << m_sample_size;
    return false;
  }

  LOG_DEBUG_S << "RANSAC: 开始配准，对应关系数量 / Starting registration with "
                 "correspondences: "
              << correspondences->size();

  // 初始化随机数生成器 / Initialize random number generator
  std::mt19937 generator(this->get_random_seed());

  // 计算所需迭代次数 / Calculate required iterations
  DataType outlier_ratio =
      static_cast<DataType>(0.5);  // 初始估计 / Initial estimate
  std::size_t max_iterations = this->get_max_iterations();
  std::size_t adaptive_iterations = calculate_iterations(outlier_ratio);
  std::size_t iterations = std::min(max_iterations, adaptive_iterations);

  // 最佳结果 / Best result
  transformation_t best_transform;
  best_transform.setIdentity();
  std::vector<std::size_t> best_inliers;
  std::size_t best_inlier_count = 0;

  // 收敛检查变量 / Convergence check variables
  const std::size_t convergence_window_size = 20;  // 滑动窗口大小 / Sliding window size
  std::vector<std::size_t> inlier_history;  // 记录每次迭代的最佳内点数 / Record best inlier count for each iteration
  inlier_history.reserve(convergence_window_size);

  // 计时器 / Timer
  toolbox::utils::stop_watch_timer_t timer("RANSAC");
  timer.start();

  // 主RANSAC循环 / Main RANSAC loop
  for (std::size_t iter = 0; iter < iterations; ++iter) {
    result.num_iterations = iter + 1;

    // 随机采样 / Random sampling
    std::vector<correspondence_t> sample;
    sample_correspondences(sample, generator);

    // 估计变换 / Estimate transformation
    transformation_t transform = estimate_rigid_transform_svd(sample);

    // 计算内点 / Count inliers
    std::vector<std::size_t> inliers;
    std::size_t inlier_count = count_inliers(transform, inliers);

    // 更新最佳结果 / Update best result
    if (inlier_count > best_inlier_count) {
      best_transform = transform;
      best_inliers = std::move(inliers);
      best_inlier_count = inlier_count;

      // 更新自适应迭代次数 / Update adaptive iterations
      outlier_ratio =
          static_cast<DataType>(correspondences->size() - best_inlier_count)
          / correspondences->size();
      adaptive_iterations = calculate_iterations(outlier_ratio);
      iterations = std::min(max_iterations, adaptive_iterations);

      // 早停检查 / Early stopping check
      DataType inlier_ratio =
          static_cast<DataType>(best_inlier_count) / correspondences->size();
      if (inlier_ratio >= m_early_stop_ratio) {
        LOG_DEBUG_S << "RANSAC: 早停，内点比例 / Early stopping, inlier ratio: "
                    << inlier_ratio;
        break;
      }
    }

    // 收敛检查 - 每次迭代都检查 / Convergence check - check every iteration
    // 记录当前最佳内点数 / Record current best inlier count
    inlier_history.push_back(best_inlier_count);
    
    // 如果历史记录已满，使用滑动窗口 / Use sliding window if history is full
    if (inlier_history.size() > convergence_window_size) {
      inlier_history.erase(inlier_history.begin());
    }
    
    // 需要足够的历史数据才能判断收敛 / Need enough history to check convergence
    if (inlier_history.size() >= convergence_window_size) {
      // 计算窗口内的总改进 / Calculate total improvement in window
      std::size_t window_improvement = inlier_history.back() - inlier_history.front();
      
      // 计算平均每次迭代的改进 / Calculate average improvement per iteration
      DataType avg_improvement_per_iter = static_cast<DataType>(window_improvement) 
          / static_cast<DataType>(convergence_window_size - 1);
      
      // 计算相对改进率 / Calculate relative improvement rate
      DataType relative_improvement = 0.0;
      if (inlier_history.front() > 0) {
        relative_improvement = static_cast<DataType>(window_improvement) 
            / static_cast<DataType>(inlier_history.front());
      }
      
      // 收敛条件：绝对改进和相对改进都很小 / Convergence: both absolute and relative improvements are small
      const DataType min_avg_improvement = 0.5;  // 平均每次迭代至少增加0.5个内点 / At least 0.5 inlier per iteration on average
      const DataType min_relative_improvement = 0.01;  // 或者1%的相对改进 / Or 1% relative improvement
      
      if (avg_improvement_per_iter < min_avg_improvement && 
          relative_improvement < min_relative_improvement) {
        LOG_DEBUG_S << "RANSAC: 收敛，最近 " << convergence_window_size 
                    << " 次迭代内点数增加 / Converged, inlier count improved by " 
                    << window_improvement << " 个 / in last " << convergence_window_size 
                    << " iterations (相对改进 / relative improvement: " 
                    << relative_improvement * 100 << "%)";
        result.converged = true;
        break;
      }
    }
  }

  timer.stop();
  double elapsed_time = timer.elapsed_time();
  LOG_DEBUG_S << "RANSAC: 完成 " << result.num_iterations
              << " 次迭代，耗时 / iterations in: " << elapsed_time << " 秒/s";

  // 检查是否找到足够的内点 / Check if enough inliers found
  if (best_inlier_count < this->get_min_inliers()) {
    LOG_ERROR_S << "RANSAC: 内点数量不足 / Insufficient inliers: "
                << best_inlier_count << " < " << this->get_min_inliers();
    return false;
  }

  // 精炼结果（可选） / Refine result (optional)
  if (m_refine_result && best_inlier_count >= m_sample_size) {
    LOG_DEBUG_S << "RANSAC: 使用 " << best_inlier_count
                << " 个内点精炼结果 / Refining with inliers";
    best_transform = refine_transformation(best_inliers);

    // 重新计算内点 / Recompute inliers
    best_inlier_count = count_inliers(best_transform, best_inliers);
  }

  // 设置结果 / Set result
  result.transformation = best_transform;
  result.inliers = std::move(best_inliers);
  result.fitness_score = compute_fitness_score(best_transform, result.inliers);
  result.converged =
      result.converged || (best_inlier_count >= this->get_min_inliers());

  LOG_DEBUG_S << "RANSAC: 配准完成，内点 / Registration complete, inliers: "
              << result.inliers.size() << "/" << correspondences->size()
              << ", 质量评分 / fitness score: " << result.fitness_score;

  return true;
}

template<typename DataType>
bool ransac_registration_t<DataType>::validate_input_impl() const
{
  // 基类已经验证了点云，这里验证RANSAC特定的输入 / Base class validated clouds,
  // validate RANSAC-specific input
  auto correspondences = this->get_correspondences();
  if (!correspondences || correspondences->empty()) {
    LOG_ERROR_S << "RANSAC: 需要对应关系 / Correspondences required";
    return false;
  }

  if (correspondences->size() < m_sample_size) {
    LOG_ERROR_S << "RANSAC: 对应关系数量不足以进行采样 / Not enough "
                   "correspondences for sampling";
    return false;
  }

  return true;
}

template<typename DataType>
std::size_t ransac_registration_t<DataType>::calculate_iterations(
    DataType outlier_ratio) const
{
  // N = log(1 - p) / log(1 - (1 - e)^s)
  // p = confidence, e = outlier_ratio, s = sample_size

  if (outlier_ratio <= 0 || outlier_ratio >= 1) {
    return this->get_max_iterations();
  }

  DataType inlier_ratio = 1 - outlier_ratio;
  DataType sample_success_prob =
      std::pow(inlier_ratio, static_cast<DataType>(m_sample_size));

  if (sample_success_prob <= 0 || sample_success_prob >= 1) {
    return this->get_max_iterations();
  }

  DataType num_iterations =
      std::log(1 - m_confidence) / std::log(1 - sample_success_prob);

  return static_cast<std::size_t>(std::ceil(num_iterations));
}

template<typename DataType>
void ransac_registration_t<DataType>::sample_correspondences(
    std::vector<correspondence_t>& sample, std::mt19937& generator) const
{
  auto correspondences = this->get_correspondences();
  const std::size_t num_correspondences = correspondences->size();

  // 清空并预留空间 / Clear and reserve space
  sample.clear();
  sample.reserve(m_sample_size);

  // 使用Fisher-Yates洗牌算法的变体进行无重复采样 / Use variant of Fisher-Yates
  // shuffle for sampling without replacement
  std::vector<std::size_t> indices(num_correspondences);
  std::iota(indices.begin(), indices.end(), 0);

  for (std::size_t i = 0; i < m_sample_size; ++i) {
    std::uniform_int_distribution<std::size_t> dist(i, num_correspondences - 1);
    std::size_t j = dist(generator);
    std::swap(indices[i], indices[j]);
    sample.push_back((*correspondences)[indices[i]]);
  }
}

template<typename DataType>
typename ransac_registration_t<DataType>::transformation_t
ransac_registration_t<DataType>::estimate_rigid_transform_svd(
    const std::vector<correspondence_t>& sample) const
{
  const std::size_t n = sample.size();
  transformation_t transform;
  transform.setIdentity();

  if (n < 3) {
    LOG_WARN_S << "RANSAC: 样本数量不足以估计变换 / Insufficient samples for "
                  "transformation estimation";
    return transform;
  }

  auto source_cloud = this->get_source_cloud();
  auto target_cloud = this->get_target_cloud();

  // 计算质心 / Compute centroids
  vector3_t source_centroid = vector3_t::Zero();
  vector3_t target_centroid = vector3_t::Zero();

  for (const auto& corr : sample) {
    const auto& src_pt = source_cloud->points[corr.src_idx];
    const auto& tgt_pt = target_cloud->points[corr.dst_idx];

    source_centroid += vector3_t(src_pt.x, src_pt.y, src_pt.z);
    target_centroid += vector3_t(tgt_pt.x, tgt_pt.y, tgt_pt.z);
  }

  source_centroid /= static_cast<DataType>(n);
  target_centroid /= static_cast<DataType>(n);

  // 构建协方差矩阵 / Build covariance matrix
  matrix3_t H = matrix3_t::Zero();

  for (const auto& corr : sample) {
    const auto& src_pt = source_cloud->points[corr.src_idx];
    const auto& tgt_pt = target_cloud->points[corr.dst_idx];

    vector3_t src_centered(src_pt.x - source_centroid[0],
                           src_pt.y - source_centroid[1],
                           src_pt.z - source_centroid[2]);
    vector3_t tgt_centered(tgt_pt.x - target_centroid[0],
                           tgt_pt.y - target_centroid[1],
                           tgt_pt.z - target_centroid[2]);

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
std::size_t ransac_registration_t<DataType>::count_inliers(
    const transformation_t& transform, std::vector<std::size_t>& inliers) const
{
  inliers.clear();

  auto correspondences = this->get_correspondences();
  auto source_cloud = this->get_source_cloud();
  auto target_cloud = this->get_target_cloud();

  const DataType threshold_squared =
      this->get_inlier_threshold() * this->get_inlier_threshold();

  // 根据是否启用并行选择实现 / Choose implementation based on parallel enabled
  if (this->is_parallel_enabled()) {
    // 并行版本 / Parallel version
    // 获取线程数 / Get number of threads
    const std::size_t num_threads = std::thread::hardware_concurrency();
    std::vector<std::vector<std::size_t>> local_inliers(num_threads);

    // 将对应关系分成多个块 / Divide correspondences into chunks
    const std::size_t chunk_size =
        (correspondences->size() + num_threads - 1) / num_threads;
    std::vector<std::future<void>> futures;

    for (std::size_t thread_id = 0; thread_id < num_threads; ++thread_id) {
      std::size_t start = thread_id * chunk_size;
      std::size_t end = std::min(start + chunk_size, correspondences->size());

      if (start >= end)
        break;

      futures.push_back(std::async(
          std::launch::async,
          [&, thread_id, start, end]()
          {
            auto& thread_inliers = local_inliers[thread_id];

            for (std::size_t i = start; i < end; ++i) {
              const auto& corr = (*correspondences)[i];
              const auto& src_pt = source_cloud->points[corr.src_idx];

              // 变换源点 / Transform source point
              vector3_t src_vec(src_pt.x, src_pt.y, src_pt.z);
              vector3_t transformed =
                  transform.template block<3, 3>(0, 0) * src_vec
                  + transform.template block<3, 1>(0, 3);

              // 计算到目标点的距离 / Compute distance to target point
              const auto& tgt_pt = target_cloud->points[corr.dst_idx];
              DataType dx = transformed[0] - tgt_pt.x;
              DataType dy = transformed[1] - tgt_pt.y;
              DataType dz = transformed[2] - tgt_pt.z;
              DataType dist_squared = dx * dx + dy * dy + dz * dz;

              if (dist_squared <= threshold_squared) {
                thread_inliers.push_back(i);
              }
            }
          }));
    }

    // 等待所有线程完成 / Wait for all threads to complete
    for (auto& future : futures) {
      future.wait();
    }

    // 合并结果 / Merge results
    for (const auto& thread_inliers : local_inliers) {
      inliers.insert(
          inliers.end(), thread_inliers.begin(), thread_inliers.end());
    }
  } else {
    // 串行版本 / Sequential version
    for (std::size_t i = 0; i < correspondences->size(); ++i) {
      const auto& corr = (*correspondences)[i];
      const auto& src_pt = source_cloud->points[corr.src_idx];

      // 变换源点 / Transform source point
      vector3_t src_vec(src_pt.x, src_pt.y, src_pt.z);
      vector3_t transformed = transform.template block<3, 3>(0, 0) * src_vec
          + transform.template block<3, 1>(0, 3);

      // 计算到目标点的距离 / Compute distance to target point
      const auto& tgt_pt = target_cloud->points[corr.dst_idx];
      DataType dx = transformed[0] - tgt_pt.x;
      DataType dy = transformed[1] - tgt_pt.y;
      DataType dz = transformed[2] - tgt_pt.z;
      DataType dist_squared = dx * dx + dy * dy + dz * dz;

      if (dist_squared <= threshold_squared) {
        inliers.push_back(i);
      }
    }
  }

  return inliers.size();
}

template<typename DataType>
typename ransac_registration_t<DataType>::transformation_t
ransac_registration_t<DataType>::refine_transformation(
    const std::vector<std::size_t>& inlier_indices) const
{
  // 使用所有内点重新估计变换 / Re-estimate transformation using all inliers
  auto correspondences = this->get_correspondences();
  std::vector<correspondence_t> inlier_correspondences;
  inlier_correspondences.reserve(inlier_indices.size());

  for (std::size_t idx : inlier_indices) {
    inlier_correspondences.push_back((*correspondences)[idx]);
  }

  return estimate_rigid_transform_svd(inlier_correspondences);
}

template<typename DataType>
DataType ransac_registration_t<DataType>::compute_fitness_score(
    const transformation_t& transform,
    const std::vector<std::size_t>& inliers) const
{
  if (inliers.empty()) {
    return std::numeric_limits<DataType>::max();
  }

  // 使用 LCPMetric 计算适应度分数 / Use LCPMetric to compute fitness score
  toolbox::metrics::LCPMetric<DataType> lcp_metric(this->get_inlier_threshold());
  
  auto correspondences = this->get_correspondences();
  auto source_cloud = this->get_source_cloud();
  auto target_cloud = this->get_target_cloud();
  
  // 创建内点对应的源点云和目标点云 / Create source and target clouds for inliers
  toolbox::types::point_cloud_t<DataType> inlier_source, inlier_target;
  inlier_source.points.reserve(inliers.size());
  inlier_target.points.reserve(inliers.size());
  
  for (std::size_t idx : inliers) {
    const auto& corr = (*correspondences)[idx];
    inlier_source.points.push_back(source_cloud->points[corr.src_idx]);
    inlier_target.points.push_back(target_cloud->points[corr.dst_idx]);
  }
  
  // 计算LCP分数 / Compute LCP score
  return lcp_metric.compute_lcp_score(inlier_source, inlier_target, transform);
}

// 显式实例化 / Explicit instantiation
template class ransac_registration_t<float>;
template class ransac_registration_t<double>;

}  // namespace toolbox::pcl