#pragma once

#include <functional>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/correspondence/base_correspondence_sorter.hpp>

namespace toolbox::pcl
{

/**
 * @brief 自定义函数的对应关系排序器 / Custom function-based correspondence
 * sorter
 *
 * 该排序器允许用户提供自定义的质量评估函数。
 * This sorter allows users to provide custom quality evaluation functions.
 *
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or
 * double)
 *
 * @code
 * // 使用示例 / Usage example
 * custom_function_sorter_t<float> sorter;
 *
 * // 设置自定义质量函数 / Set custom quality function
 * sorter.set_quality_function(
 *     [](const correspondence_t& corr,
 *        std::size_t corr_idx,
 *        const point_cloud<float>& src,
 *        const point_cloud<float>& tgt) -> float {
 *         // 自定义质量计算 / Custom quality computation
 *         float desc_quality = 1.0f - (corr.distance / 100.0f);
 *
 *         // 可以访问点云数据进行更复杂的计算 / Can access point cloud data for
 * more complex computation const auto& src_point = src.points[corr.src_idx];
 *         const auto& tgt_point = tgt.points[corr.dst_idx];
 *
 *         // 例如：考虑法向量一致性 / Example: consider normal consistency
 *         // float normal_dot = src_point.normal.dot(tgt_point.normal);
 *
 *         return desc_quality;
 *     });
 *
 * auto scores = sorter.compute_quality_scores();
 * @endcode
 */
template<typename DataType>
class CPP_TOOLBOX_EXPORT custom_function_sorter_t
    : public base_correspondence_sorter_t<custom_function_sorter_t<DataType>,
                                           DataType>
{
public:
  using base_type =
      base_correspondence_sorter_t<custom_function_sorter_t<DataType>,
                                   DataType>;
  using typename base_type::point_cloud;
  using typename base_type::quality_scores_t;

  /**
   * @brief 质量函数类型定义 / Quality function type definition
   *
   * 参数 / Parameters:
   * - correspondence: 对应关系 / Correspondence
   * - index: 对应关系在数组中的索引 / Index of correspondence in array
   * - source_cloud: 源点云 / Source point cloud
   * - target_cloud: 目标点云 / Target point cloud
   * 返回值 / Return: 质量分数 / Quality score
   */
  using quality_function_t = std::function<DataType(
      const correspondence_t& correspondence, std::size_t index,
      const point_cloud& source_cloud, const point_cloud& target_cloud)>;

  /**
   * @brief 简化的质量函数类型（仅基于对应关系） / Simplified quality function
   * type (based on correspondence only)
   */
  using simple_quality_function_t =
      std::function<DataType(const correspondence_t& correspondence)>;

  custom_function_sorter_t() = default;
  ~custom_function_sorter_t() = default;

  /**
   * @brief 设置自定义质量函数 / Set custom quality function
   * @param func 质量函数 / Quality function
   */
  void set_quality_function(quality_function_t func)
  {
    m_quality_function = std::move(func);
    m_simple_function = nullptr;
    this->m_cached = false;
  }

  /**
   * @brief 设置简化的质量函数（仅基于对应关系） / Set simplified quality
   * function (based on correspondence only)
   * @param func 简化的质量函数 / Simplified quality function
   */
  void set_simple_quality_function(simple_quality_function_t func)
  {
    m_simple_function = std::move(func);
    m_quality_function = nullptr;
    this->m_cached = false;
  }

  /**
   * @brief 检查是否设置了质量函数 / Check if quality function is set
   * @return 是否已设置 / Whether set
   */
  [[nodiscard]] bool has_quality_function() const
  {
    return m_quality_function || m_simple_function;
  }

protected:
  // Friend declaration to allow base class access
  friend class base_correspondence_sorter_t<custom_function_sorter_t<DataType>,
                                            DataType>;

  /**
   * @brief 计算质量分数的实现 / Implementation of quality score computation
   * @param scores [out] 质量分数 / Quality scores
   */
  void compute_quality_scores_impl(quality_scores_t& scores)
  {
    if (!has_quality_function()) {
      LOG_ERROR_S
          << "错误：未设置质量函数 / Error: Quality function not set";
      std::fill(scores.begin(), scores.end(), 0);
      return;
    }

    const auto& corrs = *this->m_correspondences;

    if (m_simple_function) {
      // 使用简化的质量函数 / Use simplified quality function
      for (std::size_t i = 0; i < corrs.size(); ++i) {
        scores[i] = m_simple_function(corrs[i]);
      }
    } else {
      // 使用完整的质量函数 / Use full quality function
      const auto& src_cloud = *this->m_source_cloud;
      const auto& tgt_cloud = *this->m_target_cloud;

      for (std::size_t i = 0; i < corrs.size(); ++i) {
        scores[i] = m_quality_function(corrs[i], i, src_cloud, tgt_cloud);
      }
    }
  }

  /**
   * @brief 验证输入数据的实现 / Implementation of input validation
   * @return 是否有效 / Whether valid
   */
  [[nodiscard]] bool validate_input_impl() const
  {
    // 如果使用完整的质量函数，需要点云数据 / If using full quality function,
    // need point cloud data
    if (m_quality_function && (!this->m_source_cloud || !this->m_target_cloud)) {
      LOG_ERROR_S << "错误：使用完整质量函数时需要点云数据 / Error: Point "
                     "cloud data required when using full quality function";
      return false;
    }
    return true;
  }

  /**
   * @brief 获取排序器名称实现 / Get sorter name implementation
   * @return 排序器名称 / Sorter name
   */
  [[nodiscard]] std::string get_sorter_name_impl() const
  {
    return "CustomFunction";
  }

private:
  quality_function_t m_quality_function;  ///< 完整的质量函数 / Full quality
                                          ///< function
  simple_quality_function_t
      m_simple_function;  ///< 简化的质量函数 / Simplified quality function
};

}  // namespace toolbox::pcl