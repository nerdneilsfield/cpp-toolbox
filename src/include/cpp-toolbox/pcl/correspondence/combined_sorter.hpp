#pragma once

#include <algorithm>
#include <memory>
#include <numeric>
#include <utility>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/correspondence/base_correspondence_sorter.hpp>

namespace toolbox::pcl
{

/**
 * @brief 组合多个排序器的对应关系排序器 / Correspondence sorter that combines
 * multiple sorters
 *
 * 该排序器可以组合多个子排序器的结果，通过加权平均计算最终质量分数。
 * This sorter can combine results from multiple sub-sorters by computing a
 * weighted average of their quality scores.
 *
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or
 * double)
 *
 * @code
 * // 使用示例 / Usage example
 * combined_sorter_t<float> combined_sorter;
 *
 * // 添加描述子距离排序器（权重0.6） / Add descriptor distance sorter (weight
 * 0.6) auto dist_sorter =
 * std::make_shared<descriptor_distance_sorter_t<float>>();
 * combined_sorter.add_sorter(dist_sorter, 0.6f);
 *
 * // 添加几何一致性排序器（权重0.4） / Add geometric consistency sorter
 * (weight 0.4) auto geom_sorter =
 * std::make_shared<geometric_consistency_sorter_t<float>>();
 * combined_sorter.add_sorter(geom_sorter, 0.4f);
 *
 * // 归一化权重 / Normalize weights
 * combined_sorter.normalize_weights();
 * @endcode
 */
template<typename DataType>
class CPP_TOOLBOX_EXPORT combined_sorter_t
    : public base_correspondence_sorter_t<combined_sorter_t<DataType>, DataType>
{
public:
  using base_type =
      base_correspondence_sorter_t<combined_sorter_t<DataType>, DataType>;
  using typename base_type::point_cloud_ptr;
  using typename base_type::correspondences_ptr;
  using typename base_type::quality_scores_t;

  combined_sorter_t() = default;
  ~combined_sorter_t() = default;

  /**
   * @brief 添加子排序器及其权重 / Add a sub-sorter with its weight
   * @tparam SorterType 排序器类型 / Sorter type
   * @param sorter 排序器实例 / Sorter instance
   * @param weight 权重 / Weight
   */
  template<typename SorterType>
  void add_sorter(std::shared_ptr<SorterType> sorter, DataType weight)
  {
    static_assert(
        std::is_base_of_v<
            base_correspondence_sorter_t<SorterType, DataType>, SorterType>,
        "SorterType must inherit from base_correspondence_sorter_t");

    m_sorters.emplace_back(
        std::make_unique<sorter_wrapper_impl_t<SorterType>>(sorter), weight);
    this->m_cached = false;
  }

  /**
   * @brief 归一化权重，使其总和为1 / Normalize weights to sum to 1
   */
  void normalize_weights()
  {
    DataType sum = 0;
    for (const auto& [sorter, weight] : m_sorters) {
      sum += weight;
    }

    if (sum > std::numeric_limits<DataType>::epsilon()) {
      for (auto& [sorter, weight] : m_sorters) {
        weight /= sum;
      }
    }
    this->m_cached = false;
  }

  /**
   * @brief 获取子排序器数量 / Get number of sub-sorters
   * @return 子排序器数量 / Number of sub-sorters
   */
  [[nodiscard]] std::size_t get_num_sorters() const { return m_sorters.size(); }

  /**
   * @brief 获取指定索引的权重 / Get weight at specified index
   * @param index 索引 / Index
   * @return 权重 / Weight
   */
  [[nodiscard]] DataType get_weight(std::size_t index) const
  {
    if (index >= m_sorters.size()) {
      return 0;
    }
    return m_sorters[index].second;
  }

  /**
   * @brief 清除所有子排序器 / Clear all sub-sorters
   */
  void clear_sorters()
  {
    m_sorters.clear();
    this->m_cached = false;
  }

protected:
  // Friend declaration to allow base class access
  friend class base_correspondence_sorter_t<combined_sorter_t<DataType>,
                                            DataType>;

  /**
   * @brief 计算质量分数的实现 / Implementation of quality score computation
   * @param scores [out] 质量分数 / Quality scores
   */
  void compute_quality_scores_impl(quality_scores_t& scores)
  {
    if (m_sorters.empty()) {
      LOG_WARN_S << "警告：组合排序器没有子排序器 / Warning: Combined sorter "
                    "has no sub-sorters";
      std::fill(scores.begin(), scores.end(), 0);
      return;
    }

    // 初始化分数为0 / Initialize scores to 0
    std::fill(scores.begin(), scores.end(), 0);

    // 计算加权组合分数 / Compute weighted combined scores
    for (const auto& [sorter_wrapper, weight] : m_sorters) {
      // 设置数据 / Set data
      sorter_wrapper->set_point_clouds(this->m_source_cloud,
                                        this->m_target_cloud);
      sorter_wrapper->set_correspondences(this->m_correspondences);

      // 计算子排序器的分数 / Compute sub-sorter scores
      quality_scores_t sub_scores = sorter_wrapper->compute_quality_scores();

      // 加权累加 / Weighted accumulation
      for (std::size_t i = 0; i < scores.size(); ++i) {
        scores[i] += weight * sub_scores[i];
      }
    }
  }

  /**
   * @brief 获取排序器名称实现 / Get sorter name implementation
   * @return 排序器名称 / Sorter name
   */
  [[nodiscard]] std::string get_sorter_name_impl() const
  {
    std::string name = "Combined(";
    for (std::size_t i = 0; i < m_sorters.size(); ++i) {
      if (i > 0) name += ",";
      name += m_sorters[i].first->get_sorter_name();
      name += ":" + std::to_string(m_sorters[i].second);
    }
    name += ")";
    return name;
  }

private:
  // Type-erased wrapper for sorters
  class sorter_wrapper_base_t
  {
  public:
    virtual ~sorter_wrapper_base_t() = default;
    virtual void set_point_clouds(const point_cloud_ptr& source,
                                  const point_cloud_ptr& target) = 0;
    virtual void set_correspondences(
        const correspondences_ptr& correspondences) = 0;
    virtual quality_scores_t compute_quality_scores() = 0;
    virtual std::string get_sorter_name() const = 0;
  };

  template<typename SorterType>
  class sorter_wrapper_impl_t : public sorter_wrapper_base_t
  {
  public:
    explicit sorter_wrapper_impl_t(std::shared_ptr<SorterType> sorter)
        : m_sorter(sorter)
    {
    }

    void set_point_clouds(const point_cloud_ptr& source,
                          const point_cloud_ptr& target) override
    {
      m_sorter->set_point_clouds(source, target);
    }

    void set_correspondences(
        const correspondences_ptr& correspondences) override
    {
      m_sorter->set_correspondences(correspondences);
    }

    quality_scores_t compute_quality_scores() override
    {
      return m_sorter->compute_quality_scores();
    }

    std::string get_sorter_name() const override
    {
      return m_sorter->get_sorter_name();
    }

  private:
    std::shared_ptr<SorterType> m_sorter;
  };

  // 存储排序器和权重的向量 / Vector storing sorters and weights
  std::vector<std::pair<std::unique_ptr<sorter_wrapper_base_t>, DataType>>
      m_sorters;
};

}  // namespace toolbox::pcl