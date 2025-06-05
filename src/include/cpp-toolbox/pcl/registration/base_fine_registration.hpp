#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <cpp-toolbox/cpp-toolbox_export.hpp>
#include <cpp-toolbox/logger/thread_logger.hpp>
#include <cpp-toolbox/pcl/registration/registration_result.hpp>
#include <cpp-toolbox/types/point.hpp>

#include <Eigen/Core>

namespace toolbox::pcl
{

/**
 * @brief 对应关系类型枚举 / Correspondence type enumeration
 */
enum class correspondence_type_e
{
  POINT_TO_POINT,  ///< 点到点 / Point to point
  POINT_TO_PLANE,  ///< 点到面 / Point to plane
  PLANE_TO_PLANE,  ///< 面到面 / Plane to plane
  POINT_TO_DISTRIBUTION,  ///< 点到分布（如NDT） / Point to distribution (e.g.,
                          ///< NDT)
  CUSTOM  ///< 自定义 / Custom
};

/**
 * @brief 细配准算法的基类（CRTP模式） / Base class for fine registration
 * algorithms (CRTP pattern)
 *
 * @tparam Derived 派生类类型 / Derived class type
 * @tparam DataType 数据类型 / Data type
 */
template<typename Derived, typename DataType>
class CPP_TOOLBOX_EXPORT base_fine_registration_t
{
public:
  using point_cloud = toolbox::types::point_cloud_t<DataType>;
  using point_cloud_ptr = std::shared_ptr<point_cloud>;
  using transformation_t = Eigen::Matrix<DataType, 4, 4>;
  using result_type = fine_registration_result_t<DataType>;
  using iteration_callback_t =
      std::function<bool(const iteration_state_t<DataType>&)>;

  base_fine_registration_t() = default;
  virtual ~base_fine_registration_t() = default;

  // 删除拷贝，允许移动
  base_fine_registration_t(const base_fine_registration_t&) = delete;
  base_fine_registration_t& operator=(const base_fine_registration_t&) = delete;
  base_fine_registration_t(base_fine_registration_t&&) = default;
  base_fine_registration_t& operator=(base_fine_registration_t&&) = default;

  /**
   * @brief 设置源点云 / Set source point cloud
   */
  void set_source(const point_cloud_ptr& source)
  {
    m_source_cloud = source;
    m_source_updated = true;
  }

  /**
   * @brief 设置目标点云 / Set target point cloud
   */
  void set_target(const point_cloud_ptr& target)
  {
    m_target_cloud = target;
    m_target_updated = true;
  }

  /**
   * @brief 设置最大迭代次数 / Set maximum iterations
   */
  void set_max_iterations(std::size_t max_iterations)
  {
    m_max_iterations = max_iterations;
  }

  /**
   * @brief 获取最大迭代次数 / Get maximum iterations
   */
  std::size_t get_max_iterations() const { return m_max_iterations; }

  /**
   * @brief 设置变换epsilon（收敛阈值） / Set transformation epsilon
   * (convergence threshold)
   */
  void set_transformation_epsilon(DataType epsilon)
  {
    m_transformation_epsilon = epsilon;
  }

  /**
   * @brief 获取变换epsilon / Get transformation epsilon
   */
  DataType get_transformation_epsilon() const
  {
    return m_transformation_epsilon;
  }

  /**
   * @brief 设置欧氏距离epsilon / Set Euclidean fitness epsilon
   */
  void set_euclidean_fitness_epsilon(DataType epsilon)
  {
    m_euclidean_fitness_epsilon = epsilon;
  }

  /**
   * @brief 获取欧氏距离epsilon / Get Euclidean fitness epsilon
   */
  DataType get_euclidean_fitness_epsilon() const
  {
    return m_euclidean_fitness_epsilon;
  }

  /**
   * @brief 设置最大对应距离 / Set maximum correspondence distance
   */
  void set_max_correspondence_distance(DataType distance)
  {
    m_max_correspondence_distance = distance;
  }

  /**
   * @brief 获取最大对应距离 / Get maximum correspondence distance
   */
  DataType get_max_correspondence_distance() const
  {
    return m_max_correspondence_distance;
  }

  /**
   * @brief 设置是否记录迭代历史 / Set whether to record iteration history
   */
  void set_record_history(bool record) { m_record_history = record; }

  /**
   * @brief 获取是否记录迭代历史 / Get whether to record iteration history
   */
  bool get_record_history() const { return m_record_history; }

  /**
   * @brief 设置迭代回调函数 / Set iteration callback function
   * @param callback 回调函数，返回false可提前终止 / Callback function, return
   * false to terminate early
   */
  void set_iteration_callback(iteration_callback_t callback)
  {
    m_iteration_callback = callback;
  }

  /**
   * @brief 获取对应关系类型 / Get correspondence type
   */
  correspondence_type_e get_correspondence_type() const
  {
    return static_cast<const Derived*>(this)->get_correspondence_type_impl();
  }

  /**
   * @brief 检查源点云是否有法线 / Check if source cloud has normals
   */
  bool has_source_normals() const
  {
    return m_source_cloud && !m_source_cloud->normals.empty()
        && m_source_cloud->normals.size() == m_source_cloud->points.size();
  }

  /**
   * @brief 检查目标点云是否有法线 / Check if target cloud has normals
   */
  bool has_target_normals() const
  {
    return m_target_cloud && !m_target_cloud->normals.empty()
        && m_target_cloud->normals.size() == m_target_cloud->points.size();
  }

  /**
   * @brief 执行配准 / Perform registration
   * @param initial_guess 初始变换估计 / Initial transformation guess
   * @param result [out] 配准结果 / Registration result
   * @return 是否成功 / Whether successful
   */
  bool align(const transformation_t& initial_guess, result_type& result)
  {
    // 验证输入
    if (!validate_input()) {
      return false;
    }

    // 预处理（派生类可能需要构建数据结构）
    if (m_source_updated || m_target_updated) {
      static_cast<Derived*>(this)->preprocess_impl();
      m_source_updated = false;
      m_target_updated = false;
    }

    // 初始化结果
    result.initial_transformation = initial_guess;
    result.transformation = initial_guess;
    result.converged = false;
    result.iterations_performed = 0;
    result.final_error = std::numeric_limits<DataType>::max();
    result.termination_reason = "running";

    if (m_record_history) {
      result.history.clear();
      result.history.reserve(m_max_iterations);
    }

    // 调用派生类的实现
    return static_cast<Derived*>(this)->align_impl(initial_guess, result);
  }

  /**
   * @brief 执行配准（使用单位矩阵作为初始猜测） / Perform registration (using
   * identity as initial guess)
   */
  bool align(result_type& result)
  {
    return align(transformation_t::Identity(), result);
  }

protected:
  /**
   * @brief 验证输入 / Validate input
   */
  bool validate_input() const
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

    // 验证参数
    if (m_max_iterations == 0) {
      LOG_ERROR_S << "错误：最大迭代次数为0 / Error: Maximum iterations is 0";
      return false;
    }

    if (m_max_correspondence_distance <= 0) {
      LOG_ERROR_S << "错误：最大对应距离必须为正数 / Error: Maximum "
                     "correspondence distance must be positive";
      return false;
    }

    // 派生类的额外验证
    return static_cast<const Derived*>(this)->validate_input_impl();
  }

  /**
   * @brief 派生类的额外输入验证 / Additional input validation for derived class
   */
  bool validate_input_impl() const { return true; }

  /**
   * @brief 检查收敛条件 / Check convergence criteria
   */
  bool has_converged(std::size_t iteration,
                     const transformation_t& current_transform,
                     const transformation_t& previous_transform,
                     DataType current_error,
                     DataType previous_error,
                     std::string& termination_reason) const
  {
    // 检查迭代次数
    if (iteration >= m_max_iterations) {
      termination_reason = "maximum iterations reached";
      return true;
    }

    // 检查变换变化量（跳过第一次迭代）
    if (iteration > 0) {
      transformation_t delta_transform =
          current_transform.inverse() * previous_transform;
      DataType rotation_change =
          delta_transform.template block<3, 3>(0, 0).eulerAngles(0, 1, 2).norm();
      DataType translation_change =
          delta_transform.template block<3, 1>(0, 3).norm();

      if (rotation_change < m_transformation_epsilon
          && translation_change < m_transformation_epsilon)
      {
        termination_reason = "transformation converged";
        return true;
      }
    }

    // 检查误差变化量
    DataType error_change = std::abs(current_error - previous_error);
    if (error_change < m_euclidean_fitness_epsilon) {
      termination_reason = "error converged";
      return true;
    }

    // 派生类可能有额外的收敛条件
    return static_cast<const Derived*>(this)->has_converged_impl(
        iteration,
        current_transform,
        previous_transform,
        current_error,
        previous_error,
        termination_reason);
  }

  /**
   * @brief 派生类的额外收敛检查 / Additional convergence check for derived
   * class
   */
  bool has_converged_impl(std::size_t,
                          const transformation_t&,
                          const transformation_t&,
                          DataType,
                          DataType,
                          std::string&) const
  {
    return false;
  }

  /**
   * @brief 预处理钩子（派生类实现） / Preprocessing hook (derived class
   * implementation)
   */
  void preprocess_impl() {}

  /**
   * @brief 记录迭代状态 / Record iteration state
   */
  void record_iteration(result_type& result,
                        std::size_t iteration,
                        const transformation_t& transform,
                        DataType error,
                        DataType error_change,
                        std::size_t num_correspondences)
  {
    if (m_record_history) {
      iteration_state_t<DataType> state;
      state.iteration = iteration;
      state.transformation = transform;
      state.error = error;
      state.error_change = error_change;
      state.num_correspondences = num_correspondences;
      result.history.push_back(state);
    }

    // 调用回调函数
    if (m_iteration_callback) {
      iteration_state_t<DataType> state;
      state.iteration = iteration;
      state.transformation = transform;
      state.error = error;
      state.error_change = error_change;
      state.num_correspondences = num_correspondences;

      if (!m_iteration_callback(state)) {
        // 回调返回false，提前终止
        result.termination_reason = "terminated by callback";
      }
    }
  }

  // 数据成员
  point_cloud_ptr m_source_cloud;
  point_cloud_ptr m_target_cloud;

  // 参数（带默认值）
  std::size_t m_max_iterations = 50;
  DataType m_transformation_epsilon = static_cast<DataType>(1e-8);
  DataType m_euclidean_fitness_epsilon = static_cast<DataType>(1e-6);
  DataType m_max_correspondence_distance = static_cast<DataType>(0.05);

  // 状态标志
  bool m_source_updated = false;
  bool m_target_updated = false;
  bool m_record_history = false;

  // 回调
  iteration_callback_t m_iteration_callback;
};

}  // namespace toolbox::pcl