#pragma once

#include <memory>
#include <vector>
#include <random>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/pcl/correspondence/base_correspondence_generator.hpp>
#include <cpp-toolbox/pcl/registration/registration_result.hpp>
#include <cpp-toolbox/types/point.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>

// Logger macros
#define LOG_ERROR_S toolbox::logger::thread_logger_t::instance().error_s()
#define LOG_WARN_S toolbox::logger::thread_logger_t::instance().warn_s()
#define LOG_INFO_S toolbox::logger::thread_logger_t::instance().info_s()

namespace toolbox::pcl
{

/**
 * @brief 粗配准算法的基类（CRTP模式） / Base class for coarse registration
 * algorithms (CRTP pattern)
 *
 * @tparam Derived 派生类类型 / Derived class type
 * @tparam DataType 数据类型（如float或double） / Data type (e.g., float or
 * double)
 */
template<typename Derived, typename DataType>
class CPP_TOOLBOX_EXPORT base_coarse_registration_t
{
public:
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;
  using correspondences_ptr = std::shared_ptr<std::vector<correspondence_t>>;
  using result_type = registration_result_t<DataType>;

  base_coarse_registration_t() = default;
  virtual ~base_coarse_registration_t() = default;

  // 删除拷贝，允许移动 / Delete copy, allow move
  base_coarse_registration_t(const base_coarse_registration_t&) = delete;
  base_coarse_registration_t& operator=(const base_coarse_registration_t&) =
      delete;
  base_coarse_registration_t(base_coarse_registration_t&&) = default;
  base_coarse_registration_t& operator=(base_coarse_registration_t&&) = default;

  /**
   * @brief 设置源点云 / Set source point cloud
   */
  void set_source(const point_cloud_ptr& source)
  {
    m_source_cloud = source;
    return static_cast<Derived*>(this)->set_source_impl(source);
  }

  /**
   * @brief 设置目标点云 / Set target point cloud
   */
  void set_target(const point_cloud_ptr& target)
  {
    m_target_cloud = target;
    return static_cast<Derived*>(this)->set_target_impl(target);
  }

  /**
   * @brief 设置初始对应关系（可选，主要用于RANSAC类算法） / Set initial
   * correspondences (optional, mainly for RANSAC-like algorithms)
   */
  void set_correspondences(const correspondences_ptr& correspondences)
  {
    m_correspondences = correspondences;
    return static_cast<Derived*>(this)->set_correspondences_impl(correspondences);
  }

  /**
   * @brief 设置最大迭代次数 / Set maximum iterations
   */
  void set_max_iterations(std::size_t max_iterations)
  {
    m_max_iterations = max_iterations;
    return static_cast<Derived*>(this)->set_max_iterations_impl(max_iterations);
  }

  /**
   * @brief 设置内点距离阈值 / Set inlier distance threshold
   */
  void set_inlier_threshold(DataType threshold)
  {
    m_inlier_threshold = threshold;
    return static_cast<Derived*>(this)->set_inlier_threshold_impl(threshold);
  }

  /**
   * @brief 设置收敛阈值 / Set convergence threshold
   */
  void set_convergence_threshold(DataType threshold)
  {
    m_convergence_threshold = threshold;
    return static_cast<Derived*>(this)->set_convergence_threshold_impl(threshold);
  }

  /**
   * @brief 设置最小内点数量 / Set minimum number of inliers
   */
  void set_min_inliers(std::size_t min_inliers)
  {
    m_min_inliers = min_inliers;
    return static_cast<Derived*>(this)->set_min_inliers_impl(min_inliers);
  }

  /**
   * @brief 执行配准 / Perform registration
   * @param result [out] 配准结果 / Registration result
   * @return 是否成功 / Whether successful
   */
  bool align(result_type& result)
  {
    // 验证输入 / Validate input
    if (!validate_input()) {
      return false;
    }

    // 调用派生类的实现 / Call derived class implementation
    return static_cast<Derived*>(this)->align_impl(result);
  }

  /**
   * @brief 获取算法名称 / Get algorithm name
   */
  [[nodiscard]] std::string get_algorithm_name() const
  {
    return static_cast<const Derived*>(this)->get_algorithm_name_impl();
  }

  /**
   * @brief 启用/禁用并行计算 / Enable/disable parallel computation
   * @param enable true启用，false禁用（默认启用） / true to enable, false to disable (default enabled)
   */
  void enable_parallel(bool enable = true) { m_parallel_enabled = enable; }

  /**
   * @brief 获取并行计算状态 / Get parallel computation status
   * @return 是否启用并行 / Whether parallel is enabled
   */
  [[nodiscard]] bool is_parallel_enabled() const { return m_parallel_enabled; }

  /**
   * @brief 设置随机种子 / Set random seed
   * @param seed 随机种子值 / Random seed value
   */
  void set_random_seed(unsigned int seed) { m_random_seed = seed; }

protected:
  /**
   * @brief 派生类可选实现的钩子函数 / Optional hook functions for derived classes
   */
  void set_source_impl(const point_cloud_ptr& /*source*/) {}
  void set_target_impl(const point_cloud_ptr& /*target*/) {}
  void set_correspondences_impl(const correspondences_ptr& /*correspondences*/) {}
  void set_max_iterations_impl(std::size_t /*max_iterations*/) {}
  void set_inlier_threshold_impl(DataType /*threshold*/) {}
  void set_convergence_threshold_impl(DataType /*threshold*/) {}
  void set_min_inliers_impl(std::size_t /*min_inliers*/) {}

  /**
   * @brief 验证输入数据 / Validate input data
   */
  [[nodiscard]] bool validate_input() const
  {
    if (!m_source_cloud || !m_target_cloud) {
      LOG_ERROR_S << "错误：源点云或目标点云未设置 / Error: Source or target "
                     "cloud not set";
      return false;
    }

    if (m_source_cloud->empty() || m_target_cloud->empty()) {
      LOG_ERROR_S << "错误：点云为空 / Error: Point cloud is empty";
      return false;
    }

    // 派生类可能需要额外的验证 / Derived class may need additional validation
    return static_cast<const Derived*>(this)->validate_input_impl();
  }

  /**
   * @brief 派生类的额外输入验证（可选） / Additional input validation for
   * derived class (optional)
   */
  [[nodiscard]] bool validate_input_impl() const { return true; }

  /**
   * @brief 计算配准质量评分 / Compute registration fitness score
   */
  [[nodiscard]] DataType compute_fitness_score(
      const Eigen::Matrix<DataType, 4, 4>& transformation,
      const std::vector<std::size_t>& inliers) const
  {
    // 基类提供默认实现，派生类可重写 / Base class provides default
    // implementation, derived class can override
    if (inliers.empty()) {
      return std::numeric_limits<DataType>::max();
    }

    // 计算平均内点距离 / Compute average inlier distance
    DataType total_distance = 0;
    for (std::size_t idx : inliers) {
      // 这里简化了，实际需要根据对应关系计算距离
      // Simplified here, actual implementation needs to compute distance based
      // on correspondences
      total_distance += m_inlier_threshold;  // 占位符 / Placeholder
    }

    return total_distance / inliers.size();
  }

  /**
   * @brief 获取受保护的成员变量访问 / Get access to protected members
   */
  [[nodiscard]] const point_cloud_ptr& get_source_cloud() const { return m_source_cloud; }
  [[nodiscard]] const point_cloud_ptr& get_target_cloud() const { return m_target_cloud; }
  [[nodiscard]] const correspondences_ptr& get_correspondences() const { return m_correspondences; }
  [[nodiscard]] std::size_t get_max_iterations() const { return m_max_iterations; }
  [[nodiscard]] DataType get_inlier_threshold() const { return m_inlier_threshold; }
  [[nodiscard]] DataType get_convergence_threshold() const { return m_convergence_threshold; }
  [[nodiscard]] std::size_t get_min_inliers() const { return m_min_inliers; }
  [[nodiscard]] unsigned int get_random_seed() const { return m_random_seed; }

private:
  // 数据成员 / Data members
  point_cloud_ptr m_source_cloud;
  point_cloud_ptr m_target_cloud;
  correspondences_ptr m_correspondences;  ///< 可选的初始对应关系 / Optional
                                          ///< initial correspondences

  // 参数 / Parameters
  std::size_t m_max_iterations = 1000;
  DataType m_inlier_threshold = 0.05;
  DataType m_convergence_threshold = 1e-6;
  std::size_t m_min_inliers = 3;
  bool m_parallel_enabled = true;  ///< 并行计算开关（默认开启） / Parallel computation switch (default enabled)
  unsigned int m_random_seed = std::random_device{}();  ///< 随机种子 / Random seed
};

}  // namespace toolbox::pcl