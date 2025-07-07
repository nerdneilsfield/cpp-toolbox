#pragma once

/**
 * @file registration.hpp
 * @brief 点云配准算法统一导出文件 / Unified export file for point cloud registration algorithms
 * 
 * 该文件提供了点云配准的统一接口，包括粗配准和细配准算法。
 * This file provides a unified interface for point cloud registration, including both
 * coarse and fine registration algorithms.
 * 
 * 粗配准算法 / Coarse Registration Algorithms:
 * - RANSAC: 基于对应关系的随机采样一致性算法 / Correspondence-based random sample consensus
 * - PROSAC: 渐进式采样一致性，利用对应关系质量排序 / Progressive sample consensus with quality ordering
 * - 4PCS: 4点共面集算法，不需要初始对应关系 / 4-Point Congruent Sets, no initial correspondences needed
 * - Super4PCS: 优化的4PCS，适合大规模点云 / Optimized 4PCS for large-scale point clouds
 * 
 * 细配准算法 / Fine Registration Algorithms:
 * - Point-to-Point ICP: 基础ICP算法 / Basic ICP algorithm
 * - Point-to-Plane ICP: 点到平面ICP，需要法线 / Point-to-plane ICP, requires normals
 * - Generalized ICP: 平面到平面ICP / Plane-to-plane ICP
 * - AA-ICP: Anderson加速的ICP / Anderson Accelerated ICP
 * - NDT: 正态分布变换 / Normal Distributions Transform
 * 
 * @code
 * #include <cpp-toolbox/pcl/registration/registration.hpp>
 * 
 * using namespace toolbox::pcl;
 * 
 * // 粗配准示例 / Coarse registration example
 * super_four_pcs_registration_t<float> coarse_reg;
 * coarse_reg.set_source(source_cloud);
 * coarse_reg.set_target(target_cloud);
 * coarse_reg.set_delta(0.01f);     // 1cm精度 / 1cm accuracy
 * coarse_reg.set_overlap(0.5f);    // 50%重叠 / 50% overlap
 * 
 * coarse_registration_result_t<float> coarse_result;
 * if (coarse_reg.align(coarse_result)) {
 *     // 使用粗配准结果作为细配准的初始值
 *     // Use coarse result as initial guess for fine registration
 * }
 * 
 * // 细配准示例 / Fine registration example
 * point_to_point_icp_t<float> fine_reg;
 * fine_reg.set_source(source_cloud);
 * fine_reg.set_target(target_cloud);
 * fine_reg.set_max_iterations(50);
 * 
 * fine_registration_result_t<float> fine_result;
 * fine_reg.align(coarse_result.transformation, fine_result);
 * @endcode
 */

// 基础类和结果结构 / Base classes and result structures
#include <cpp-toolbox/pcl/registration/registration_result.hpp>
#include <cpp-toolbox/pcl/registration/base_coarse_registration.hpp>
#include <cpp-toolbox/pcl/registration/base_fine_registration.hpp>

// 粗配准算法 / Coarse registration algorithms
#include <cpp-toolbox/pcl/registration/ransac_registration.hpp>
#include <cpp-toolbox/pcl/registration/prosac_registration.hpp>
#include <cpp-toolbox/pcl/registration/four_pcs_registration.hpp>
#include <cpp-toolbox/pcl/registration/super_four_pcs_registration.hpp>

// 细配准算法 / Fine registration algorithms
#include <cpp-toolbox/pcl/registration/point_to_point_icp.hpp>
#include <cpp-toolbox/pcl/registration/point_to_plane_icp.hpp>
#include <cpp-toolbox/pcl/registration/generalized_icp.hpp>
#include <cpp-toolbox/pcl/registration/aa_icp.hpp>
#include <cpp-toolbox/pcl/registration/ndt.hpp>

// 必要的类型定义 / Required type definitions
#include <cpp-toolbox/types/point.hpp>

// 日志宏 / Logger macros
#include <cpp-toolbox/logger/thread_logger.hpp>
#define LOG_ERROR_S toolbox::logger::thread_logger_t::instance().error_s()
#define LOG_WARN_S toolbox::logger::thread_logger_t::instance().warn_s()
#define LOG_INFO_S toolbox::logger::thread_logger_t::instance().info_s()

// 便捷类型别名 / Convenient type aliases
namespace toolbox::pcl
{

/**
 * @defgroup registration 点云配准 / Point Cloud Registration
 * @{
 */

// 粗配准单精度类型别名 / Coarse registration single precision aliases
using ransac_registration = ransac_registration_t<float>;
using prosac_registration = prosac_registration_t<float>;
using four_pcs_registration = four_pcs_registration_t<float>;
using super_four_pcs_registration = super_four_pcs_registration_t<float>;
using coarse_registration_result = registration_result_t<float>;

// 粗配准双精度类型别名 / Coarse registration double precision aliases
using ransac_registration_d = ransac_registration_t<double>;
using prosac_registration_d = prosac_registration_t<double>;
using four_pcs_registration_d = four_pcs_registration_t<double>;
using super_four_pcs_registration_d = super_four_pcs_registration_t<double>;
using coarse_registration_result_d = registration_result_t<double>;

// 细配准单精度类型别名 / Fine registration single precision aliases
using point_to_point_icp = point_to_point_icp_t<float>;
using point_to_plane_icp = point_to_plane_icp_t<float>;
using generalized_icp = generalized_icp_t<float>;
using aa_icp = aa_icp_t<float>;
using ndt = ndt_t<float>;
using fine_registration_result = fine_registration_result_t<float>;

// 细配准双精度类型别名 / Fine registration double precision aliases
using point_to_point_icp_d = point_to_point_icp_t<double>;
using point_to_plane_icp_d = point_to_plane_icp_t<double>;
using generalized_icp_d = generalized_icp_t<double>;
using aa_icp_d = aa_icp_t<double>;
using ndt_d = ndt_t<double>;
using fine_registration_result_d = fine_registration_result_t<double>;

/**
 * @brief 配准算法选择指南 / Registration algorithm selection guide
 * 
 * 粗配准算法选择 / Coarse Registration Selection:
 * 
 * 1. **是否有对应关系 / Whether correspondences are available**
 *    - 有对应关系：使用RANSAC或PROSAC / With correspondences: use RANSAC or PROSAC
 *      - 质量排序可用：使用PROSAC（更快） / Quality ordering available: use PROSAC (faster)
 *      - 无质量信息：使用RANSAC / No quality info: use RANSAC
 *    - 无对应关系：使用4PCS或Super4PCS / Without: use 4PCS or Super4PCS
 * 
 * 2. **点云规模 / Point cloud scale**
 *    - 小规模（<10K点）：任意算法 / Small (<10K points): any algorithm
 *    - 中等规模（10K-100K）：RANSAC或4PCS / Medium (10K-100K): RANSAC or 4PCS
 *    - 大规模（>100K）：Super4PCS / Large (>100K): Super4PCS
 * 
 * 细配准算法选择 / Fine Registration Selection:
 * 
 * 1. **点云特征 / Point cloud features**
 *    - 无法线：Point-to-Point ICP 或 NDT / No normals: Point-to-Point ICP or NDT
 *    - 有法线：Point-to-Plane ICP 或 Generalized ICP / With normals: Point-to-Plane ICP or Generalized ICP
 * 
 * 2. **收敛速度要求 / Convergence speed requirements**
 *    - 标准速度：Point-to-Point/Plane ICP / Standard: Point-to-Point/Plane ICP
 *    - 快速收敛：AA-ICP (Anderson加速) / Fast: AA-ICP (Anderson Accelerated)
 * 
 * 3. **鲁棒性需求 / Robustness requirements**
 *    - 标准：Point-to-Point ICP / Standard: Point-to-Point ICP
 *    - 高鲁棒性：Generalized ICP 或 NDT / High robustness: Generalized ICP or NDT
 * 
 * 4. **大规模点云 / Large-scale clouds**
 *    - NDT：基于体素的方法，适合大规模点云 / NDT: Voxel-based, suitable for large clouds
 */

/**
 * @brief 完整配准流程 / Complete registration pipeline
 * @tparam DataType 数据类型 / Data type
 * @param source 源点云 / Source cloud
 * @param target 目标点云 / Target cloud
 * @param use_coarse 是否使用粗配准 / Whether to use coarse registration
 * @param fine_algorithm 细配准算法 / Fine registration algorithm
 * @return 最终变换矩阵 / Final transformation matrix
 * 
 * @code
 * // 完整配准流程 / Complete registration pipeline
 * auto transform = complete_registration(source, target, true, "p2p");
 * 
 * // 仅细配准 / Fine registration only
 * auto transform = complete_registration(source, target, false, "ndt");
 * @endcode
 */
template<typename DataType>
Eigen::Matrix<DataType, 4, 4> complete_registration(
    const std::shared_ptr<toolbox::types::point_cloud_t<DataType>>& source,
    const std::shared_ptr<toolbox::types::point_cloud_t<DataType>>& target,
    bool use_coarse = true,
    const std::string& fine_algorithm = "p2p")
{
  Eigen::Matrix<DataType, 4, 4> initial_guess = Eigen::Matrix<DataType, 4, 4>::Identity();
  
  // 粗配准 / Coarse registration
  if (use_coarse) {
    super_four_pcs_registration_t<DataType> coarse_reg;
    coarse_reg.set_source(source);
    coarse_reg.set_target(target);
    coarse_reg.set_delta(static_cast<DataType>(0.02));  // 2cm精度
    coarse_reg.set_overlap(static_cast<DataType>(0.5)); // 50%重叠
    
    registration_result_t<DataType> coarse_result;
    if (coarse_reg.align(coarse_result) && coarse_result.fitness_score > 0.3) {
      initial_guess = coarse_result.transformation;
      LOG_INFO_S << "粗配准成功，适应度分数 / Coarse registration successful, fitness score: " 
                 << coarse_result.fitness_score;
    }
  }
  
  // 细配准 / Fine registration
  fine_registration_result_t<DataType> fine_result;
  
  if (fine_algorithm == "p2p") {
    point_to_point_icp_t<DataType> reg;
    reg.set_source(source);
    reg.set_target(target);
    reg.set_max_iterations(50);
    reg.align(initial_guess, fine_result);
  } else if (fine_algorithm == "p2l") {
    if (!target->normals.empty()) {
      point_to_plane_icp_t<DataType> reg;
      reg.set_source(source);
      reg.set_target(target);
      reg.set_max_iterations(50);
      reg.align(initial_guess, fine_result);
    } else {
      LOG_WARN_S << "目标点云无法线，回退到Point-to-Point ICP / "
                    "Target cloud has no normals, falling back to Point-to-Point ICP";
      point_to_point_icp_t<DataType> reg;
      reg.set_source(source);
      reg.set_target(target);
      reg.set_max_iterations(50);
      reg.align(initial_guess, fine_result);
    }
  } else if (fine_algorithm == "gicp") {
    generalized_icp_t<DataType> reg;
    reg.set_source(source);
    reg.set_target(target);
    reg.set_max_iterations(50);
    reg.align(initial_guess, fine_result);
  } else if (fine_algorithm == "aa_icp") {
    aa_icp_t<DataType> reg;
    reg.set_source(source);
    reg.set_target(target);
    reg.set_max_iterations(50);
    reg.align(initial_guess, fine_result);
  } else if (fine_algorithm == "ndt") {
    ndt_t<DataType> reg;
    reg.set_source(source);
    reg.set_target(target);
    reg.set_resolution(static_cast<DataType>(0.5));  // 0.5m体素
    reg.set_max_iterations(50);
    reg.align(initial_guess, fine_result);
  } else {
    LOG_ERROR_S << "未知细配准算法 / Unknown fine registration algorithm: " << fine_algorithm;
    return initial_guess;
  }
  
  if (fine_result.converged) {
    LOG_INFO_S << "细配准收敛，迭代次数 / Fine registration converged, iterations: " 
               << fine_result.iterations_performed
               << "，最终误差 / , final error: " << fine_result.final_error;
  } else {
    LOG_WARN_S << "细配准未收敛，终止原因 / Fine registration did not converge, reason: " 
               << fine_result.termination_reason;
  }
  
  return fine_result.transformation;
}

/** @} */ // end of registration group

}  // namespace toolbox::pcl