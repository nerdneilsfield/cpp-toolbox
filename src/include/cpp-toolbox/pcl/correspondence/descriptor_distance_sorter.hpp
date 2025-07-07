#pragma once

#include <algorithm>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/correspondence/base_correspondence_sorter.hpp>

namespace toolbox::pcl
{

/**
 * @brief 基于描述子距离的对应关系排序器 / Descriptor distance-based
 * correspondence sorter
 *
 * 该排序器使用对应关系中的描述子距离作为质量度量。
 * 可以选择是否反转分数（距离越小质量越高）。
 * This sorter uses descriptor distance in correspondences as quality measure.
 * Can optionally invert the score (smaller distance means higher quality).
 *
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or
 * double)
 *
 * @code
 * // 使用示例 / Usage example
 * descriptor_distance_sorter_t<float> sorter;
 * sorter.set_invert_score(true);  // 距离越小，质量越高 / Smaller distance,
 * higher quality sorter.set_correspondences(correspondences); auto scores =
 * sorter.compute_quality_scores();
 * @endcode
 */
template<typename DataType>
class CPP_TOOLBOX_EXPORT descriptor_distance_sorter_t
    : public base_correspondence_sorter_t<
          descriptor_distance_sorter_t<DataType>, DataType>
{
public:
  using base_type =
      base_correspondence_sorter_t<descriptor_distance_sorter_t<DataType>,
                                   DataType>;
  using typename base_type::quality_scores_t;

  descriptor_distance_sorter_t() = default;
  ~descriptor_distance_sorter_t() = default;

  /**
   * @brief 设置是否反转分数（距离越小质量越高） / Set whether to invert score
   * (smaller distance means higher quality)
   * @param invert true表示反转，false表示不反转 / true to invert, false not to
   * invert
   */
  void set_invert_score(bool invert)
  {
    m_invert = invert;
    this->m_cached = false;  // 参数改变，清除缓存 / Parameter changed, clear
                             // cache
  }

  /**
   * @brief 获取是否反转分数 / Get whether to invert score
   * @return 是否反转 / Whether to invert
   */
  [[nodiscard]] bool get_invert_score() const { return m_invert; }

  /**
   * @brief 设置归一化方式 / Set normalization method
   * @param normalize true启用归一化，false禁用 / true to enable normalization,
   * false to disable
   */
  void set_normalize(bool normalize)
  {
    m_normalize = normalize;
    this->m_cached = false;
  }

  /**
   * @brief 获取是否归一化 / Get whether to normalize
   * @return 是否归一化 / Whether to normalize
   */
  [[nodiscard]] bool get_normalize() const { return m_normalize; }

protected:
  // Friend declaration to allow base class access
  friend class base_correspondence_sorter_t<
      descriptor_distance_sorter_t<DataType>, DataType>;

  /**
   * @brief 计算质量分数的实现 / Implementation of quality score computation
   * @param scores [out] 质量分数 / Quality scores
   */
  void compute_quality_scores_impl(quality_scores_t& scores)
  {
    const auto& corrs = *this->m_correspondences;

    if (m_normalize) {
      // 找到最大和最小距离用于归一化 / Find max and min distance for
      // normalization
      DataType max_distance = 0;
      DataType min_distance = std::numeric_limits<DataType>::max();

      for (const auto& corr : corrs) {
        max_distance =
            std::max(max_distance, static_cast<DataType>(corr.distance));
        min_distance =
            std::min(min_distance, static_cast<DataType>(corr.distance));
      }

      // 避免除零 / Avoid division by zero
      DataType range = max_distance - min_distance;
      if (range < std::numeric_limits<DataType>::epsilon()) {
        range = static_cast<DataType>(1.0);
      }

      // 计算归一化的质量分数 / Compute normalized quality scores
      for (std::size_t i = 0; i < corrs.size(); ++i) {
        DataType normalized =
            (corrs[i].distance - min_distance) / range;  // [0, 1]

        if (m_invert) {
          // 距离越小，质量越高 / Smaller distance, higher quality
          scores[i] = static_cast<DataType>(1.0) - normalized;
        } else {
          // 距离越大，质量越高（不常用） / Larger distance, higher quality
          // (uncommon)
          scores[i] = normalized;
        }
      }
    } else {
      // 不归一化，直接使用距离 / No normalization, use distance directly
      for (std::size_t i = 0; i < corrs.size(); ++i) {
        if (m_invert) {
          // 使用负距离，这样排序时距离小的会排在前面 / Use negative distance,
          // so smaller distances rank first
          scores[i] = -static_cast<DataType>(corrs[i].distance);
        } else {
          scores[i] = static_cast<DataType>(corrs[i].distance);
        }
      }
    }
  }

  /**
   * @brief 获取排序器名称实现 / Get sorter name implementation
   * @return 排序器名称 / Sorter name
   */
  [[nodiscard]] std::string get_sorter_name_impl() const
  {
    return "DescriptorDistance";
  }

private:
  bool m_invert = true;  ///< 默认：距离越小质量越高 / Default: smaller
                         ///< distance means higher quality
  bool m_normalize = true;  ///< 默认启用归一化 / Default: enable normalization
};

}  // namespace toolbox::pcl