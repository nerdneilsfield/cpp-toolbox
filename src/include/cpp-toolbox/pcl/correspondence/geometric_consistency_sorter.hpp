#pragma once

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include <cpp-toolbox/base/thread_pool.hpp>
#include <cpp-toolbox/base/thread_pool_singleton.hpp>
#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/correspondence/base_correspondence_sorter.hpp>

namespace toolbox::pcl
{

/**
 * @brief 基于几何一致性的对应关系排序器 / Geometric consistency-based
 * correspondence sorter
 *
 * 该排序器通过检查对应关系之间的几何关系保持程度来评估质量。
 * 如果源点云中两点之间的距离与目标点云中对应两点之间的距离相似，
 * 则认为这些对应关系具有高几何一致性。
 * This sorter evaluates quality by checking how well geometric relationships
 * are preserved between correspondences. If the distance between two points in
 * the source cloud is similar to the distance between the corresponding two
 * points in the target cloud, these correspondences are considered to have high
 * geometric consistency.
 *
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or
 * double)
 */
template<typename DataType>
class CPP_TOOLBOX_EXPORT geometric_consistency_sorter_t
    : public base_correspondence_sorter_t<
          geometric_consistency_sorter_t<DataType>, DataType>
{
public:
  using base_type =
      base_correspondence_sorter_t<geometric_consistency_sorter_t<DataType>,
                                   DataType>;
  using typename base_type::point_cloud;
  using typename base_type::quality_scores_t;

  geometric_consistency_sorter_t() = default;
  ~geometric_consistency_sorter_t() = default;

  /**
   * @brief 设置用于评估的邻域大小 / Set neighborhood size for evaluation
   * @param size 邻域大小（要检查的其他对应关系数量） / Neighborhood size (number
   * of other correspondences to check)
   */
  void set_neighborhood_size(std::size_t size)
  {
    m_neighborhood_size = size;
    this->m_cached = false;
  }

  /**
   * @brief 获取邻域大小 / Get neighborhood size
   * @return 邻域大小 / Neighborhood size
   */
  [[nodiscard]] std::size_t get_neighborhood_size() const
  {
    return m_neighborhood_size;
  }

  /**
   * @brief 设置距离比率阈值 / Set distance ratio threshold
   * @param threshold 阈值（例如0.2表示20%的距离变化容忍度） / Threshold (e.g.,
   * 0.2 means 20% distance variation tolerance)
   */
  void set_distance_ratio_threshold(DataType threshold)
  {
    m_distance_ratio_threshold = threshold;
    this->m_cached = false;
  }

  /**
   * @brief 获取距离比率阈值 / Get distance ratio threshold
   * @return 距离比率阈值 / Distance ratio threshold
   */
  [[nodiscard]] DataType get_distance_ratio_threshold() const
  {
    return m_distance_ratio_threshold;
  }

  /**
   * @brief 设置采样策略 / Set sampling strategy
   * @param random true使用随机采样，false使用最近的邻域 / true for random
   * sampling, false for nearest neighbors
   */
  void set_random_sampling(bool random)
  {
    m_random_sampling = random;
    this->m_cached = false;
  }

  /**
   * @brief 获取是否使用随机采样 / Get whether to use random sampling
   * @return 是否随机采样 / Whether to use random sampling
   */
  [[nodiscard]] bool get_random_sampling() const { return m_random_sampling; }

protected:
  // Friend declaration to allow base class access
  friend class base_correspondence_sorter_t<
      geometric_consistency_sorter_t<DataType>, DataType>;

  /**
   * @brief 验证输入数据的实现 / Implementation of input validation
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool validate_input_impl() const
  {
    if (!this->m_source_cloud || !this->m_target_cloud) {
      LOG_ERROR_S << "错误：几何一致性排序器需要点云数据 / Error: Geometric "
                     "consistency sorter requires point cloud data";
      return false;
    }
    return true;
  }

  /**
   * @brief 计算质量分数的实现 / Implementation of quality score computation
   * @param scores [out] 质量分数 / Quality scores
   */
  void compute_quality_scores_impl(quality_scores_t& scores)
  {
    const auto& corrs = *this->m_correspondences;
    const auto& src_cloud = *this->m_source_cloud;
    const auto& tgt_cloud = *this->m_target_cloud;

    if (this->m_parallel_enabled &&
        corrs.size() > 100)  // 只有足够多的对应关系才并行 / Parallelize only
                             // with enough correspondences
    {
      // 使用线程池并行计算 / Use thread pool for parallel computation
      auto& thread_pool = toolbox::base::thread_pool_singleton_t::instance();
      std::vector<std::future<void>> futures;
      futures.reserve(corrs.size());

      for (std::size_t i = 0; i < corrs.size(); ++i) {
        futures.emplace_back(thread_pool.submit([this, i, &scores, &corrs,
                                                  &src_cloud, &tgt_cloud]() {
          scores[i] = compute_single_consistency(i, corrs, src_cloud, tgt_cloud);
        }));
      }

      // 等待所有任务完成 / Wait for all tasks to complete
      for (auto& future : futures) {
        future.get();
      }
    } else {
      // 串行计算 / Serial computation
      for (std::size_t i = 0; i < corrs.size(); ++i) {
        scores[i] = compute_single_consistency(i, corrs, src_cloud, tgt_cloud);
      }
    }
  }

  /**
   * @brief 获取排序器名称实现 / Get sorter name implementation
   * @return 排序器名称 / Sorter name
   */
  [[nodiscard]] std::string get_sorter_name_impl() const
  {
    return "GeometricConsistency";
  }

private:
  /**
   * @brief 计算单个对应关系的几何一致性 / Compute geometric consistency for a
   * single correspondence
   * @param idx 对应关系索引 / Correspondence index
   * @param corrs 所有对应关系 / All correspondences
   * @param src_cloud 源点云 / Source point cloud
   * @param tgt_cloud 目标点云 / Target point cloud
   * @return 一致性分数 / Consistency score
   */
  DataType compute_single_consistency(
      std::size_t idx, const std::vector<correspondence_t>& corrs,
      const point_cloud& src_cloud, const point_cloud& tgt_cloud) const
  {
    const auto& corr = corrs[idx];
    DataType consistency_score = 0;
    std::size_t valid_pairs = 0;

    // 确定要检查的对应关系 / Determine correspondences to check
    std::vector<std::size_t> indices_to_check;

    if (m_random_sampling) {
      // 随机选择其他对应关系 / Randomly select other correspondences
      indices_to_check = get_random_indices(idx, corrs.size(),
                                            m_neighborhood_size);
    } else {
      // 选择最近的对应关系（基于描述子距离） / Select nearest correspondences
      // (based on descriptor distance)
      indices_to_check = get_nearest_indices(idx, corrs, m_neighborhood_size);
    }

    // 计算与其他对应关系的几何一致性 / Compute geometric consistency with
    // other correspondences
    for (std::size_t j : indices_to_check) {
      // 计算源点对之间的距离 / Compute distance between source point pair
      const auto& src_p1 = src_cloud.points[corr.src_idx];
      const auto& src_p2 = src_cloud.points[corrs[j].src_idx];
      DataType src_dist = compute_distance(src_p1, src_p2);

      // 计算目标点对之间的距离 / Compute distance between target point pair
      const auto& tgt_p1 = tgt_cloud.points[corr.dst_idx];
      const auto& tgt_p2 = tgt_cloud.points[corrs[j].dst_idx];
      DataType tgt_dist = compute_distance(tgt_p1, tgt_p2);

      // 检查距离比率 / Check distance ratio
      if (src_dist > m_min_distance && tgt_dist > m_min_distance) {
        DataType ratio = src_dist / tgt_dist;
        if (ratio > (1.0 - m_distance_ratio_threshold) &&
            ratio < (1.0 + m_distance_ratio_threshold)) {
          consistency_score += 1.0;
        }
        valid_pairs++;
      }
    }

    return valid_pairs > 0 ? consistency_score / valid_pairs : 0;
  }

  /**
   * @brief 计算两点之间的欧氏距离 / Compute Euclidean distance between two
   * points
   * @tparam PointT 点类型 / Point type
   * @param p1 第一个点 / First point
   * @param p2 第二个点 / Second point
   * @return 距离 / Distance
   */
  template<typename PointT>
  DataType compute_distance(const PointT& p1, const PointT& p2) const
  {
    return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) +
                     (p1.y - p2.y) * (p1.y - p2.y) +
                     (p1.z - p2.z) * (p1.z - p2.z));
  }

  /**
   * @brief 获取随机索引（排除当前索引） / Get random indices (excluding
   * current index)
   * @param exclude_idx 要排除的索引 / Index to exclude
   * @param total_size 总大小 / Total size
   * @param num_samples 要采样的数量 / Number of samples
   * @return 随机索引 / Random indices
   */
  std::vector<std::size_t> get_random_indices(std::size_t exclude_idx,
                                               std::size_t total_size,
                                               std::size_t num_samples) const
  {
    std::vector<std::size_t> indices;
    indices.reserve(num_samples);

    // 创建所有可能的索引（排除当前索引） / Create all possible indices
    // (excluding current)
    std::vector<std::size_t> all_indices;
    all_indices.reserve(total_size - 1);
    for (std::size_t i = 0; i < total_size; ++i) {
      if (i != exclude_idx) {
        all_indices.push_back(i);
      }
    }

    // 随机打乱并选择前num_samples个 / Shuffle randomly and select first
    // num_samples
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(all_indices.begin(), all_indices.end(), gen);

    std::size_t actual_samples = std::min(num_samples, all_indices.size());
    indices.insert(indices.end(), all_indices.begin(),
                   all_indices.begin() + actual_samples);

    return indices;
  }

  /**
   * @brief 获取最近的索引（基于描述子距离） / Get nearest indices (based on
   * descriptor distance)
   * @param current_idx 当前索引 / Current index
   * @param corrs 所有对应关系 / All correspondences
   * @param num_neighbors 邻居数量 / Number of neighbors
   * @return 最近的索引 / Nearest indices
   */
  std::vector<std::size_t> get_nearest_indices(
      std::size_t current_idx, const std::vector<correspondence_t>& corrs,
      std::size_t num_neighbors) const
  {
    // 创建索引-距离对 / Create index-distance pairs
    std::vector<std::pair<std::size_t, float>> index_distance_pairs;
    index_distance_pairs.reserve(corrs.size() - 1);

    float current_distance = corrs[current_idx].distance;

    for (std::size_t i = 0; i < corrs.size(); ++i) {
      if (i != current_idx) {
        float dist_diff = std::abs(corrs[i].distance - current_distance);
        index_distance_pairs.emplace_back(i, dist_diff);
      }
    }

    // 按距离差排序 / Sort by distance difference
    std::sort(index_distance_pairs.begin(), index_distance_pairs.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    // 提取前num_neighbors个索引 / Extract first num_neighbors indices
    std::vector<std::size_t> indices;
    indices.reserve(num_neighbors);
    std::size_t actual_neighbors =
        std::min(num_neighbors, index_distance_pairs.size());

    for (std::size_t i = 0; i < actual_neighbors; ++i) {
      indices.push_back(index_distance_pairs[i].first);
    }

    return indices;
  }

  // 参数 / Parameters
  std::size_t m_neighborhood_size = 10;  ///< 邻域大小 / Neighborhood size
  DataType m_distance_ratio_threshold =
      static_cast<DataType>(0.2);  ///< 距离比率阈值（20%） / Distance ratio
                                   ///< threshold (20%)
  bool m_random_sampling = true;  ///< 是否使用随机采样 / Whether to use random
                                  ///< sampling
  DataType m_min_distance =
      static_cast<DataType>(0.001);  ///< 最小距离阈值，避免除零 / Minimum
                                     ///< distance threshold to avoid division
                                     ///< by zero
};

}  // namespace toolbox::pcl