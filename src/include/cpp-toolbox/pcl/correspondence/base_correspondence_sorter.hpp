#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <numeric>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/pcl/correspondence/base_correspondence_generator.hpp>
#include <cpp-toolbox/types/point.hpp>

// Logger macros
#define LOG_ERROR_S toolbox::logger::thread_logger_t::instance().error_s()
#define LOG_WARN_S toolbox::logger::thread_logger_t::instance().warn_s()
#define LOG_INFO_S toolbox::logger::thread_logger_t::instance().info_s()

namespace toolbox::pcl
{

/**
 * @brief 对应关系排序器的基类（CRTP模式） / Base class for correspondence
 * sorters (CRTP pattern)
 *
 * 该类定义了所有对应关系排序算法的通用接口，用于评估和排序对应关系的质量。
 * This class defines the common interface for all correspondence sorting
 * algorithms, used to evaluate and sort correspondence quality.
 *
 * @tparam Derived 派生类类型 / Derived class type
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or
 * double)
 *
 * @code
 * // 派生类示例 / Derived class example
 * template<typename DataType>
 * class my_sorter_t
 *     : public base_correspondence_sorter_t<
 *         my_sorter_t<DataType>, DataType>
 * {
 * public:
 *     void compute_quality_scores_impl(quality_scores_t& scores) {
 *         // 实现具体的质量评分算法 / Implement specific quality scoring
 *         algorithm
 *     }
 * };
 * @endcode
 */
template<typename Derived, typename DataType>
class CPP_TOOLBOX_EXPORT base_correspondence_sorter_t
{
public:
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;
  using correspondences_ptr = std::shared_ptr<std::vector<correspondence_t>>;
  using quality_scores_t = std::vector<DataType>;
  using sorted_indices_t = std::vector<std::size_t>;

  base_correspondence_sorter_t() = default;
  virtual ~base_correspondence_sorter_t() = default;

  // 删除拷贝，允许移动 / Delete copy, allow move
  base_correspondence_sorter_t(const base_correspondence_sorter_t&) = delete;
  base_correspondence_sorter_t& operator=(const base_correspondence_sorter_t&) =
      delete;
  base_correspondence_sorter_t(base_correspondence_sorter_t&&) = default;
  base_correspondence_sorter_t& operator=(base_correspondence_sorter_t&&) =
      default;

  /**
   * @brief 设置源点云和目标点云 / Set source and target point clouds
   * @param source 源点云 / Source point cloud
   * @param target 目标点云 / Target point cloud
   */
  void set_point_clouds(const point_cloud_ptr& source,
                        const point_cloud_ptr& target)
  {
    m_source_cloud = source;
    m_target_cloud = target;
    m_cached = false;
  }

  /**
   * @brief 设置对应关系 / Set correspondences
   * @param correspondences 对应关系 / Correspondences
   */
  void set_correspondences(const correspondences_ptr& correspondences)
  {
    m_correspondences = correspondences;
    m_cached = false;
  }

  /**
   * @brief 计算质量分数并返回排序后的索引 / Compute quality scores and return
   * sorted indices
   * @param[out] scores 每个对应关系的质量分数 / Quality score for each
   * correspondence
   * @return 按质量降序排序的索引 / Indices sorted by quality in descending
   * order
   */
  sorted_indices_t compute_sorted_indices(quality_scores_t& scores)
  {
    if (!validate_input()) {
      return sorted_indices_t{};
    }

    // 如果已缓存且输入未改变，直接返回 / If cached and input unchanged, return
    // directly
    if (m_cached) {
      scores = m_cached_scores;
      return m_cached_indices;
    }

    // 调用派生类实现计算质量分数 / Call derived class implementation to
    // compute quality scores
    scores.resize(m_correspondences->size());
    static_cast<Derived*>(this)->compute_quality_scores_impl(scores);

    // 创建索引数组并按质量分数排序 / Create index array and sort by quality
    // scores
    sorted_indices_t indices(scores.size());
    std::iota(indices.begin(), indices.end(), 0);

    // 降序排序（质量高的在前） / Sort in descending order (higher quality
    // first)
    std::sort(indices.begin(), indices.end(),
              [&scores](std::size_t i, std::size_t j) {
                return scores[i] > scores[j];
              });

    // 缓存结果 / Cache results
    m_cached_scores = scores;
    m_cached_indices = indices;
    m_cached = true;

    return indices;
  }

  /**
   * @brief 仅计算质量分数，不排序 / Compute quality scores only, without
   * sorting
   * @return 质量分数 / Quality scores
   */
  quality_scores_t compute_quality_scores()
  {
    quality_scores_t scores;
    compute_sorted_indices(scores);
    return scores;
  }

  /**
   * @brief 获取排序器名称 / Get sorter name
   * @return 排序器名称 / Sorter name
   */
  [[nodiscard]] std::string get_sorter_name() const
  {
    return static_cast<const Derived*>(this)->get_sorter_name_impl();
  }

  /**
   * @brief 清除缓存 / Clear cache
   */
  void clear_cache()
  {
    m_cached = false;
    m_cached_scores.clear();
    m_cached_indices.clear();
  }

  /**
   * @brief 启用/禁用并行计算 / Enable/disable parallel computation
   * @param enable true启用，false禁用（默认启用） / true to enable, false to
   * disable (default enabled)
   */
  void enable_parallel(bool enable = true) { m_parallel_enabled = enable; }

  /**
   * @brief 获取并行计算状态 / Get parallel computation status
   * @return 是否启用并行 / Whether parallel is enabled
   */
  [[nodiscard]] bool is_parallel_enabled() const { return m_parallel_enabled; }

protected:
  /**
   * @brief 验证输入数据 / Validate input data
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool validate_input() const
  {
    if (!m_correspondences || m_correspondences->empty()) {
      LOG_ERROR_S << "错误：对应关系为空 / Error: Correspondences are empty";
      return false;
    }

    if (!m_source_cloud || !m_target_cloud) {
      LOG_WARN_S
          << "警告：点云未设置，某些排序器可能需要点云数据 / Warning: Point "
             "clouds not set, some sorters may need point cloud data";
    }

    return static_cast<const Derived*>(this)->validate_input_impl();
  }

  /**
   * @brief 派生类可选的额外验证 / Optional additional validation for derived
   * class
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool validate_input_impl() const { return true; }

  /**
   * @brief 派生类必须实现的排序器名称获取方法 / Sorter name getter that
   * derived class must implement
   * @return 排序器名称 / Sorter name
   */
  [[nodiscard]] std::string get_sorter_name_impl() const
  {
    return "BaseCorrespondenceSorter";
  }

  /**
   * @brief 获取受保护的成员变量访问 / Get access to protected members
   */
  [[nodiscard]] const point_cloud_ptr& get_source_cloud() const
  {
    return m_source_cloud;
  }
  [[nodiscard]] const point_cloud_ptr& get_target_cloud() const
  {
    return m_target_cloud;
  }
  [[nodiscard]] const correspondences_ptr& get_correspondences() const
  {
    return m_correspondences;
  }

  // 数据成员 / Data members
  point_cloud_ptr m_source_cloud;
  point_cloud_ptr m_target_cloud;
  correspondences_ptr m_correspondences;

  // 缓存 / Cache
  bool m_cached = false;
  quality_scores_t m_cached_scores;
  sorted_indices_t m_cached_indices;

  // 并行计算开关（默认开启） / Parallel computation switch (default enabled)
  bool m_parallel_enabled = true;
};

}  // namespace toolbox::pcl